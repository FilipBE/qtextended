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
#include "outlooksync.h"
#include "outlookthread.h"

#include <trace.h>
#include <qcopenvelope_qd.h>
#include <desktopsettings.h>
#include <center.h>

#include <QBuffer>
#include <QApplication>
#include <QXmlStreamReader>
#include <QTimer>

class CacheSettings : public QSettings
{
public:
    CacheSettings( const QString &id )
        : QSettings(DesktopSettings::homePath()+"outlooksync.cache", QSettings::IniFormat)
    {
        beginGroup(id);
    }
    ~CacheSettings()
    {
    }
};

// =====================================================================

OutlookSyncPlugin::OutlookSyncPlugin( QObject *parent )
    : QDServerSyncPlugin( parent ), thread( 0 )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::OutlookSyncPlugin";
}

OutlookSyncPlugin::~OutlookSyncPlugin()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::~OutlookSyncPlugin";
}

QString OutlookSyncPlugin::id()
{
    return QString("com.trolltech.plugin.outlook.sync.%1").arg(dataset());
}

void OutlookSyncPlugin::prepareForSync()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::prepareForSync";
    Q_ASSERT(!thread);

    thread = OutlookThread::getInstance( centerInterface()->syncObject() );

    so = new OTSyncObject();
    so->q = this;
    so->o = thread->o;
    so->moveToThread( thread );

    // connect up the sync messages to the thread
    connect( this, SIGNAL(t_fetchChangesSince(QDateTime)), so, SLOT(fetchChangesSince(QDateTime)) );
    connect( this, SIGNAL(t_createClientRecord(QByteArray)), so, SLOT(createClientRecord(QByteArray)), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL(t_replaceClientRecord(QByteArray)), so, SLOT(replaceClientRecord(QByteArray)), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL(t_removeClientRecord(QString)), so, SLOT(removeClientRecord(QString)), Qt::BlockingQueuedConnection );
    connect( this, SIGNAL(t_waitForAbort()), so, SLOT(waitForAbort()), Qt::BlockingQueuedConnection );
    // connect up the thread's signals to ours
    connect( so, SIGNAL(mappedId(QString,QString)), this, SIGNAL(mappedId(QString,QString)) );
    connect( so, SIGNAL(createServerRecord(QByteArray)), this, SIGNAL(createServerRecord(QByteArray)) );
    connect( so, SIGNAL(replaceServerRecord(QByteArray)), this, SIGNAL(replaceServerRecord(QByteArray)) );
    connect( so, SIGNAL(removeServerRecord(QString)), this, SIGNAL(removeServerRecord(QString)) );
    connect( so, SIGNAL(serverChangesCompleted()), this, SIGNAL(serverChangesCompleted()) );

    connect( so, SIGNAL(logonDone(bool)), this, SLOT(logonDone(bool)) );
    QTimer::singleShot( 0, so, SLOT(logon()) );
}

void OutlookSyncPlugin::finishSync()
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::finishSync";
    // Since this object may be in use on another thread, set the abort flag and
    // deliver a blocking queued event to verify that it is safe to delete it.
    so->abort = true;
    emit t_waitForAbort();
    delete so;
    so = 0;
    thread = 0; // cleaned up by centerInterface()->syncObject()
    emit finishedSync();
}

void OutlookSyncPlugin::fetchChangesSince(const QDateTime &timestamp)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::fetchChangesSince";
    // returns immediately
    emit t_fetchChangesSince( timestamp );
}

void OutlookSyncPlugin::logonDone( bool ok )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::logonDone";
    emit readyForSync( ok );
}

void OutlookSyncPlugin::createClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::createClientRecord";
    // blocks until complete
    emit t_createClientRecord( record );
}

void OutlookSyncPlugin::replaceClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::replaceClientRecord";
    // blocks until complete
    emit t_replaceClientRecord( record );
}

void OutlookSyncPlugin::removeClientRecord(const QString &identifier)
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::removeClientRecord";
    // blocks until complete
    emit t_removeClientRecord( identifier );
}

QString OutlookSyncPlugin::dateToString( const QDate &date )
{
    return date.toString( "yyyy-MM-dd" );
}

QDate OutlookSyncPlugin::stringToDate( const QString &string )
{
    return QDate::fromString( string, "yyyy-MM-dd" );
}

QString OutlookSyncPlugin::dateTimeToString( const QDateTime &datetime, bool utc )
{
    if ( utc )
        return datetime.toString( "yyyy-MM-ddThh:mm:ssZ" );
    else
        return datetime.toString( "yyyy-MM-ddThh:mm:ss" );
}

QDateTime OutlookSyncPlugin::stringToDateTime( const QString &string, bool utc )
{
    if ( utc )
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ssZ" );
    else
        return QDateTime::fromString( string, "yyyy-MM-ddThh:mm:ss" );
}

QString OutlookSyncPlugin::escape( const QString &string )
{
    QString ret = string;
    ret.replace(QRegExp("\r\n"), "\n"); // convert to Unix line endings
    ret.replace(QRegExp("\n$"), ""); // remove the trailing newline
    return ret;
}

bool OutlookSyncPlugin::getIdentifier( const QByteArray &record, QString &id, bool &local )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::getIdentifier";
    QXmlStreamReader reader(record);
    bool isIdent = false;
    bool leave = false;
    while (!reader.atEnd()) {
        switch(reader.readNext()) {
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::Invalid:
                WARNING() << "ERROR: getIdentifier() could not parse record due to error:"
                          << (reader.hasError()?reader.errorString():QString("Unspecified Error"))
                          << "record:" << endl << record;
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
                    id = reader.text().toString();
                break;
            default:
                break;
        }
        if ( leave )
            break;
    }
    return true;
}

QString OutlookSyncPlugin::dump_item_class( Outlook::OlObjectClass item_class )
{
#define CASE(x) case Outlook::x: return #x;
    switch ( item_class ) {
        CASE(olApplication)
        CASE(olNamespace)
        CASE(olFolder)
        CASE(olRecipient)
        CASE(olAttachment)
        CASE(olAddressList)
        CASE(olAddressEntry)
        CASE(olFolders)
        CASE(olItems)
        CASE(olRecipients)
        CASE(olAttachments)
        CASE(olAddressLists)
        CASE(olAddressEntries)
        CASE(olAppointment)
        CASE(olMeetingRequest)
        CASE(olMeetingCancellation)
        CASE(olMeetingResponseNegative)
        CASE(olMeetingResponsePositive)
        CASE(olMeetingResponseTentative)
        CASE(olRecurrencePattern)
        CASE(olExceptions)
        CASE(olException)
        CASE(olAction)
        CASE(olActions)
        CASE(olExplorer)
        CASE(olInspector)
        CASE(olPages)
        CASE(olFormDescription)
        CASE(olUserProperties)
        CASE(olUserProperty)
        CASE(olContact)
        CASE(olDocument)
        CASE(olJournal)
        CASE(olMail)
        CASE(olNote)
        CASE(olPost)
        CASE(olReport)
        CASE(olRemote)
        CASE(olTask)
        CASE(olTaskRequest)
        CASE(olTaskRequestUpdate)
        CASE(olTaskRequestAccept)
        CASE(olTaskRequestDecline)
        CASE(olExplorers)
        CASE(olInspectors)
        CASE(olPanes)
        CASE(olOutlookBarPane)
        CASE(olOutlookBarStorage)
        CASE(olOutlookBarGroups)
        CASE(olOutlookBarGroup)
        CASE(olOutlookBarShortcuts)
        CASE(olOutlookBarShortcut)
        CASE(olDistributionList)
        CASE(olPropertyPageSite)
        CASE(olPropertyPages)
        CASE(olSyncObject)
        CASE(olSyncObjects)
        CASE(olSelection)
        CASE(olLink)
        CASE(olLinks)
        CASE(olSearch)
        CASE(olResults)
        CASE(olViews)
        CASE(olView)
        CASE(olItemProperties)
        CASE(olItemProperty)
        CASE(olReminders)
        CASE(olReminder)
#undef CASE
        default:
            return QString("Unknown (%1)").arg((int)item_class);
    }
}

void OutlookSyncPlugin::init_item( IDispatchPtr dispatch )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::init_item";
}

// Transaction support is not implemented
void OutlookSyncPlugin::beginTransaction(const QDateTime & /*timestamp*/)
{
}

void OutlookSyncPlugin::abortTransaction()
{
}

void OutlookSyncPlugin::commitTransaction()
{
}

// =====================================================================

OTSyncObject::OTSyncObject()
    : QObject(), q(0), o(0), mapi(0), abort( false )
{
}

OTSyncObject::~OTSyncObject()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::~OTSyncObject";
    LOG() << "mapi" << mapi;
    if ( mapi ) {
        delete mapi;
        mapi = 0;
    }

    CacheSettings settings(q->id());
    LOG() << "SAVING rememberedIds" << rememberedIds;
    settings.setValue( "rememberedIds", rememberedIds );
    // Clean out stale cache entries
    settings.beginGroup("cache");
    foreach (const QString &key, settings.allKeys())
        if (!rememberedIds.contains(key))
            settings.remove(key);
}

void OTSyncObject::logon()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::logon";
    bool ok = false;

    if ( !q ) {
        WARNING() << "BUG: Cannot call OTSyncObject::logon() while q is 0!";
        return;
    }

    LOG() << "q" << q;
    LOG() << "q->id()" << q->id();
    LOG() << "CacheSettings settings(q->id())";
    CacheSettings settings(q->id());
    LOG() << "settings.value(\"rememberedIds\")";
    rememberedIds = settings.value( "rememberedIds" ).toStringList();
    LOG() << rememberedIds;

    if ( o->logon() ) {
        LOG() << "mapi = new QMAPI::Session()";
        mapi = new QMAPI::Session();
        LOG() << "mapi" << mapi << "connected" << mapi->connected();
        if ( !mapi->connected() ) {
            delete mapi;
            mapi = 0;
            LOG() << "Can't connect to MAPI";
            // TODO there should be a non-connection-specific hint system to plug into
            QCopEnvelope e( "QD/Connection", "setHint(QString,QString)" );
            e << tr("Can't connect to MAPI") << (QString)QLatin1String("mapi");
        }
        ok = true;
    }

    LOG() << "emit logonDone()";
    emit logonDone( ok );
}

void OTSyncObject::waitForAbort()
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::waitForAbort";
}

void OTSyncObject::fetchChangesSince( const QDateTime &_timestamp )
{
    // Outlook timestamps are in local time
    QDateTime timestamp = _timestamp.toLocalTime();
    TRACE(OutlookSyncPlugin) << "OTSyncObject::fetchChangesSince" << "timestamp" << timestamp;
    QStringList leftover = rememberedIds;
    Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
    Outlook::_ItemsPtr items = folder->GetItems();
    int max = items->GetCount();
    LOG() << "iterating over" << max << "items";
    QStringList seenIds;
    if ( abort ) return;
    for ( int i = 0; i < max; i++ )  {
        QString id;
        QDateTime lm;
        long item_to_get = i+1;
        q->getProperties( items->Item(item_to_get), id, lm );
        if ( id.isEmpty() ) // A COM error occurred while reading this item!
            continue;
        LOG() << "I've seen id" << id << "last modified" << lm;
        seenIds << id;
        if ( rememberedIds.contains(id) )
            leftover.removeAt( leftover.indexOf(id) );
        // Skip items that are of the wrong class
        if ( !q->isValidObject(items->Item(item_to_get)) ) {
            LOG() << "Item is not valid";
            continue;
        }
        // New items need to be synced even if their last modified timestamp
        // is before the actual timestamp because when Outlook imports items
        // from another .pst it does not update the last modified timestamp.
        bool new_item = timestamp.isNull() || !rememberedIds.contains(id);
        if ( new_item || lm >= timestamp ) {
            QBuffer buffer;
            buffer.open( QIODevice::WriteOnly );
            {
                // scoped to ensure everything gets flushed out
                QXmlStreamWriter stream( &buffer );
                stream.writeStartDocument();
                q->dump_item(items->Item(item_to_get), stream);
                stream.writeEndDocument();
            }
            LOG() << "remembered?" << (bool)rememberedIds.contains(id);
            if ( new_item ) {
                setPreviousBuffer(id, buffer.buffer());
                emit createServerRecord( buffer.buffer() );
            } else {
                if (buffer.buffer() != previousBuffer(id)) {
                    setPreviousBuffer(id, buffer.buffer());
                    emit replaceServerRecord( buffer.buffer() );
                } else {
                    LOG() << "not changed, suppress";
                }
            }
        }
        if ( abort ) return;
    }
    if ( !timestamp.isNull() ) {
        foreach ( const QString &id, leftover ) {
            setPreviousBuffer(id, QByteArray());
            emit removeServerRecord( id );
            if ( abort ) return;
        }
    }
    rememberedIds = seenIds;
    LOG() << "rememberedIds" << rememberedIds;
    emit serverChangesCompleted();
}

void OTSyncObject::createClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::createClientRecord";
    LOG() << "record" << record;

    QString clientid;
    bool local;
    bool ok = q->getIdentifier( record, clientid, local );
    LOG() << "getIdentifier() returned" << ok << "clientid" << clientid << "local" << local;
    if (!ok) return;

    QString entryid;

    if ( local ) {
        entryid = clientid;
        LOG() << "This is a known record with entryid" << entryid;
        WARNING() << "BUG: Got a create with a known identifier!";
        return;
    } else {
        LOG() << "This is an unknown record with clientid" << clientid;
    }

    IDispatchPtr dispatch = findItem( entryid );

    entryid = q->read_item( dispatch, record );
    LOG() << "read_item() returned entryid" << entryid;
    if ( entryid.isEmpty() ) {
        WARNING() << "BUG: Could not add/update item!!!";
        return;
    }
    // Set the buffer here so we don't get confused the next time we read from Outlook
    setPreviousBuffer(entryid, record);

    rememberedIds << entryid;
    LOG() << "rememberedIds" << rememberedIds;
    emit mappedId( entryid, clientid );
}

void OTSyncObject::replaceClientRecord(const QByteArray &record)
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::replaceClientRecord";
    LOG() << "record" << record;

    QString clientid;
    bool local;
    bool ok = q->getIdentifier( record, clientid, local );
    LOG() << "getIdentifier() returned" << ok << "clientid" << clientid << "local" << local;
    if (!ok) return;

    QString entryid;

    if ( local ) {
        entryid = clientid;
        LOG() << "This is a known record with entryid" << entryid;
    } else {
        LOG() << "This is an unknown record with clientid" << clientid;
        WARNING() << "BUG: Got a replace with an unknown identifier!";
        return;
    }

    IDispatchPtr dispatch = findItem( entryid );

    entryid = q->read_item( dispatch, record );
    LOG() << "read_item() returned entryid" << entryid;
    if ( entryid.isEmpty() ) {
        WARNING() << "BUG: Could not add/update item!!!";
        return;
    }
    // Set the buffer here so we don't get confused the next time we read from Outlook
    setPreviousBuffer(entryid, record);
}

void OTSyncObject::removeClientRecord(const QString &identifier)
{
    TRACE(OutlookSyncPlugin) << "OTSyncObject::removeClientRecord" << "identifier" << identifier;
    Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
    Outlook::_ItemsPtr items = folder->GetItems();
    int max = items->GetCount();
    bool found = false;
    for ( int i = 0; i < max; ++i ) {
        QString id;
        QDateTime lm;
        long item_to_get = i+1;
        // Skip items that are of the wrong class
        if ( !q->isValidObject(items->Item(item_to_get)) ) {
            LOG() << "Item is not valid";
            continue;
        }
        q->getProperties( items->Item(item_to_get), id, lm );
        if ( id.isEmpty() ) // A COM error occurred while reading this item!
            continue;
        if ( id == identifier ) {
            found = true;
            q->delete_item( items->Item(item_to_get) );
            rememberedIds.removeAt( rememberedIds.indexOf(id) );
            LOG() << "rememberedIds" << rememberedIds;
            // Set the buffer here so we don't get confused the next time we read from Outlook
            setPreviousBuffer(id, QByteArray());
            break;
        }
    }
    if ( !found )
        LOG() << "Could not find item" << identifier << "to remove!";
}

IDispatchPtr OTSyncObject::findItem( const QString &entryid )
{
    TRACE(OutlookSyncPlugin) << "OutlookSyncPlugin::findItem" << "entryid" << entryid;

    IDispatchPtr disp;

    if ( !entryid.isEmpty() ) {
        Outlook::MAPIFolderPtr folder = o->ns->GetDefaultFolder(q->folderEnum());
        Outlook::_ItemsPtr items = folder->GetItems();
        int max = items->GetCount();
        for ( int i = 0; i < max; i++ )  {
            QString id;
            QDateTime lm;
            long item_to_get = i+1;
            // Skip items that are of the wrong class
            if ( !q->isValidObject(items->Item(item_to_get)) ) {
                LOG() << "Item is not valid";
                continue;
            }
            q->getProperties( items->Item(item_to_get), id, lm );
            if ( id.isEmpty() ) // A COM error occurred while reading this item!
                continue;
            if ( entryid == id ) {
                disp = items->Item(item_to_get);
                break;
            }
        }
    }

    if ( disp ); else {
        LOG() << "Item not found... creating a new item";
        LOG() << "disp = o->ap->CreateItem(" << q->itemEnum() << ")";
        disp = o->ap->CreateItem(q->itemEnum());
        LOG() << "init_item( disp )";
        q->init_item( disp );
    }

    return disp;
}

QByteArray OTSyncObject::previousBuffer( const QString &id )
{
    CacheSettings settings(q->id());
    settings.beginGroup("cache");
    return settings.value(id).toString().toLocal8Bit();
}

void OTSyncObject::setPreviousBuffer( const QString &id, const QByteArray &data )
{
    CacheSettings settings(q->id());
    settings.beginGroup("cache");
    if (data.count())
        settings.setValue(id, QString(data));
    else
        settings.remove(id);
}

