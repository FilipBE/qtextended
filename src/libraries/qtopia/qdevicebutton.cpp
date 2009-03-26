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

#include <qpixmap.h>
#include <qstring.h>
#include "qdevicebutton.h"

/*! \class QDeviceButton
    \inpublicgroup QtBaseModule

    \brief The QDeviceButton class represents a user-programmable button on a Qt Extended device.

    This class represents a physical special-function button on a Qt Extended device.
    The location and number of buttons will vary from device to
    device and the button is described in the help documentation by \c userText() and \c pixmap().

    QDeviceButton objects are only usefully obtained from QDeviceButtonManager::buttonForKeycode().

    \ingroup userinput
*/

/*!
  Constructs QDeviceButton.
*/
QDeviceButton::QDeviceButton() :
    m_Keycode(0),
    m_PressedActionMappable(true),
    m_HeldActionMappable(true),
    m_releasedActionMappable(true)
{
}

/*!
  Destructs QDeviceButton.
*/
QDeviceButton::~QDeviceButton()
{
}

/*!
  Returns the button keycode. See the Qt::Key enum for the values.
 */
int QDeviceButton::keycode() const
{
    return m_Keycode;
}


/*!
  Returns the button context. If the context is empty, then the button
  applies in all contexts. The semantics of non-empty contexts is device specific,
  but in general the meaning of a context is a certain device state. The Qt Extended server defines the context "HomeScreen" such that device button mappings can
  be made that only apply on the device home screen.
 */
QString QDeviceButton::context() const
{
    return m_Context;
}


/*!
  Returns a human-readable, translated description of the button.
 */
QString QDeviceButton::userText() const
{
    return m_UserText;
}

/*!
  Returns the pixmap for this button, or 
  an empty (null) pixmap if none is defined.
 */
QPixmap QDeviceButton::pixmap() const
{
    if ( m_Pixmap.isNull() && !m_PixmapName.isEmpty() ) {
        QDeviceButton *that = (QDeviceButton *)this;
        that->m_Pixmap = QPixmap( ":image/"+m_PixmapName );
    }
    return m_Pixmap;
}

/*!
  Returns the user-assigned action for when this button is pressed.
 */
QtopiaServiceRequest QDeviceButton::pressedAction() const
{
    return m_PressedAction;
}

/*!
  Returns the user-assigned action for when this button is pressed
  and held.
 */
QtopiaServiceRequest QDeviceButton::heldAction() const
{
    return m_HeldAction;
}

/*!
  Returns the user-assigned action for when this button is released.
 */
QtopiaServiceRequest QDeviceButton::releasedAction() const
{
    return m_releasedAction;
}


/*!
  Sets the \a keycode that is sent when the button is pressed.
*/
void QDeviceButton::setKeycode(int keycode)
{
    m_Keycode = keycode;
}

/*!
  Sets the \a context where the keycode applies.

  \sa context()
*/
void QDeviceButton::setContext(const QString& context)
{
    m_Context = context;
}

/*!
  Sets the human-readable, translated description of the button to \a text.
*/
void QDeviceButton::setUserText(const QString& text)
{
    m_UserText = text;
}

/*!
  Set the pixmap for this button to the resource named \a pmn.
  This will ideally match the label on the physical button.
*/
void QDeviceButton::setPixmap(const QString& pmn)
{
    if ( !m_PixmapName.isEmpty() )
        m_Pixmap = QPixmap();
    m_PixmapName = pmn;
}

/*!
  Set the action to be performed when this button is pressed to \a action.
*/
void QDeviceButton::setPressedAction(const QtopiaServiceRequest& action)
{
    m_PressedAction = action;
}

/*!
  Set the action to be performed when this button is pressed and
  held to \a action.
*/
void QDeviceButton::setHeldAction(const QtopiaServiceRequest& action)
{
    m_HeldAction = action;
}

/*!
  Set the action to be performed when this button is released \a action.
*/

void QDeviceButton::setReleasedAction(const QtopiaServiceRequest& action)
{
    m_releasedAction = action;
}

/*!
  \fn bool QDeviceButton::pressedActionMappable() const
  Returns the mappability set for the pressed action.
*/

/*!
  \fn bool QDeviceButton::heldActionMappable() const
  Returns the mappability set for the pressed action.
*/

/*!
  \fn bool QDeviceButton::releasedActionMappable() const
  Returns the mappability set for the released action.
*/

/*!
  Set the mappability of the pressed action to \a mappable.
*/
void QDeviceButton::setPressedActionMappable(bool mappable)
{
    m_PressedActionMappable = mappable;
}

/*!
  Set the mappability of the held action to \a mappable.
*/
void QDeviceButton::setHeldActionMappable(bool mappable)
{
    m_HeldActionMappable = mappable;
}

/*!
  Set the mappability of the released action to \a mappable.
*/
void QDeviceButton::setReleasedActionMappable(bool mappable)
{
    m_releasedActionMappable = mappable;
}

/*!
  Returns true if this QDeviceButton is equal to \a e.
*/
bool QDeviceButton::operator==(const QDeviceButton &e) const
{
    return ((keycode() == e.keycode()) &&
            (userText() == e.userText()) &&
            (context() == e.context()) &&
            (pressedAction() == e.pressedAction()) &&
            (heldAction() == e.heldAction()) &&
            (releasedAction() == e.releasedAction()));
}
