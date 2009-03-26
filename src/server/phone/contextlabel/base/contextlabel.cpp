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

#include "contextlabel.h"

#include <qtopiaipcenvelope.h>
#include <qtopialog.h>
#include <QDesktopWidget>
#include <QSettings>
#include <QStyle>
#include <QPixmapCache>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>

/*!
    \class BaseContextLabel::BaseButton
    \inpublicgroup QtUiModule
    \ingroup QtopiaServer::PhoneUI
    \brief The BaseButton class provides a storage for the buttons used by the BaseContextLabel class.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    \fn BaseContextLabel::BaseButton::BaseButton(int key)

    Creates a new BaseButton instance with the given \a key.
*/

/*!
    \fn bool BaseContextLabel::BaseButton::changed()

    Returns true if the buttons description or pixmap has recently changed. This can be
    used to detect recent changes.
*/

/*!
    \fn void BaseContextLabel::BaseButton::setChanged(bool c)

    Sets the changed flag to \a c. BaseContextLabel sets this flag when it detects a change of
    the description or pixmap for this button.
*/

/*!
    \fn int BaseContextLabel::BaseButton::key()

    Returns the key linked to this button.
*/

/*!
    \class BaseContextLabel
    \inpublicgroup QtUiModule
    \ingroup QtopiaServer::PhoneUI
    \brief The BaseContextLabel class provides a dockable soft key bar for phones.

    This class provides an abstract dockable soft key bar for phones.
    It should be used as a base for any soft key bar implementation. The only missing part
    for a complete context label is the user frontend. The context label should be completed
    by subclassing this class.

    The ThemedContextLabel class is an example subclass and adds the UI via theming capabilities.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa ThemedContextLabel, QAbstractContextLabel
*/

/*!
    Create a new BaseContextLabel widget, with the appropriate \a parent and \a flags.
*/

BaseContextLabel::BaseContextLabel( QWidget *parent, Qt::WFlags flags )
    : QAbstractContextLabel(parent, flags),
        menuProvider(0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool |
                   Qt::WindowStaysOnTopHint);

    menuProvider = new QSoftMenuBarProvider(this);
    QObject::connect(menuProvider,SIGNAL(keyChanged(QSoftMenuBarProvider::MenuButton)),
                     this, SLOT(keyChanged(QSoftMenuBarProvider::MenuButton)));
    qLog(UI) << "BaseContextLabel: generating" << menuProvider->keyCount() <<  "buttons";
    if(menuProvider->keyCount()) {
        for(int ii = 0; ii < menuProvider->keyCount(); ++ii) {
            BaseButton *btn = new BaseButton(menuProvider->key(ii).key());
            qLog(UI) << "Add contextbutton:" << btn->key();
            btn->setChanged(false);
            baseBtns.append( btn );
        }
    }
    QtopiaInputEvents::addKeyboardFilter(this);
}

/*!
    Destroys the widget.
*/
BaseContextLabel::~BaseContextLabel()
{
    while ( baseBtns.count() > 0 )
        delete baseBtns.takeLast();
}

/*!
    Returns the list of Buttons managed by the context label. This may be of
    use when accessing the button information from subclasses.
*/
QList<BaseContextLabel::BaseButton*> BaseContextLabel::baseButtons() const
{
    return baseBtns;
}

/*!
    Returns the QSoftMenuProvider controlled by this context label.
*/
QSoftMenuBarProvider *BaseContextLabel::softMenuProvider() const
{
    return menuProvider;
}

/*! \internal */
bool BaseContextLabel::filter(int unicode, int keycode, int modifiers, bool press,
                          bool autoRepeat)
{
    if(isHidden())
        return false;

    //we have to change Key_Context1 to Key_Back and vice versa
    bool rtl = QApplication::layoutDirection() == Qt::RightToLeft;
    if ( rtl ) {
        int code = -1;
        if ( keycode == Qt::Key_Context1 )
            code = Qt::Key_Back;
        if ( keycode == Qt::Key_Back )
            code = Qt::Key_Context1;
        if ( code >= 0 ) {
            QtopiaInputEvents::sendKeyEvent( unicode, code, (Qt::KeyboardModifiers) modifiers, press, autoRepeat );
            return true;
        }
    }
    return false;
}

/*! \internal */
void BaseContextLabel::keyChanged(const QSoftMenuBarProvider::MenuButton &button)
{
    Q_ASSERT(button.index() < menuProvider->keyCount());
    baseButtons().at(button.index())->setChanged(true);
    emit buttonsChanged();
}

/*!
    Specifies the behaviour of the context label when \a pressedButton is pressed.
*/
void BaseContextLabel::buttonPressed(int pressedButton)
{
    if ( pressedButton >= 0 ) {
        int keycode = baseButtons().at(pressedButton)->key();
        if ( QApplication::layoutDirection() == Qt::RightToLeft ) {
             if ( keycode == Qt::Key_Context1 )
                keycode = Qt::Key_Back;
             else if ( keycode == Qt::Key_Back )
                keycode = Qt::Key_Context1;
        }
        QtopiaInputEvents::processKeyEvent(0xffff, keycode, 0, true, false);
    }
}

/*!
    Specifies the behaviour of the context label when \a releasedButton is released.
*/
void BaseContextLabel::buttonReleased(int releasedButton)
{
    if ( releasedButton >= 0 ) {
        int keycode = baseButtons().at(releasedButton)->key();
        if ( QApplication::layoutDirection() == Qt::RightToLeft ) {
             if ( keycode == Qt::Key_Context1 )
                keycode = Qt::Key_Back;
             else if ( keycode == Qt::Key_Back )
                keycode = Qt::Key_Context1;
        }
        QtopiaInputEvents::processKeyEvent(0xffff, keycode, 0, false, false);
    }
}

/*!
    \fn QSize BaseContextLabel::reservedSize() const

    Returns the size reserved for the contextlabel. This size depends on the
    implementation of the class and should be rewritten according to the need.
    The ThemedContextLabel provides an implementation of this function.
*/

/*! \fn void BaseContextLabel::buttonsChanged ()

    Emitted when the key mapping of the menu bar has been changed by QSoftMenuBarProvider.
    The usual action would be to update the pixmap/text displayed by the UI.
    \l BaseButton::changed() returns \c true for the button that has been changed. Once the
    update is performed \l BaseButton::setChanged() should be used to reset the change flag.
*/
