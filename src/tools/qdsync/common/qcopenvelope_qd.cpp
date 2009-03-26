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
#ifndef Q_QDOC
#include <qcopenvelope_qd.h>
#include <qcopchannel_qd.h>

#include <QDebug>
#include <QBuffer>

/*!
  \class QCopEnvelope
  \brief The QCopEnvelope class simplifies sending messages.

  The QCopEnvelope class should be used instead of the QCopChannel class when sending messages.
  It performs it's work on destruction so if you want to send a message in the middle of a method
  you should create a scope around the QCopEnvelope.

  \code
    QCopEnvelope e("QD/Connection", "setHint(QString)");
    e << "the hint is to duck";
    // send when e goes out of scope
  \endcode

  \code
    {
        QCopEnvelope e("QD/Connection", "setHint(QString)");
        e << "the hint is to duck";
        // send here
    }
  \endcode
*/

/*!
  Construct a QCopEnvelope that will be sent to the \a channel with the \a message.
  It is typical to use parameters in the message so that the receiver can pull the data out.
  Data should be inserted using the stream operator.

  \code
    QCopEnvelope e("QD/Connection", "setHint(QString)");
    e << "the hint is to duck";
  \endcode
*/
QCopEnvelope::QCopEnvelope( const QString &channel, const QString &message )
    : QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(QIODevice::WriteOnly);
}

/*!
  Send the message.
*/
QCopEnvelope::~QCopEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    QCopChannel::send(ch,msg,data);
    delete device();
}

#endif
