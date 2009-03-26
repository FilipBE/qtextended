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
#include <qcopchannel_qd.h>

#include <QDebug>
#include <QList>
#include <QMap>
#include <QDataStream>
#include <QRegExp>

namespace qdsync {

class QCopServerRegexp
{
public:
    QCopServerRegexp( const QString &channel, QCopChannel *client );
    QCopServerRegexp( const QCopServerRegexp &other );

    QString channel;
    QCopChannel *client;
    QRegExp regexp;
};

};

QCopServerRegexp::QCopServerRegexp( const QString &channel, QCopChannel *client )
{
    this->channel = channel;
    this->client = client;
    this->regexp = QRegExp( channel, Qt::CaseSensitive, QRegExp::Wildcard );
}

QCopServerRegexp::QCopServerRegexp( const QCopServerRegexp &other )
{
    channel = other.channel;
    client = other.client;
    regexp = other.regexp;
}

typedef QList<QCopServerRegexp> QCopServerRegexpList;
static QCopServerRegexpList *qcopServerRegexpList = 0;

typedef QMap<QString, QList<QCopChannel*> > QCopClientMap;
static QCopClientMap *qcopClientMap = 0;

// Determine if a channel name contains wildcard characters.
static bool containsWildcards( const QString &channel )
{
    return channel.contains(QLatin1Char('*'));
}

// See qcopchannel_qd.qdoc for class documentation

/*!
  Construct a QCopChannel listening to the specified \a channel and with \a parent as the owning QObject.
*/
QCopChannel::QCopChannel(const QString &channel, QObject *parent)
    : QObject(parent)
{
    init(channel);
}

/*!
  \internal
*/
void QCopChannel::init(const QString &channel)
{
    // If the channel name contains wildcard characters, then we also
    // register it on the server regexp matching list.
    if ( containsWildcards(channel) ) {
	QCopServerRegexp item(channel, this);
	if (!qcopServerRegexpList)
	    qcopServerRegexpList = new QCopServerRegexpList;
	qcopServerRegexpList->append( item );
    }

    mChannel = channel;

    if (!qcopClientMap)
        qcopClientMap = new QCopClientMap;

    // do we need a new channel list ?
    QCopClientMap::Iterator it = qcopClientMap->find(channel);
    if (it != qcopClientMap->end()) {
        it.value().append(this);
        return;
    }

    it = qcopClientMap->insert(channel, QList<QCopChannel*>());
    it.value().append(this);
}

/*!
  Destructor.
*/
QCopChannel::~QCopChannel()
{
    {
        QCopClientMap::Iterator it = qcopClientMap->find(mChannel);
        Q_ASSERT(it != qcopClientMap->end());
        it.value().removeAll(this);
    }
    if ( qcopServerRegexpList ) {
        for ( QCopServerRegexpList::iterator it = qcopServerRegexpList->begin(); it != qcopServerRegexpList->end(); ++it ) {
            if ( (*it).client == this ) {
                qcopServerRegexpList->erase(it);
                break;
            }
        }
    }
}

/*!
  Return the channel that this QCopChannel is listening on.
*/
QString QCopChannel::channel() const
{
    return mChannel;
}

/*!
  \internal
*/
void QCopChannel::receive(const QString &msg, const QByteArray &data)
{
    emit received(msg, data);
}

/*!
  \internal
*/
void QCopChannel::send(const QString &ch, const QString &msg, const QByteArray &data)
{
    Q_ASSERT(qcopClientMap);

    // filter out internal events
    if (ch.isEmpty())
        return;

    // feed local clients with received data
    foreach ( QCopChannel *chan, (*qcopClientMap)[ch] )
        chan->receive(msg, data);

    if ( qcopServerRegexpList ) {
        foreach ( const QCopServerRegexp &r, *qcopServerRegexpList ) {
            if ( r.regexp.exactMatch( ch ) ) {
                QByteArray _data;
                QDataStream stream( &_data, QIODevice::WriteOnly );
                stream << ch << msg << data;
                r.client->receive("forwardedMessage(QString,QString,QByteArray)", _data);
            }
        }
    }
}

/*!
  \fn void QCopChannel::received(const QString &msg, const QByteArray &data)
  This signal is emitted when a message arrives. The \a data should be decoded according
  to the contents of \a msg.

  \code
    void messageReceived( const QString &msg, const QByteArray &data )
    {
        QDataStream stream( data );
        if ( msg == "setHint(QString)" ) {
            QString hint;
            hint >> stream;
            // do something
        }
  \endcode
*/

/*!
  Returns true if the \a channel is registered, false otherwise.
*/
bool QCopChannel::isRegistered( const QString &channel )
{
    if ( qcopClientMap->contains(channel) )
        return true;

    if ( qcopServerRegexpList )
        foreach ( const QCopServerRegexp &r, *qcopServerRegexpList )
            if ( r.channel == channel || r.regexp.exactMatch( channel ) )
                return true;

    return false;
}

#endif
