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

#include "common.h"

ChannelObject::ChannelObject()
{
    lastId = 0;
    QCopChannel *chan = new QCopChannel( "QD/*", this );
    connect( chan, SIGNAL(received(QString,QByteArray)), this, SLOT(qpeMessage(QString,QByteArray)) );
}

ChannelObject::~ChannelObject()
{
}

void ChannelObject::send_later( const QString &channel, const QString &message, const QByteArray &data )
{
    TRACE() << "ChannelObject::send_later";
    Message m;
    m.channel = channel;
    m.message = message;
    m.data = data;
    LOG() << "sending" << m.channel << m.message;// << m.data;
    sendingMessages << m;
}

void ChannelObject::send()
{
    //TRACE() << "ChannelObject::send";
    foreach ( const Message &m, sendingMessages ) {
        //LOG() << "Sending" << m.channel << m.message;// << m.data;
        QCopChannel::send( m.channel, m.message, m.data );
    }
    sendingMessages.clear();
}

int ChannelObject::expect( const QString &channel, const QString &message, const QByteArray &data )
{
    TRACE() << "ChannelObject::expect";
    Message m;
    m.id = ++lastId;
    m.channel = channel;
    m.message = message;
    m.data = data;
    LOG() << "expecting" << m.channel << m.message;// << m.data;
    expectedMessages << m;
    checkExpected();
    return m.id;
}

int ChannelObject::expectedCount()
{
    return expectedMessages.count();
}

int ChannelObject::pendingCount()
{
    return pendingMessages.count();
}

void ChannelObject::qpeMessage( const QString &message, const QByteArray &data )
{
    if ( message == "forwardedMessage(QString,QString,QByteArray)" ) {
        TRACE() << "ChannelObject::qpeMessage";
        QDataStream stream( data );
        Message m;
        m.id = 0;
        stream >> m.channel >> m.message >> m.data;
        LOG() << "received" << m.channel << m.message;// << m.data;
        foreach ( const Message &em, expectedMessages ) {
            if ( em == m ) {
                LOG() << "got expected" << em.channel << em.message;// << em.data;
                int id = em.id;
                QByteArray data = m.data;
                expectedMessages.removeAll( em );
                emit gotExpected( id, data );
                return;
            }

        }
        pendingMessages << m;
        LOG() << "got unexpected" << m.channel << m.message;// << m.data;
        emit gotUnexpected( m.channel, m.message, m.data );
    }
}

void ChannelObject::checkExpected()
{
    TRACE() << "ChannelObject::checkExpected";
    foreach ( const Message &em, expectedMessages ) {
        foreach ( const Message &pm, pendingMessages ) {
            if ( em == pm ) {
                LOG() << "got expected" << em.channel << em.message;// << em.data;
                int id = em.id;
                QByteArray data = pm.data;
                expectedMessages.removeAll( em );
                pendingMessages.removeAll( pm );
                emit gotExpected( id, data );
                //break;
            }
        }
    }
}

QString dateToString( const QDate &date )
{
    return date.toString( "yyyy-MM-dd" );
}

QDate stringToDate( const QString &string )
{
    return QDate::fromString( string, "yyyy-MM-dd" );
}

QString dateTimeToString( const QDateTime &datetime, bool utc )
{
    if ( utc )
        return datetime.toString( "yyyy-MM-ddThh:mm:ssZ" );
    else
        return datetime.toString( "yyyy-MM-ddThh:mm:ss" );
}

QDateTime stringToDateTime( const QString &string, bool utc )
{
    if ( utc )
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ssZ" );
    else
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ss" );
}

QString escape( const QString &string )
{
    QString ret = string;
    ret.replace(QRegExp("&"), "&amp;");
    ret.replace(QRegExp(">"), "&gt;");
    ret.replace(QRegExp("<"), "&lt;");
    ret.replace(QRegExp("\r\n"), "\n"); // convert to Unix line endings
    ret.replace(QRegExp("\n$"), ""); // remove the trailing newline
    return ret;
}

QString unescape( const QString &string )
{
    QString ret = string;
    ret.replace(QRegExp("&lt;"), "<");
    ret.replace(QRegExp("&gt;"), ">");
    ret.replace(QRegExp("&amp;"), "&");
    return ret;
}

bool getIdentifier( const QByteArray &record, QString &id, bool &local )
{
    QXmlStreamReader reader(record);
    bool isIdent = false;
    bool leave = false;
    while (!reader.atEnd()) {
        switch(reader.readNext()) {
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::Invalid:
                return false;
            case QXmlStreamReader::StartElement:
                if (reader.qualifiedName() == "Identifier") {
                    isIdent = true;
                    QXmlStreamAttributes identAttr = reader.attributes();
                    QStringRef v = identAttr.value("localIdentifier");
                    if ( v.isNull() || v == "true" || v == "1" )
                        local = true;
                    else
                        local = false;
                }
                break;
            case QXmlStreamReader::EndElement:
                if (isIdent)
                    leave = true;
                break;
            case QXmlStreamReader::Characters:
                if (isIdent)
                    id = unescape(reader.text().toString());
                break;
            default:
                break;
        }
        if ( leave )
            break;
    }
    return true;
}

bool isEquivalent( const QByteArray &record1, const QByteArray &record2 )
{
    if ( record1 == record2 ) return true;

    QByteArray data1 = record1;
    QByteArray data2 = record2;

#define FIX_FORMAT(x)\
    data1.replace("<" #x "></" #x ">","<" #x ">\n</" #x ">");\
    data2.replace("<" #x "></" #x ">","<" #x ">\n</" #x ">")

    FIX_FORMAT(Repeat);
    FIX_FORMAT(CustomFields);
    FIX_FORMAT(Categories);
    FIX_FORMAT(Alarm);

    QStringList list1 = QString(data1.trimmed()).split("\n");
    QStringList list2 = QString(data2.trimmed()).split("\n");

    int max = qMin(list1.count(), list2.count());
    for ( int i = 0; i < max; i++ ) {
        QString line1 = list1[i];
        QString line2 = list2[i];

        line1.replace( QRegExp("^\\s+<"), "<" );
        line2.replace( QRegExp("^\\s+<"), "<" );

        //qWarning() << endl << line1 << endl << line2;
        if ( line1 != line2 ) return false;
    }

    return true;
}

#undef QVERIFY
#define QVERIFY(x) if ( !(x) ) { qWarning() << "Failed" << QString(#x) << "LINE" << __LINE__; ok = false; }\
    else if ( qdTraceEnabled() ) { qWarning() << "VERIFY" << QString(#x) << "LINE" << __LINE__; }
#undef QVERIFY2
#define QVERIFY2(x,m) if ( !(x) ) { qWarning() << m << "(Failed \"" #x "\")" << "LINE" << __LINE__; ok = false; }
bool CommonMethods::startSync()
{
    bool ok = true;
    SEND_QCOP("QPE/Qtopia4Sync","serverSyncRequest(QString)",<< dataset);
    SEND_QCOP("QPE/Qtopia4Sync","serverIdentity(QString)",<< QString("Qtopia Desktop"));
    SEND_QCOP("QPE/Qtopia4Sync","serverVersion(int,int,int)",<< 4 << 3 << 0);
    EXPECT_QCOP("QD/Qtopia4Sync","clientIdentity(QString)",); // random data!
    EXPECT_QCOP("QD/Qtopia4Sync","clientVersion(int,int,int)",<< 4 << 3 << 0);
    WAIT(5,);

    SEND_QCOP("QPE/Qtopia4Sync","serverSyncAnchors(QDateTime,QDateTime)",<< QDateTime() << QDateTime());
    EXPECT_QCOP("QD/Qtopia4Sync","clientSyncAnchors(QDateTime,QDateTime)",);
    WAIT(5,);
    return ok;
}

bool CommonMethods::doSync()
{
    bool ok = true;
    SEND_QCOP("QPE/Qtopia4Sync","requestSlowSync()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    WAIT(5,);
    return ok;
}

bool CommonMethods::endSync()
{
    bool ok = true;
    SEND_QCOP("QPE/Qtopia4Sync","serverEnd()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    WAIT(5,);
    return ok;
}

bool CommonMethods::cleanOutTable()
{
    TRACE() << "cleanOutTable";
    bool ok = true;
    QVERIFY(startSync());

    QList<QString> idsToDelete;
    SEND_QCOP("QPE/Qtopia4Sync","requestSlowSync()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    WAIT(5,{
        while ( spy.count() ) {
            QList<QVariant> args = spy.first();
            QString channel = args.at(0).toString();
            QString message = args.at(1).toString();
            QByteArray data = args.at(2).toByteArray();
            if ( message == "createClientRecord(QByteArray)" ) {
                spy.takeFirst();
                chanobj->expect(channel, message, data);
                QDataStream stream( data );
                QString id;
                bool local;
                QByteArray record;
                stream >> record;
                getIdentifier( record, id, local );
                LOG() << "Client added" << (local?"local":"remote") << "id" << id << "record" << endl << record;
                idsToDelete << id;
            }
        }
    });

    foreach ( const QString &id, idsToDelete )
        SEND_QCOP("QPE/Qtopia4Sync","removeServerRecord(QString)",<<id);

    QVERIFY(endSync());
    return ok;
}

bool CommonMethods::checkForEmptyTable()
{
    bool ok = true;
    QVERIFY(startSync());
    QVERIFY(doSync());
    QVERIFY(endSync());
    return ok;
}

bool CommonMethods::addClientRecord( const QByteArray &record )
{
    bool ok = true;
    QVERIFY(startSync());
    QVERIFY(doSync());

    SEND_QCOP("QPE/Qtopia4Sync","createServerRecord(QByteArray)",<<record);

    SEND_QCOP("QPE/Qtopia4Sync","serverEnd()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    int gotMappedId = 0;
    WAIT(5,{
        while ( spy.count() ) {
            QList<QVariant> args = spy.first();
            QString channel = args.at(0).toString();
            QString message = args.at(1).toString();
            QByteArray data = args.at(2).toByteArray();
            if ( message == "mappedId(QString,QString)" ) {
                gotMappedId++;
                spy.takeFirst();
                QDataStream stream( data );
                QString local;
                QString remote;
                stream >> remote >> local;
                //qDebug() << "Got mapId()" << remote << local;
                idMap[local] = remote;
                chanobj->expect(channel, message, data);
            }
        }
    });
    QVERIFY(gotMappedId);

    return ok;
}

bool CommonMethods::checkForAddedItem( const QByteArray &expected )
{
    TRACE() << "checkForAddedItem";
    bool ok = true;
    QVERIFY(startSync());

    SEND_QCOP("QPE/Qtopia4Sync","requestSlowSync()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    int gotItem = 0;
    WAIT(5,{
        while ( spy.count() ) {
            QList<QVariant> args = spy.first();
            QString channel = args.at(0).toString();
            QString message = args.at(1).toString();
            QByteArray data = args.at(2).toByteArray();
            if ( message == "createClientRecord(QByteArray)" ) {
                gotItem++;
                spy.takeFirst();
                chanobj->expect(channel, message, data);
                QDataStream stream( data );
                QString id;
                bool local;
                QByteArray record;
                stream >> record;
                getIdentifier( record, id, local );
                record.replace(id.toLocal8Bit(), idMap[id].toLocal8Bit());
                QByteArray _record = SANITIZE(record);
                QByteArray _expected = SANITIZE(expected);
                if ( !isEquivalent(_record, _expected) ) {
                    WARNING() << "expected" << endl << _expected << endl
                              << "got" << endl << _record;
                }
                QVERIFY(isEquivalent(_record, _expected));
            }
        }
    });
    QVERIFY(gotItem);

    QVERIFY(endSync());
    return ok;
}

bool CommonMethods::editClientRecord( const QByteArray &record )
{
    bool ok = true;
    QVERIFY(startSync());

    SEND_QCOP("QPE/Qtopia4Sync","requestSlowSync()",);
    EXPECT_QCOP("QD/Qtopia4Sync","createClientRecord(QByteArray)",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    WAIT(5,);

    SEND_QCOP("QPE/Qtopia4Sync","replaceServerRecord(QByteArray)",<<record);

    SEND_QCOP("QPE/Qtopia4Sync","serverEnd()",);
    EXPECT_QCOP("QD/Qtopia4Sync","clientEnd()",);
    WAIT(5,{
        while ( spy.count() ) {
            QList<QVariant> args = spy.first();
            QString channel = args.at(0).toString();
            QString message = args.at(1).toString();
            QByteArray data = args.at(2).toByteArray();
            if ( message == "mapId(QString,QString)" ) {
                spy.takeFirst();
                QDataStream stream( data );
                QString local;
                QString remote;
                stream >> remote >> local;
                idMap[local] = remote;
                chanobj->expect(channel, message, data);
            }
        }
    });

    return ok;
}

void (*messagehandler)(QtMsgType type, const char *msg) = 0;
void aBetterMessageHandler(QtMsgType type, const char *msg)
{
    if ( strlen(msg) < 1000 )
        messagehandler( type, msg );
    else {
        // Output for console
        fprintf(stdout, "%s\n", msg);
        fflush(stdout);
    }
}

