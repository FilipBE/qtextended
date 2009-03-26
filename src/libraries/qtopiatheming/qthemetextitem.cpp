/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <qthemetextitem.h>
#include "qthemetextitem_p.h"
#include <qthemeitem.h>
#include <qthemedscene.h>
#include <qthemeitemattribute.h>
#include <QPainter>
#include <QXmlStreamReader>
#include <QExpressionEvaluator>
#include <QDebug>
#include <Qtopia>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <limits.h>

#include "qthemeitem_p.h"

/***************************************************************************/

/*!
  \class QThemeTextItem
    \inpublicgroup QtBaseModule
  \brief The QThemeTextItem class provides a text item that you can add to a QThemedView to display formatted text.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeTextItem.
  The \a parent is passed to the base class constructor.
*/
QThemeTextItem::QThemeTextItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemeTextItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeTextItem.
*/
QThemeTextItem::~QThemeTextItem()
{
    if (d->textExpression) {
        delete d->textExpression;
        d->textExpression = 0;
    }
    delete d;
}

/*!
  \reimp
*/
void QThemeTextItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeItem::loadAttributes(reader);

    d->alignment.setValue(parseAlignment(reader.attributes().value("align").toString()));

    QFont font = themedScene()->font();

    if (!reader.attributes().value("font").isEmpty())
        font = QFont(reader.attributes().value("font").toString());
    if (!reader.attributes().value("bold").isEmpty())
        font.setWeight(reader.attributes().value("bold") == "yes" ? QFont::Bold : QFont::Normal);
    if (!reader.attributes().value("size").isEmpty())
        font.setPointSize(reader.attributes().value("size").toString().toInt());
    if (!reader.attributes().value("color").isEmpty())
        d->color.setValue(reader.attributes().value("color").toString());
    if (!reader.attributes().value("outline").isEmpty())
        d->outline.setValue(reader.attributes().value("outline").toString());
    d->font.setValue(font);
    if (!reader.attributes().value("format").isEmpty()) {
        if (reader.attributes().value("format") == QLatin1String("RichText")) {
            d->format = Qt::RichText;
            d->richText = true;
        } else if (reader.attributes().value("format") == QLatin1String("PlainText")) {
            d->format = Qt::PlainText;
        }
    }
}

/*!
  \reimp
*/
void QThemeTextItem::loadChildren(QXmlStreamReader &reader)
{
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isEndElement())
            break;

        if (reader.isCharacters())
            d->text += reader.text().toString().trimmed();

        if (reader.isStartElement()) {
            if (reader.name() == "tr")
                while (!(reader.isEndElement() && reader.name() == "tr")) {
                    reader.readNext();

                    if (reader.isStartElement() && reader.name() == "trtext")
                        d->trText = reader.readElementText();

                    if (reader.isStartElement() && reader.name() == "trarg")
                        d->trArgs += loadTranslationArguments(reader);
                }
        }
    }
}
#ifdef THEME_EDITOR
void QThemeTextItem::saveAttributes(QXmlStreamWriter &writer)
{
    QThemeItem::saveAttributes(writer);
    QFont font;
    font.fromString(d->font.value().toString());
    if (font.weight() == QFont::Bold)
        writer.writeAttribute("bold", "yes");
    writer.writeAttribute("size", QString::number(font.pointSize()));
    if (d->color.isModified())
        writer.writeAttribute("color", d->color.value().toString());
    if (d->outline.isModified())
        writer.writeAttribute("outline", d->outline.value().toString());
    int a = d->alignment.value().toInt();
    QString align;
    if (a & Qt::AlignRight)
        align += "right,";
    if (a & Qt::AlignHCenter)
        align += "hcenter,";
    if (a & Qt::AlignBottom)
        align += "bottom,";
    if (a & Qt::AlignVCenter)
        align += "vcenter,";
    if (!align.isEmpty()) {
        align.chop(1);
        writer.writeAttribute("align", align);
    }
    writer.writeAttribute("format", d->richText ? "RichText" : "PlainText");
    if (d->trText.isEmpty()) {
        if (d->textExpression)
            writer.writeCharacters("expr:" + d->textExpression->expression());
        else
            writer.writeCharacters(d->displayText);
    } else {
        writer.writeStartElement("tr");
        writer.writeStartElement("trtext");
        writer.writeCharacters(d->trText);
        writer.writeEndElement();
        foreach(QString trArg, d->trArgs) {
            writer.writeStartElement("trarg");
            writer.writeCharacters(trArg);
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
}
#endif

/*!
  \reimp
*/
int QThemeTextItem::type() const
{
    return Type;
}

/*!
  \reimp
  */
void QThemeTextItem::constructionComplete()
{
    QThemeItem::constructionComplete();
    setupThemeText();
}

/*!
  \internal
*/
void QThemeTextItem::loadTranslation()
{
//    QString name = m_view->themeName();
//    name.replace(QChar(' '), QLatin1String(" "));
//    name.prepend(QLatin1String("Theme-"));
//    QString translated = Qtopia::translate(name, m_view->pageName(), trtext);
    QString translated = d->trText;
#if !defined(THEME_EDITOR)
    translated = Qtopia::translate("Theme-Qtopia", "home", d->trText);
#endif

    QRegExp numregex("%[0-9]");

    QStringList split;
    int curIdx = 0, idx = 0;

    while (idx != -1) {
        idx = translated.indexOf(numregex, curIdx);
        if (idx >= 0) {
            if (idx - curIdx > 0) {
                split.append(translated.mid(curIdx, idx - curIdx));
                curIdx = idx;
            }
            split.append(translated.mid(curIdx, 2));
            curIdx += 2;
            idx = curIdx;
        } else if (curIdx < translated.length()) {
            split.append(translated.right(translated.length() - curIdx));
        }
    }

    QString expanded;

    int ii = 0;

    while (ii < split.count()) {
        QString cur = split[ii];

        if (numregex.exactMatch(cur)) {
            if (ii > 0)
                expanded += " . ";

            expanded += cur;

        } else {
            if (ii > 0)
                expanded += " . ";

            if (cur.isEmpty())
                expanded += "\"\"";
            else
                expanded += (QString("\"") + cur + QString("\""));
        }

        ++ii;
    }

    for (QStringList::Iterator it = d->trArgs.begin(); it != d->trArgs.end() ; ++it)
        expanded = expanded.arg(*it);

    if (expanded.isEmpty())
        expanded += "\"\"";

    if (expanded.contains(numregex))
        qWarning("trtext has more variable markers than corresponding trarg elements.");

    //parent->addCharacters(expanded);
    d->text = expanded;
    QThemeItem::d->dataExpression = true;
}

/*!
  \internal
*/
QString QThemeTextItem::loadTranslationArguments(QXmlStreamReader &reader) const
{
    while (!(reader.isEndElement() && name() == "trarg")) {
        reader.readNext();

        if (reader.isEndElement())
            break;

        if (reader.isCharacters())
            return reader.text().toString().trimmed();
    }

    return QString();
}

/*!
  \reimp
  */
void QThemeTextItem::expressionChanged(QExpressionEvaluator *expression)
{
    if (d->textExpression == expression) {
        QVariant result = getExpressionResult(expression, QVariant::String);

        if (!result.canConvert(QVariant::String)) {
            qWarning("ThemeTextItem::expressionChanged() - Expression '%s' could not be coerced to string type, ignoring value.", expression->expression().constData());

        } else {
            setText(result.toString());
        }

    } else {
        QThemeItem::expressionChanged(expression);
    }
}

/*!
  Sets the content for this text item. \a text will be drawn to the screen.
  */
void QThemeTextItem::setText(const QString &text)
{
    Q_ASSERT(d->text.isEmpty()); // no theme text should be left to process
    if (text != d->displayText) {
        d->displayText = text;
        if (d->format == Qt::AutoText)
            d->richText = Qt::mightBeRichText(d->displayText);
        /*        delete d->shadowImg;
                d->shadowImg = 0;*/
        if (isVisible())
            update();
    }
    themedScene()->layout();
    update();
}

/*!
    Returns the text set for this text item.
*/
QString QThemeTextItem::text() const
{ return d->displayText; }

/*!
  Sets the text format of the item explicitly to \a format.
  If the value is Qt::AutoText, the item tries to determine the correct format.
*/
void QThemeTextItem::setTextFormat(Qt::TextFormat format)
{
    d->format = format;
    switch (d->format) {
    case Qt::AutoText:
        d->richText = Qt::mightBeRichText(d->displayText);
        break;
    case Qt::RichText:
        d->richText = true;
        break;
    default:
        d->richText = false;
    }
}

/*!
  Returns the current text format of the text item.
*/
Qt::TextFormat QThemeTextItem::textFormat() const
{
    return d->format;
}

/*!
  \internal
*/
void QThemeTextItem::setupThemeText()
{
    if (!d->trText.isEmpty())
        loadTranslation();

    if (!d->text.isEmpty()) {
        QString newtext = d->text;
        d->text = QString();

        if (QThemeItem::d->dataExpression && !newtext.trimmed().startsWith("expr:"))
            newtext = newtext.prepend("expr:");

        if (isExpression(newtext)) {
            //Q_ASSERT(d->textExpression == 0);
            if (d->textExpression)
                delete d->textExpression;
            d->textExpression = createExpression(strippedExpression(newtext));
            if (d->textExpression != 0)
                expressionChanged(d->textExpression); // Force initial update
        } else {
            setText(newtext);
        }
        setTextFormat(d->format);
    }
}

/*!
  \internal
*/
void QThemeTextItem::drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text)
{
    // Cheaper to paint into pixmap and blit that four times than
    // to draw text four times.

    // First get correct image size for DPI of target.
    QImage img(QSize(1, 1), QImage::Format_ARGB32_Premultiplied);
    QPainter ppm(&img);
    ppm.setFont(painter->font());
    QFontMetrics fm(ppm.font());
    QRect br = fm.boundingRect(rect, flags, text);
    ppm.end();

    // Now create proper image and paint.
    img = QImage(br.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(qRgba(0, 0, 0, 0));
    ppm.begin(&img);
    ppm.setFont(painter->font());
    ppm.setPen(colorFromString(d->outline.value().toString()));
    ppm.translate(rect.topLeft() - br.topLeft());
    ppm.drawText(rect, flags, text);

    QPoint pos(br.topLeft());
    pos += QPoint(-1, 0);
    painter->drawImage(pos, img);
    pos += QPoint(2, 0);
    painter->drawImage(pos, img);
    pos += QPoint(-1, -1);
    painter->drawImage(pos, img);
    pos += QPoint(0, 2);
    painter->drawImage(pos, img);

    img.fill(qRgba(0, 0, 0, 0));
    ppm.setPen(painter->pen());
    ppm.drawText(rect, flags, text);

    pos += QPoint(0, -1);
    painter->drawImage(pos, img);
}

/*!
  \internal
*/
void QThemeTextItem::drawOutline(QPainter *p, const QRect &r, const QPalette &pal, QAbstractTextDocumentLayout *layout)
{
    QColor outlineColor = QThemeItem::colorFromString(d->outline.value().toString());

    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = themedScene()->palette();
//    context.palette.setColor(QPalette::Text, color(outlineColor, outlineRole));
    context.palette.setColor(QPalette::Text, outlineColor);

    p->translate(r.x() - 1, 0);
    layout->draw(p, context);
    p->translate(2, 0);
    layout->draw(p, context);
    p->translate(-1, -1);
    layout->draw(p, context);
    p->translate(0, 2);
    layout->draw(p, context);

    context.palette.setColor(QPalette::Text, pal.color(QPalette::Text));
    p->translate(0, -1);
    layout->draw(p, context);
}

/*!
  \reimp
*/
void QThemeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QThemeItem::paint(painter, option, widget);
    if (d->displayText.isEmpty())
        return;

//     painter->setPen(colorFromString(d->color.value().toString()));
//     painter->setFont(d->font.value().value<QFont>());
//     painter->drawText(boundingRect(), d->alignment.value().toInt(), d->displayText);
//     drawOutline(painter, boundingRect().toRect(), d->alignment.value().toInt(), d->displayText);

    /****/

    QFont defaultFnt = d->font.value().value<QFont>();
    if (!d->displayText.isEmpty()) {
        QFont fnt(defaultFnt);
        QString text(d->displayText);
        qreal x = 0;
        qreal y = 0;
        qreal w = boundingRect().width();
        qreal h = boundingRect().height();
        QTextDocument *doc = 0;
        if (d->richText) {
            int align = d->alignment.value().toInt();
            if (align & Qt::AlignHCenter)
                text = "<center>" + text + "</center>";
            doc = new QTextDocument();
            doc->setHtml(text);
            doc->setPageSize(QSizeF(w, INT_MAX).toSize());
            QAbstractTextDocumentLayout *layout = doc->documentLayout();
            QSizeF bound(layout->documentSize());
            w = bound.width();
            h = bound.height();
            if (align & Qt::AlignRight)
                x += boundingRect().width() - w - 1;
            if (align & Qt::AlignBottom)
                y += boundingRect().height() - h - 1;
            else if (align & Qt::AlignVCenter)
                y += (boundingRect().height() - h - 1) / 2;
//         } else if (d->elidedMode != -1) {
//             QFontMetrics fm(fnt, painter->device());
//             text = fm.elidedText (text, (Qt::TextElideMode)d->elidedMode, rect.width());
        }

        QPalette pal(themedScene()->palette());
//         if (d->shadow) {
//             if (!d->shadowImg) {
//                 QPixmap spm(qMin(w+2,boundingRect().width()), qMin(h+2, boundingRect().height()));
//                 spm.fill();
//                 QPainter sp(&spm);
//                 if (d->richText) {
//                     QAbstractTextDocumentLayout *layout = doc->documentLayout();
//                     doc->setPageSize(QSize(w, INT_MAX));
//                     QAbstractTextDocumentLayout::PaintContext context;
//                     context.palette = view()->palette();
//                     context.palette.setColor(QPalette::Text, Qt::black);
//                     layout->draw(&sp, context);
//                 } else {
//                     sp.setPen(Qt::black);
//                     sp.setFont(fnt);
//                     sp.drawText(QRect(1,1,w,h), d->align, text);
//                 }
//                 QImage img = spm.toImage();
//                 d->shadowImg = new QImage(w+2, h+2, QImage::Format_ARGB32);
//                 d->shadowImg->fill(0x00000000);
//                 int sv = (d->shadow/5) << 24;
//                 for (int i = 1; i < img.height()-1; i++) {
//                     QRgb *r0 = (QRgb *)d->shadowImg->scanLine(i-1);
//                     QRgb *r1 = (QRgb *)d->shadowImg->scanLine(i);
//                     QRgb *r2 = (QRgb *)d->shadowImg->scanLine(i+1);
//                     for (int j = 1; j < img.width()-1; j++, r0++, r1++, r2++) {
//                         if (!(img.pixel(j, i) & 0x00ffffff)) {
//                             *r0 += sv;
//                             *r1 += sv;
//                             *(r1+1) += sv;
//                             *(r1+2) += sv;
//                             *r2 += sv;
//                             *(r2+1) += sv;
//                         }
//                     }
//                 }
//             }
//             painter->drawImage(x, y-1, *d->shadowImg);
//         }
        QColor color = QThemeItem::colorFromString(d->color.value().toString());
        QColor outline = QThemeItem::colorFromString(d->outline.value().toString());
        /*        int role = attribute(QLatin1String("colorRole"), state()),
                    outlineRole = attribute(QLatin1String("outlineRole"), state());*/
        if (d->richText) {
            if (outline.isValid()) {
                pal.setColor(QPalette::Text, color);
                drawOutline(painter, QRectF(x, y, boundingRect().width(), boundingRect().height()).toRect(), pal, doc->documentLayout());
            } else {
                QAbstractTextDocumentLayout *layout = doc->documentLayout();
                doc->setPageSize(QSizeF(w, INT_MAX).toSize());
                QAbstractTextDocumentLayout::PaintContext context;
                context.palette = scene()->palette();
                context.palette.setColor(QPalette::Text, color);
                QRectF cr = QRectF(0, 0, boundingRect().width(), boundingRect().height());
                painter->translate(x, y);
                painter->setClipRect(cr);
                layout->draw(painter, context);
                painter->setClipping(false); // clear our call to setClipRect()
                painter->translate(-x, -y);
            }
        } else {
            painter->setPen(color);
            painter->setFont(fnt);
            if (outline.isValid())
                drawOutline(painter, QRectF(x, y, w, h).toRect(), d->alignment.value().toInt(), text);
            else {
                painter->drawText(QRectF(x, y, w, h).toRect(), d->alignment.value().toInt(), text);
            }
        }
        delete doc;
    }
}

#ifdef THEME_EDITOR
QWidget *QThemeTextItem::editWidget()
{
    return new QThemeTextItemEditor(this, QThemeItem::editWidget());
}
#endif
