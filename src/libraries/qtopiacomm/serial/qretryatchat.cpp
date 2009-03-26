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

#include <qretryatchat.h>
#include <qtimer.h>

/*!
    \class QRetryAtChat
    \inpublicgroup QtBaseModule

    \brief The QRetryAtChat class provides a mechanism to retry an AT command until it succeeds.
    \ingroup telephony::serial

    The done() signal will be emitted when the command succeeds or when
    the number of retries has been exhausted.

    \sa QAtChat
*/

class QRetryAtChatPrivate
{
public:
    QAtChat *atchat;
    QString command;
    int numRetries;
    bool deleteAfterEmit;
    int timeout;
    QTimer *timer;
};

/*!
    Construct a new retry At chat handler attached to \a parent that will
    send \a command on \a atchat for \a numRetries times before giving up.

    If \a deleteAfterEmit is true, then deleteLater() will be called
    on this object once the done() signal has been emitted.

    The \a timeout parameter specifies the amount of time to wait
    between retries (default is 1 second).
*/
QRetryAtChat::QRetryAtChat( QAtChat *atchat, const QString& command,
                            int numRetries, int timeout,
                            bool deleteAfterEmit, QObject *parent )
    : QObject( parent )
{
    d = new QRetryAtChatPrivate();
    d->atchat = atchat;
    d->command = command;
    d->numRetries = numRetries;
    d->deleteAfterEmit = deleteAfterEmit;
    d->timeout = timeout;
    d->timer = new QTimer( this );
    d->timer->setSingleShot( true );
    connect( d->timer, SIGNAL(timeout()), this, SLOT(timeout()) );
    atchat->chat( command, this, SLOT(doneInternal(bool,QAtResult)) );
}

/*!
    Destruct this retry AT chat handler.
*/
QRetryAtChat::~QRetryAtChat()
{
    delete d;
}

/*!
    \fn void QRetryAtChat::done( bool ok, const QAtResult& result )

    Signal that is emitted when the command completes successfully,
    or when the number of retries has been exhausted.  The command
    results are supplied in \a ok and \a result.
*/

void QRetryAtChat::doneInternal( bool ok, const QAtResult& result )
{
    if ( ok || d->numRetries <= 1 ) {
        emit done( ok, result );
        if ( d->deleteAfterEmit )
            deleteLater();
    } else {
        d->timer->start( d->timeout );
    }
}

void QRetryAtChat::timeout()
{
    --(d->numRetries);
    d->atchat->chat( d->command, this, SLOT(doneInternal(bool,QAtResult)) );
}
