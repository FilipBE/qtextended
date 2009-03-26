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

#include "qthemelevelitem.h"
#include "qthemelevelitem_p.h"
#include <QPainter>
#include <QXmlStreamReader>
#include <QThemeItemAttribute>
#include <QExpressionEvaluator>
#include <QDebug>


QThemeAnimationFrameInfo::QThemeAnimationFrameInfo(QThemeLevelItem *a, int p)
        : QObject(), anim(a), tid(0), period(p)
{
}

QThemeAnimationFrameInfo::~QThemeAnimationFrameInfo()
{
    if (tid) {
        killTimer(tid);
        tid = 0;
    }
}

void QThemeAnimationFrameInfo::setPeriod(int p)
{
    period = p;
}

int QThemeAnimationFrameInfo::getPeriod()
{
    return period;
}

void QThemeAnimationFrameInfo::start()
{
    stop();
    tid = startTimer(period);
}

void QThemeAnimationFrameInfo::stop()
{
    if (tid) {
        killTimer(tid);
        tid = 0;
    }
}

void QThemeAnimationFrameInfo::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);
    anim->advance();
}


/***************************************************************************/
/*!
  \class QThemeLevelItem
    \inpublicgroup QtBaseModule
  \brief The QThemeLevelItem class provides a level item that you can add to a QThemedScene to display the signal or battery level.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeLevelItem.
  The \a parent is passed to the base class constructor.
*/
QThemeLevelItem::QThemeLevelItem(QThemeItem *parent)
        : QThemeImageItem(parent), d(new QThemeLevelItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeLevelItem.
*/
QThemeLevelItem::~QThemeLevelItem()
{
    if (d->playExpression) {
        delete d->playExpression;
        d->playExpression = 0;
    }
    if (d->frameExpression) {
        delete d->frameExpression;
        d->frameExpression = 0;
    }
    delete d;
}

/*!
  \reimp
*/
void QThemeLevelItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeImageItem::loadAttributes(reader);
    if (!reader.attributes().value("count").isEmpty())
        d->count = reader.attributes().value("count").toString().toInt();
    if (!reader.attributes().value("min").isEmpty())
        d->min = reader.attributes().value("min").toString().toInt();
    if (!reader.attributes().value("max").isEmpty())
        d->max = reader.attributes().value("max").toString().toInt();
    if (!reader.attributes().value("loop").isEmpty())
        d->loop = reader.attributes().value("loop").toString().toInt();
    if (!reader.attributes().value("looprev").isEmpty())
        d->looprev = reader.attributes().value("looprev").toString().toInt();

    if (d->animInfo)
        delete d->animInfo;
    d->animInfo = new QThemeAnimationFrameInfo(this, reader.attributes().value("delay").toString().toInt());

    if (!reader.attributes().value("play").isEmpty()) {
        QString play = reader.attributes().value("play").toString();
        if (isExpression(play)) {
            d->playExpression = createExpression(strippedExpression(play));
            if (d->playExpression)
                expressionChanged(d->playExpression);
        } else if (play != "no") {
            /*            if (!attribute("playing"))*/
            start();
        }
    }
}

/*!
  \reimp
*/
void QThemeLevelItem::loadChildren(QXmlStreamReader &reader)
{
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isEndElement())
            break;

        if (reader.isCharacters())
            d->text += reader.text().toString().trimmed();
    }
}

#ifdef THEME_EDITOR
/*!
  \reimp
*/
void QThemeLevelItem::saveAttributes(QXmlStreamWriter &writer)
{
    QThemeImageItem::saveAttributes(writer);
    writer.writeAttribute("count", QString::number(d->count));
    writer.writeAttribute("min", QString::number(d->min));
    writer.writeAttribute("max", QString::number(d->max));
    writer.writeAttribute("loop", QString::number(d->loop));
    writer.writeAttribute("looprev", QString::number(d->looprev));
    if (d->animInfo)
        writer.writeAttribute("delay", QString::number(d->animInfo->getPeriod()));
    else
        writer.writeAttribute("delay", "1000");
    if (d->playExpression)
        writer.writeAttribute("play", QString("expr:") + d->playExpression->expression());
    else
        writer.writeAttribute("play", d->playing ? "yes" : "no");
    if (d->frameExpression)
        writer.writeCharacters(QString("expr:") + d->frameExpression->expression());
    else
        writer.writeCharacters(QString::number(d->value - d->min));
}
#endif
/*!
  \reimp
*/
void QThemeLevelItem::constructionComplete()
{
    if (!d->text.isEmpty()) {
        if (isExpression(d->text)) {
            d->frameExpression = createExpression(strippedExpression(d->text));
            if (d->frameExpression != 0)
                expressionChanged(d->frameExpression);
        } else {
            setFrame(d->text.toInt());
        }
        d->text = QString();
    }
    QThemeImageItem::constructionComplete();
}

/*!
  \reimp
*/
int QThemeLevelItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeLevelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    resizeImages(boundingRect().width() * d->count, boundingRect().height());
    QRectF source(d->currentFrame * boundingRect().width(), 0, boundingRect().width(), boundingRect().height());
    painter->drawPixmap(boundingRect(), image(), source);
    QThemeItem::paint(painter, option, widget);
}

/*!
  \reimp
*/
void QThemeLevelItem::advance(int)
{
    d->currentFrame += d->inc;
    if (d->currentFrame >= d->count) {
        if (d->loop >= 0) {
            if (d->looprev) {
                d->inc = -d->inc;
                d->currentFrame = d->count - 2;
            } else {
                d->currentFrame = d->loop;
            }
        } else {
            stop();
        }
    } else if (d->currentFrame < d->loop && d->inc < 0) {
        d->currentFrame = d->loop + 1;
        d->inc = - d->inc;
    }
    if (isVisible())
        update();
}

/*!
  Starts the animation of the level.
*/
void QThemeLevelItem::start()
{
    d->currentFrame = 0;
    d->inc = 1;

/*    QImage image = getImage();*/
    if (d->animInfo) {
        d->animInfo->start();
        d->playing = true;
    }
}

/*!
  Stops the animation of the level.
*/
void QThemeLevelItem::stop()
{
    if (d->animInfo)
        d->animInfo->stop();
    d->playing = false;
    updateValue(d->value);
}

/*!
  Sets the current frame of the level item to \a frame.
*/
void QThemeLevelItem::setFrame(int frame)
{
    setValue(frame + d->min);
}

/*!
  Sets the current level of the level item to \a value.
*/
void QThemeLevelItem::setValue(int value)
{
    if (d->value != value)
        updateValue(value);
}

/*!
  \internal
*/
void QThemeLevelItem::updateValue(int v)
{
    d->value = v;
    int idx = 0;
    if (d->max - d->min != 0)
        idx = d->count * (d->value - d->min) / (d->max - d->min);
    if (idx < 0)
        idx = 0;
    if (idx >= d->count)
        idx = d->count - 1;

    d->currentFrame = idx;
    update();
}

/*!
  \reimp
*/
void QThemeLevelItem::expressionChanged(QExpressionEvaluator *expr)
{
    if (expr == d->frameExpression) {
        QVariant result = getExpressionResult(expr, QVariant::Int);
        if (!result.canConvert(QVariant::Int)) {
            qWarning() << "QThemeLevelItem::expressionChanged() - Cannot convert value to Int.";
        } else {
            setFrame(result.toInt());
        }
    } else if (expr == d->playExpression) {
        QVariant result = getExpressionResult(expr, QVariant::Bool);
        if (result.canConvert(QVariant::Bool)) {
            bool b = result.toBool();
            if (b) {
                if (d->playing == false)
                    start();
            } else {
                if (d->playing == true)
                    stop();
            }
        }
    } else {
        QThemeItem::expressionChanged(expr);
    }
}

#ifdef THEME_EDITOR

QWidget *QThemeLevelItem::editWidget()
{
    return new QThemeLevelItemEditor(this, QThemeItem::editWidget());
}

#endif

