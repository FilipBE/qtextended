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

#include "mmscomms.h"
#include "qmailaccount.h"

#include <private/accountconfiguration_p.h>

#include <QSettings>
#include <qtopialog.h>

/*!
  \class MmsComms
  \inpublicgroup QtMessagingModule
  \brief The MmsComms class is the base class for MMS communications.

  The MmsComms class is the base class for MMS communications.  Usually
  MMS communications via WAP is implemented by deriving from this class
  and implementing the pure virtual functions and emitting the signals
  when appropriate.
*/

/*!
  \fn void MmsComms::sendMessage(MMSMessage &msg, const QByteArray& encoded)

  Connect to the MMSC and POST the specified MMS message \a msg, the
  encoded content of which is contained in \a encoded.
  If a connection to a WAP gateway is necessary, it should be done also.
  The QMailAccount details can be accessed via the \i account variable.

  \sa sendConf()
*/

/*!
  \fn void MmsComms::retrieveMessage(const QUrl &url)

  Connect to the MMSC and GET an MMS message from URL \a url.
  If a connection to a WAP gateway is necessary, it should be done also.
  The QMailAccount details can be accessed via the \i account variable.

  \sa retrieveConf()
*/

/*!
  \fn bool MmsComms::isActive() const

  Return true if there are requests pending or in progress, otherwise false.
*/

/*!
  \fn void MmsComms::clearRequests()

  Clear any requests that are pending or in progress.
*/

/*!
  \fn void MmsComms::notificationInd(const MMSMessage &msg)

  This signal must be emitted when an m-notification-ind MMS message \a msg
  arrives via WAP PUSH.
*/

/*!
  \fn void MmsComms::deliveryInd(const MMSMessage &msg)

  This signal must be emitted when an m-delivery-ind MMS message \a msg
  arrives via WAP PUSH.
*/

/*!
  \fn void MmsComms::sendConf(const MMSMessage &msg)

  This signal must be emitted in response to a sendMessage(), unless
  an error occurs. The MMS message \a msg contains the confirmation details
  and the message content as received by sendMessage() .

  \sa error()
*/

/*!
  \fn void MmsComms::retrieveConf(const MMSMessage &msg, int size)

  This signal must be emitted in response to a retrieveMessage(), unless
  an error occurrs. The MMS message \a msg contains the message as decoded by retrieveMessage() which
  has a encoded size of \a size.

  \sa error()
*/

/*!
  \fn void MmsComms::statusChange(const QString &status)

  This signal can be emitted when the status of a transfer changes, e.g.
  connected to server, transferring data. The status \a status could be used to
  provide user feed back eg: a status bar.

  \sa error()
*/

/*!
  \fn void MmsComms::error(int code, const QString &msg)

  This signal must be emitted when a fatal error occurs during
  the communication process, e.g. connection refused. The code \a code should reflect
  the error seen by the underlying network protocol being used. The error message \a msg must
  be a suitable for displaying to the user and refect the meaning of \a code.
*/

/*!
  \fn void MmsComms::transferSize(int size)

  This signal must be emitted during data transfers so that the user
  can see how much data has been transferred. \a size denotes the
  amount of data transferred in bytes.
*/

/*!
  \fn void MmsComms::transfersComplete()

  This signal must be emitted when all pending or current transfers
  have completed.
*/

/*!
  Constructs a MmsComms, associated with account \a acc and having
  parent object \a parent.
*/

MmsComms::MmsComms(const QMailAccountId &id, QObject *parent)
    : QObject(parent), accountId(id)
{
}

/*!
  Destroys this MmsComms.
*/

MmsComms::~MmsComms()
{
}

/*!
  Returns the default WAP networking account.
*/

QString MmsComms::networkConfig() const
{
    AccountConfiguration config(accountId);

    qLog(Messaging) << "Using network config" << config.networkConfig();
    return config.networkConfig();
}

