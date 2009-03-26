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

#include "qtextentryproxy.h"

#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QString>
#include <QStyle>
#include <QStyleOption>
#include <QApplication>
#include <QtopiaApplication>
#include <QSoftMenuBar>

class QTextEntryProxyData
{
public:
    QTextEntryProxyData() : target(NULL) {}
    QString text;
    QString imText;
    // worry about honoring IM format later. for now, just underline.
    QRect microFocus;
    QWidget *target;
};

/*!
  \class QTextEntryProxy
    \inpublicgroup QtBaseModule
  \preliminary
  \since 4.3

  \brief The QTextEntryProxy class provides a text entry widget for overlaying list widgets.

  QTextEntryProxy is a helper widget for overlaying on listboxes (or other widgets)
  which need a text based display of the currently entered text, whilst still allowing
  the proxied widget to retain focus.

  \code
    QTextEntryProxy *textEntry = new QTextEntryProxy(this, listbox);

    int mFindHeight = textEntry->sizeHint().height();
    QLabel *findIcon = new QLabel;
    findIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
    findIcon->setMargin(2);
    findIcon->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->addWidget(findIcon);
    findLayout->addWidget(textEntry);
    qobject_cast<QVBoxLayout*>(this->layout())->addLayout(findLayout);

    connect(textEntry, SIGNAL(textChanged(QString)), this, SLOT(textEntrytextChanged(QString)));
  \endcode

  To Note: QTextEntryProxy has no way of determining what the SoftMenuBar context labels
  are/were, so it will replace the back key location with QSoftMenuBar::Back when no text is
  displayed by the object. As this handles 90% of use cases, no further work is scheduled on
  this until necessary.

  \ingroup userinput
  \sa setTarget()
*/

/*!
  \fn void QTextEntryProxy::textChanged(const QString &text)

  This signal is emitted whenever the \a text for this control is updated via user input to
  the target control.
*/

/*!
    Constructs a new text entry proxy object setting \a parent and \a target widgets.
*/

QTextEntryProxy::QTextEntryProxy(QWidget *parent, QWidget *target)
    : QWidget(parent)
{
    d = new QTextEntryProxyData;
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::NoFocus);
    setTarget(target);
}

/*!
    Destroys this object
*/
QTextEntryProxy::~QTextEntryProxy()
{
    if(d->target)
    {
        disconnect(d->target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
        d->target->removeEventFilter(this);
        d->target = NULL;
    }
}

static const int verticalMargin = 1;
static const int horizontalMargin =  2;

/*!
    \reimp
*/
QSize QTextEntryProxy::sizeHint() const
{
    QFontMetrics fm(font());

    QStyleOptionFrame option;
    option.initFrom(this);
    option.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    option.midLineWidth = 0;
    option.rect = contentsRect();
    option.state = option.state | QStyle::State_HasFocus | QStyle::State_HasEditFocus;

    int leftmargin, topmargin, rightmargin, bottommargin;
    getContentsMargins(&leftmargin, &topmargin, &rightmargin, &bottommargin);

    int h = qMax(fm.lineSpacing(), 14) + 2*verticalMargin
            + topmargin + bottommargin;
    int w = fm.width(QLatin1Char('x')) * 17 + 2*horizontalMargin
            + leftmargin + rightmargin; // "some"

    return style()->sizeFromContents(QStyle::CT_LineEdit, &option, QSize(w, h).expandedTo(QApplication::globalStrut()), this);
}

/*!
    \internal
    Slot for InputMethod handling.
*/
bool QTextEntryProxy::processInputMethodEvent(QInputMethodEvent *event)
{
    int currentLength = d->text.length();

    // commit text, noting replacement start and length;
    QString newtext = d->text.left(currentLength-event->replacementStart())
        + event->commitString()
        + d->text.mid(currentLength-event->replacementStart()+event->replacementLength());

    d->imText = event->preeditString();
    event->accept();
    if (d->text != newtext) {
        d->text = newtext;
        emit textChanged(text());
    }
    if(text().length() != 0)
        QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::BackSpace);
    else
        QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::Back);
    update();
    return true;
}

/*!
    \internal
    Slot for keypress handling.
*/
bool QTextEntryProxy::processKeyPressEvent(QKeyEvent *event)
{
    if (!event->text().isEmpty() && event->text()[0].isPrint())
    {
        d->text += event->text();
        event->accept();
        if(text().length() != 0)
            QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::BackSpace);
        else
            QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::Back);
        emit textChanged(text());
        update();
        return true;
    } else if ((event->key() == Qt::Key_Back || event->key() == Qt::Key_Backspace) && d->text.length()) {
        d->text = d->text.left(d->text.length()-1);
        event->accept();
        if(text().length() != 0)
            QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::BackSpace);
        else
            QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::Back);
        emit textChanged(text());
        update();
        return true;
    } else {
        return false;
    }
}

/*!
    Returns the current text displayed in the widget
*/
QString QTextEntryProxy::text() const
{
    return d->text;
}

/*!
    Deletes all the text in the text entry.
*/
void QTextEntryProxy::clear()
{
    if (!d->text.isEmpty()) {
        QSoftMenuBar::setLabel(d->target, Qt::Key_Back, QSoftMenuBar::Back);
        d->text.clear();
        emit textChanged(text());
    }
}

/*!
    \reimp
*/
QVariant QTextEntryProxy::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch(query) {
        case Qt::ImMicroFocus:
            return d->microFocus;
            // rect, specifically the cursor rect
        case Qt::ImFont:
            return font();
        case Qt::ImCursorPosition:
            // int
            return d->text.length();
        case Qt::ImSurroundingText:
            // text
            return d->text;
        case Qt::ImCurrentSelection:
            // text
            return QString();
    }
    return QVariant();
}

/*!
    \reimp
*/
void QTextEntryProxy::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    QFont f(font());
    QFontMetrics fm(f);
    f.setUnderline(true);

    // draw QLineEdit in rect
    // draw text in line edit
    // draw undlering imText in lineEdit.
    QStyleOptionFrame option;
    option.initFrom(this);
    option.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    option.midLineWidth = 0;
    option.rect = contentsRect();

    option.state = option.state | QStyle::State_HasFocus | QStyle::State_HasEditFocus | QStyle::State_Sunken;

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, this);

    QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);

    QRect lineRect(r.x() + horizontalMargin, r.y() + (r.height() - fm.height() + 1) / 2,
                    r.width() - 2*horizontalMargin, fm.height());

    painter.setClipRect(lineRect, Qt::IntersectClip);
    int flags = isLeftToRight() ? Qt::AlignLeft : Qt::AlignRight;
    painter.drawText(lineRect, flags, d->text);

    int textWidth = fm.width(d->text);
    int cursorPos;
    if (textWidth < lineRect.width()) {
        if (isLeftToRight()) {
            lineRect.setLeft(lineRect.left()+textWidth);
        } else {
            lineRect.setRight(lineRect.right()-textWidth);
        }

        painter.setClipRect(lineRect, Qt::IntersectClip);
        painter.setFont(f);
        painter.drawText(lineRect, flags, d->imText);
        if (isLeftToRight())
            cursorPos = lineRect.right()+fm.width(d->imText);
        else
            cursorPos = lineRect.left()-fm.width(d->imText);
    } else {
        if (isLeftToRight())
            cursorPos = lineRect.right();
        else
            cursorPos = lineRect.left();
    }
    d->microFocus.setRect(cursorPos, lineRect.y(), 1, lineRect.height());
}
/*!
    \reimp
*/

bool QTextEntryProxy::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == d->target)
    {
        if(event->type() == QEvent::InputMethod)
        {
            if(processInputMethodEvent((QInputMethodEvent*)event))
                return true;
        }
        else if(event->type() == QEvent::KeyPress /*|| event->type() == QEvent::ShortcutOverride*/)
        {
            if(processKeyPressEvent((QKeyEvent*)event))
                return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

/*!
    Sets the \a target of this object. This involves creating an \l {QObject::installEventFilter()}{event filter} on
    the target object, so that it can filter events that it is interested in.
    If this object was previously targeted at another widget, it removes the eventfilters
    placed upon the original object.
*/
void QTextEntryProxy::setTarget ( QWidget * target )
{
    if(d->target)
    {
        disconnect(d->target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
        d->target->removeEventFilter(this);
    }
    if(target)
    {
        connect(target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
        target->installEventFilter(this);
    }
    d->target = target;
}


void QTextEntryProxy::targetDestroyed(QObject * /*obj*/)
{
    disconnect(d->target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
    d->target->removeEventFilter(this);
    d->target = NULL;
}
