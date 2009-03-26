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

#include "qdevicebutton.h"
#include "qdevicebuttonmanager.h"

#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#include <QObject>
#include <QPointer>

QPointer<QDeviceButtonManager> QDeviceButtonManager::m_Instance = 0L;

/*!
  \class QDeviceButtonManager
    \inpublicgroup QtBaseModule

  \brief The QDeviceButtonManager class manages device button mappings.

  The function buttons on a device may generate key presses and also
  activate services.  \c QDeviceButtonManager allows the role of the function
  buttons to be accessed or modified.

  The device key mapping configuration can be stored in one of two configuration
  files, each with the same syntax.  The first is the device's
  \c {defaultbuttons.conf} file, located under the \c {$QPEDIR/etc} path and
  the second is the standard \c {Trolltech/ButtonSettings} configuration file.
  Conflicting entries in the \c {Trolltech/ButtonSettings} will override the
  \c {defaultbuttons.conf} versions.

  The general configuration format is:
  \code
  [Button]
  Count=<mapped button count>

  [Button<X>]
  Name[]=<User Visible Name>
  Key=<Key name>
  Context=<Applicable Context>

  PressedActionMappable=[0|1]
  PressedActionService=<Service name>
  PressedActionMessage=<Message type>
  PressedActionArgs=<Argument list>

  HeldActionMappable=[0|1]
  HeldActionService=<Service name>
  HeldActionMessage=<Message type>
  HeldActionArgs=<Argument list>
  \endcode

  The first group, \c {Button} contains the number of mappings present in
  the file.  Subsequent \c {Button<X>} groups are then named from
  \c {X = 0 ... Count - 1} to represent each mapped key.

  Each key mapping contains the key to which it maps, a name,
  a context and the actions to take when the key is pressed and when it is held.
  The key is mandatory and can be any one of the keys named in the
  Qt::Key enum.  For example, \c {Context1} and \c {F34} are both valid.  The
  user visible name and context are accessible through QDeviceButton::userText()
  and QDeviceButton::context() respectively.

  The \c {PressedActionMappable} and \c {HeldActionMappable} allow the mapping
  for either the pressed or held action to be disabled.  If ommitted, each
  action is enabled (assuming the service and message information required to
  define a complete action is present).

  The \c {PressedActionMessage} and \c {PressedActionService} define the message
  and the service to which the message will be sent.  The \c {PressedActionArgs}
  defines the arguments to send with the message and is deserialized using the
  QtopiaServiceRequest::deserializeArguments() method.  The held action grouping
  takes the same form as the pressed action.

  Whenever the device buttons are updated through the QDeviceButtonManager API,
  a \c {deviceButtonMappingChanged()} message is sent on the \c {QPE/System}
  channel.

  The Buttons settings application in Qt Extended allows the user to configure
  mappable device buttons.


    \ingroup userinput
*/

/*!
  Returns an instance of the \c QDeviceButtonManager.  \c QDeviceButtonManager
  cannot be constructed explicitly.
*/
QDeviceButtonManager& QDeviceButtonManager::instance()
{
    if (!m_Instance)
        m_Instance = new QDeviceButtonManager();
    return *m_Instance;
}

/*!
  \internal
*/
QDeviceButtonManager::QDeviceButtonManager() : QObject( qApp )
{
  QtopiaChannel *channel = new QtopiaChannel( "QPE/System", this );
  connect( channel, SIGNAL(received(QString,QByteArray)),
           this, SLOT(received(QString,QByteArray)) );
  loadButtons();
}

/*!
  \internal
*/
QDeviceButtonManager::~QDeviceButtonManager()
{
    while (m_Buttons.count()) delete m_Buttons.takeLast();
}

/*!
 Returns the available buttons on this device.  The number and location
 of buttons will vary depending on the device.  Button numbers will be assigned
 by the device manufacturer and will be from most preferred button to least preffered
 button.  Note that this list only contains user-programmable buttons.
 */
const QList<QDeviceButton*>& QDeviceButtonManager::buttons() const
{
  return m_Buttons;
}


/*!
  Returns the QDeviceButton for the \a keyCode in the global context.  If \a keyCode is not found, it
  returns 0.
 */
const QDeviceButton* QDeviceButtonManager::buttonForKeycode(int keyCode) const
{
    foreach (const QDeviceButton* b, buttons())
        if (b->keycode() == keyCode && b->context().isEmpty())
            return b;
    return 0L;
}

/*!
  Returns the QDeviceButton for the \a keyCode in a certain \a context.  If \a keyCode is not found, it
  returns 0.
 */
const QDeviceButton* QDeviceButtonManager::buttonForKeycode(int keyCode, const QString& context) const
{
    foreach (const QDeviceButton* b, buttons())
        if (b->keycode() == keyCode && b->context() == context)
            return b;
    return 0L;
}

/*!
 Reassigns the pressed action for \a button_index to \a action.
 */
void QDeviceButtonManager::remapPressedAction(int button_index, const QtopiaServiceRequest& action)
{
    QDeviceButton& button = *m_Buttons[button_index];
    button.setPressedAction(action);
    QSettings buttonFile("Trolltech","ButtonSettings");
    buttonFile.beginGroup("Button" + QString::number(button_index));
    buttonFile.setValue("PressedActionService",
                        (const char*)button.pressedAction().service().toAscii());
    buttonFile.setValue("PressedActionMessage",
                        (const char*)button.pressedAction().message().toAscii());
    buttonFile.setValue("PressedActionArgs",
                        QtopiaServiceRequest::serializeArguments(action));
    buttonFile.endGroup();
    QtopiaIpcEnvelope("QPE/System", "deviceButtonMappingChanged()");
}

/*!
 Reassigns the held action for \a button_index to \a action.
 */
void QDeviceButtonManager::remapHeldAction(int button_index, const QtopiaServiceRequest& action)
{
    QDeviceButton& button = *m_Buttons[button_index];
    button.setHeldAction(action);
    QSettings buttonFile("Trolltech","ButtonSettings");
    buttonFile.beginGroup("Button" + QString::number(button_index));
    buttonFile.setValue("HeldActionService", (const char*)button.heldAction().service().toAscii());
    buttonFile.setValue("HeldActionMessage", (const char*)button.heldAction().message().toAscii());
    buttonFile.setValue("HeldActionArgs", QtopiaServiceRequest::serializeArguments(action));
    buttonFile.endGroup();
    QtopiaIpcEnvelope("QPE/System", "deviceButtonMappingChanged()");
}

/*!
 Reassigns the released action for \a button_index to \a action.
 */
void QDeviceButtonManager::remapReleasedAction(int button_index, const QtopiaServiceRequest& action)
{
    QDeviceButton& button = *m_Buttons[button_index];
    button.setReleasedAction(action);
    QSettings buttonFile("Trolltech","ButtonSettings");
    buttonFile.beginGroup("Button" + QString::number(button_index));
    buttonFile.setValue("ReleasedActionService", (const char*)button.releasedAction().service().toAscii());
    buttonFile.setValue("ReleasedActionMessage", (const char*)button.releasedAction().message().toAscii());
    buttonFile.setValue("ReleasedActionArgs", QtopiaServiceRequest::serializeArguments(action));
    buttonFile.endGroup();
    QtopiaIpcEnvelope("QPE/System", "deviceButtonMappingChanged()");
}

void QDeviceButtonManager::loadButtonSettings(QTranslatableSettings& buttonFile, bool local, bool factory)
{
    if (buttonFile.status()!=QSettings::NoError)
        return;
    QString groupBase("Button"); // No tr
    int n;
    if ( local ) {
        n = m_Buttons.count();
    } else {
        buttonFile.beginGroup(groupBase);
        n = buttonFile.value("Count",0).toInt();
        for (int i=0; i<n; i++)
            m_Buttons.append(new QDeviceButton);
        buttonFile.endGroup();
    }
    int i=0;
    foreach (QDeviceButton* b, m_Buttons) {
        if (i>=n)
            break;
        QString groupName = groupBase + QString::number(i);
        buttonFile.beginGroup(groupName);
        if ( factory && local ) {
            buttonFile.remove("");
        } else {
            if (buttonFile.contains("PressedActionService")) {
                QtopiaServiceRequest pressedAction(buttonFile.value("PressedActionService").toString().toLatin1(), buttonFile.value("PressedActionMessage").toString().toLatin1());
                QByteArray tempArray = buttonFile.value("PressedActionArgs").toByteArray();
                if (!tempArray.isEmpty()) {
                    QtopiaServiceRequest::deserializeArguments(pressedAction, tempArray);
                }
                b->setPressedAction(pressedAction);
            }

            if (buttonFile.contains("HeldActionService")) {
                QtopiaServiceRequest heldAction(buttonFile.value("HeldActionService").toString().toLatin1(), buttonFile.value("HeldActionMessage").toString().toLatin1());
                QByteArray tempArray = buttonFile.value("HeldActionArgs").toByteArray();
                if (!tempArray.isEmpty()) {
                    QtopiaServiceRequest::deserializeArguments(heldAction, tempArray);
                }
                b->setHeldAction(heldAction);
            }

            if (buttonFile.contains("ReleasedActionService")) {
                QtopiaServiceRequest releaseAction(buttonFile.value("ReleasedActionService").toString().toLatin1(),
                                                    buttonFile.value("ReleasedActionMessage").toString().toLatin1());
                QByteArray tempArray = buttonFile.value("ReleasedActionArgs").toByteArray();

                if (!tempArray.isEmpty()) {
                    QtopiaServiceRequest::deserializeArguments(releaseAction, tempArray);
                }

                b->setReleasedAction(releaseAction);
            }

            if ( !local ) {
                // non-variable values
                b->setKeycode(QKeySequence(buttonFile.value("Key").toString()));
                b->setUserText(buttonFile.value("Name").toString());
                b->setPixmap(QString("Button/%1").arg(i));
                b->setPressedActionMappable(buttonFile.value("PressedActionMappable",true).toBool());
                b->setHeldActionMappable(buttonFile.value("HeldActionMappable",true).toBool());
                b->setReleasedActionMappable(buttonFile.value("ReleasedActionMappable", true).toBool());
                b->setContext(buttonFile.value("Context").toString());
            }
        }
        buttonFile.endGroup();
        ++i;
    }
}

void QDeviceButtonManager::loadButtons(bool factory)
{
    while (m_Buttons.count()) delete m_Buttons.takeLast();
    QTranslatableSettings globalButtonFile(Qtopia::defaultButtonsFile(),QSettings::IniFormat);
    loadButtonSettings(globalButtonFile,false,factory);
    QTranslatableSettings localButtonFile("Trolltech","ButtonSettings");
    loadButtonSettings(localButtonFile,true,factory);
}

void QDeviceButtonManager::received(const QString& message, const QByteArray&)
{
    if (message == "deviceButtonMappingChanged()")
        loadButtons();
}

/*!
  Reset the buttons back to the factory default settings.
*/
void QDeviceButtonManager::factoryResetButtons()
{
    loadButtons(true);
}
