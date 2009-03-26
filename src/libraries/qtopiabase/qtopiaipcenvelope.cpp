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

#include "qtopiaipcenvelope.h"
#include <qtopianamespace.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>

#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/*!
  \class QtopiaIpcEnvelope
    \inpublicgroup QtBaseModule

    \ingroup ipc
  \brief The QtopiaIpcEnvelope class encapsulates and sends QCop messages.

  QCop messages allow applications to communicate with each other by
  sending messages using QtopiaIpcEnvelope and receiving messages by connecting
  to a QtopiaChannel.

  To send a message, use the following protocol:
  \code
     QtopiaIpcEnvelope e(channelname, messagename);
     e << parameter1 << parameter2 << ...;
  \endcode

  For messages without parameters, simply use:
  \code
     QtopiaIpcEnvelope e(channelname, messagename);
  \endcode
  where:
  \list
  \o \c{channelname} is the channel name within Qtopia, commencing with "QPE/".
  \o \c{messagename} is a function identifier followed by a list of types
  in parentheses. No white space is permitted.
  \endlist

  Note: Do not try to simplify this further as it may confuse some
  compilers.

  To receive a message either:
  \list
  \o use the predefined QPE/Application/\i{appname} channel in the application.
  For further information refer to:  QtopiaApplication::appMessage().
  \o create another channel and connect it to a slot using:
  \code
      myChannel = new QtopiaChannel( "QPE/FooBar", this );
      connect( myChannel, SIGNAL(received(QString,QByteArray)),
               this, SLOT(fooBarMessage(QString,QByteArray)) );
  \endcode
  \endlist
  See also: \l {Qt Extended IPC Layer}{Qt Extended IPC} and \l {Services}{Services}.

  Consider using QtopiaIpcAdaptor or QtopiaServiceRequest instead of
  QtopiaIpcEnvelope, as they provide a better interface for sending
  QCop messages.

  \sa QtopiaChannel, QtopiaIpcAdaptor, QtopiaServiceRequest
*/

/*!
  Constructs a QtopiaIpcEnvelope to write \a message to \a channel.
  If \a message has parameters then use operator<<() to
  add the parameters to the envelope.
*/
QtopiaIpcEnvelope::QtopiaIpcEnvelope( const QString& channel, const QString& message ) :
    QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(QIODevice::WriteOnly);
}

/*!
  Writes the message and then destroys the QtopiaIpcEnvelope.
*/
QtopiaIpcEnvelope::~QtopiaIpcEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    QtopiaChannel::send( ch, msg, data );
    delete device();
}
