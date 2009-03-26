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

#include "inputdevicesettings.h"
#include <QtopiaChannel>
#include <QDataStream>
#include "qtopiaserverapplication.h"
#include "qtopiainputevents.h"
#include <stdlib.h>

/*!
  \class InputDeviceSettings
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The InputDeviceSettings class allows keyboard and mouse configuration to be adjusted at runtime.

  The current keyboard and mouse driver can be changed, at runtime by sending
  the following QCop messages to the \c {QPE/System} channel:

  \table
  \header \o Message \o Description
  \row \o \c {setMouseProto(QString)}
       \o \i {setMouseProto(QString proto)}

       Set the new mouse protocol to \i proto.  \i proto has the same form as
       would be used in the \c QWS_MOUSE_PROTO environment variable.
  \row \o \c {setKeyboard(QString)}
       \o \i {setKeyboard(QString keyboard)}

       Set the new keyboard driver to \i keyboard.  \i keyboard has the same
       form as would be used in the \c {QWS_KEYBOARD} environment variable.
  \endtable

  The InputDeviceSettings class provides the \c {InputDeviceSettings} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */
/*!
  Create a new InputDeviceSettings instance with the specified \a parent.
 */
InputDeviceSettings::InputDeviceSettings(QObject *parent)
: QObject(parent)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/System", this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(systemMsg(QString,QByteArray)) );
}

void InputDeviceSettings::systemMsg(const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );

    if ( msg == "setMouseProto(QString)" ) {
        QString mice;
        stream >> mice;
        ::setenv("QWS_MOUSE_PROTO",(const char *)mice.toLatin1(),1);
        QtopiaInputEvents::openMouse();
    } else if ( msg == "setKeyboard(QString)" ) {
        QString kb;
        stream >> kb;
        ::setenv("QWS_KEYBOARD",(const char *)kb.toLatin1(),1);
        QtopiaInputEvents::openKeyboard();

    }
}

QTOPIA_TASK(InputDeviceSettings, InputDeviceSettings);
