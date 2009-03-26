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

#include "qabstracthomescreen.h"

/*!
    \class QAbstractHomeScreen
    \inpublicgroup QtBaseModule
    \brief The QAbstractHomeScreen class allows developers to replace the "home screen" portion of the Phone UI.
    \ingroup QtopiaServer::PhoneUI::TTSmartPhone

    The QAbstractHomeScreen interface is part of the
    \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework} and allows developers
    to replace the standard home/idle screen in the Qt Extended phone UI.
    A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
    be found in QAbstractServerInterface class documentation.

    The QAbstractHomeScreen interface is marked as singleton interface. For more details
    about the concept of singleton server widgets refer to the \l {QtopiaServerApplication#singleton-pattern}{server widget documentation}.

    If a homescreen implementation provides a display area for user messages
    (such as Network registration or Location information) it should use the MessageBoard
    task to subscribe to new messages from the system.

    \section1 Integration hints and requirements

    For integration purposes any HomeScreen should support the following features:

    \section2 Background image

    The HomeScreen should listen on the \c {QPE/System} qcop channel for
    \c applyHomeScreenImage() messages. This message is sent when the user sets
    a new background image/wallpaper for the HomeScreen. The image information are stored
    in the \c {Trolltech/qpe} settings file. The \c HomeScreen/HomeScreenPicture key stores
    the image path and \c HomeScreen/HomeScreenPictureMode gives an indication how the
    image should be layed out (ScaleAndCrop, Center/Tile, Scale and Stretch). The
    Homescreen is closely linked to the HomeScreen settings application which is used to set
    these values.

    \section2 General HomeScreen information

    The HomeScreen should listen on the \c {QPE/System} qcop channel for
    \c {updateHomeScreenInfo()} messages. This message is sent when the user changes the
    items to be displayed on the HomeScreen. The HomeScreen settings application
    stores the following values in the \c {Trolltech/qpe} settings file:
    \table
        \header \o Key  \o Description
        \row    \o HomeScreen/ShowDate     \o Set to true if the Homescreen should show the current date.
        \row    \o HomeScreen/ShowLocation \o Set to true if the current GSM/3G cell location should be shown.
        \row    \o HomeScreen/ShowOperator \o Set to true if the current net operator name should be shown.
        \row    \o HomeScreen/ShowProfile  \o Set to true if the name of the current ring profile should be shown.
        \row    \o HomeScreen/ShowTime     \o Set to true if the current time should be shown.
    \endtable

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa MessageBoard
*/

/*!
    Creates a new QAbstractHomeScreen instance with the given \a parent and \a flags.
*/
QAbstractHomeScreen::QAbstractHomeScreen(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the QAbstractHomeScreen instance.
*/
QAbstractHomeScreen::~QAbstractHomeScreen()
{
}

/*!
    \fn bool QAbstractHomeScreen::locked() const = 0

    Returns true if the homescreen is locked. This mode can be set by the user if he
    wants to prevent accidental input while the device is e.g. in his pocket.

    The Qt Extended default implementation disinguishes between a
    touchscreen and a keypad based lock. To disengage the touchscreen lock the user has
    to perform a sliding gesture whereas the keypad lock requires the pressing of certain
    key combination.

    \sa setLocked()
*/

#ifdef QTOPIA_CELL
/*!
    \fn bool QAbstractHomeScreen::simLocked() const = 0

    Returns true if the HomeScreen is locked due to a missing or not yet entered SIM PIN.
    This state can be left by entering the correct SIM PIN.

    \sa setLocked(), locked()
*/
#endif

/*!
    \fn void QAbstractHomeScreen::setLocked(bool state) = 0

    Sets the current lock state of the HomeScreen to \a state.

    \sa locked()
*/

#ifdef QTOPIA_TELEPHONY

/*!
    \fn void QAbstractHomeScreen::callEmergency(const QString &number)

    This signal is emitted when the user has chosen to dial the emergency \a number.
    Since the HomeScreen is not the usual place for dial input this only happens
    while the HomeScreen is locked. Otherwise the user would enter the dialer to
    dial the emergency number.
*/

/*!
    \fn void QAbstractHomeScreen::showCallScreen()

    This signal is emitted when the call screen is required. the connecting slot should
    show the call screen.

    \sa QAbstractCallScreen
*/

/*!
    \fn void QAbstractHomeScreen::showCallHistory()

    This signal is emitted when the callhistory should be shown.

    \sa QAbstractCallHistory
*/

/*!
    \fn void QAbstractHomeScreen::showMissedCalls()

    This signal is emitted when the list of missed calls should be shown.
    The most likely reaction to this signal is very similar to the showCallHistory() signal
    with the difference that the shown list should have a filter applied that only
    accepts missed calls.
*/

/*!
    \fn void QAbstractHomeScreen::dialNumber(const QString &number)

    This signal is emitted when the user dials a \a number. The connecting slot should
    show the dial screen.

    \sa QAbstractDialerScreen
*/

/*!
    \fn void QAbstractHomeScreen::hangupCall()

    This signal is emitted when the user presses the hangup key while on the HomeScreen.
    the connecting slot should hang the currently active call up.
*/
#endif

/*!
    \fn void QAbstractHomeScreen::speedDial(const QString &speeddial)

    This signal is emitted when the user performs a speedial operation. A \a speeddial operation
    is usually indicated by a press and hold key action.
*/

/*!
    \fn void QAbstractHomeScreen::lockStateChanged(bool locked)

    This signal is emitted when the current locking state of the HomeScreen changes to \a locked.

    \sa locked(), setLocked()

*/

/*!
    \fn void QAbstractHomeScreen::showPhoneBrowser()

    This signal is emitted when the user changes to the application browser window. Usually
    this is indicated by pressing the select key. The connecting slot should open the application
    browser screen.

    \sa QAbstractBrowserScreen
*/
