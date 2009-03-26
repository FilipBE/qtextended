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
#ifndef QDPLUGIN_H
#define QDPLUGIN_H

#include <qdplugindefs.h>

#include <qdglobal.h>

#include <qplugin.h>
#include <QObject>
#include <QString>
#include <QIcon>
#include <QDateTime>

class PluginManager;
class QTextStream;
class QIODevice;

// ====================================================================


class QD_EXPORT QDPluginFactory : public QObject
{
    Q_OBJECT
public:
    QDPluginFactory( QObject *parent = 0 );
    virtual ~QDPluginFactory();

    virtual QString executableName() const = 0;
    virtual QStringList keys() const = 0;
    virtual QDPlugin *create( const QString &key ) = 0;
};


// ====================================================================


class QD_EXPORT QDPlugin : public QObject
{
    friend class PluginManager;
    friend class QtopiaDesktopApplication;

    Q_OBJECT
public:
    QDPlugin( QObject *parent = 0 );
    virtual ~QDPlugin();

    CenterInterface *centerInterface();
    virtual QString id() = 0;
    virtual QString displayName() = 0;

    virtual void init();

    void lock( QDPlugin *plugin );
    void unlock( QDPlugin *plugin );
    bool locked() const;

protected:
    virtual void internal_init();
    QDPluginData *d;
};


// ====================================================================


class QD_EXPORT QDAppPlugin : public QDPlugin
{
    Q_OBJECT
public:
    QDAppPlugin( QObject *parent = 0 );
    virtual ~QDAppPlugin();

    virtual QIcon icon() = 0;
    virtual QWidget *initApp() = 0;
    virtual QWidget *initSettings();
};


// ====================================================================


class QD_EXPORT QDLinkPlugin : public QDPlugin
{
    Q_OBJECT
public:
    QDLinkPlugin( QObject *parent = 0 );
    virtual ~QDLinkPlugin();

    virtual bool tryConnect( QDConPlugin *con ) = 0;
    virtual void stop() = 0;
    virtual int ping_interval();
    virtual bool send_ack();

    enum State {
        Up = 0,
        Down = 1,
    };
signals:
    void setState( QDLinkPlugin *plugin, int state );

public:
    QIODevice *socket();
protected:
    void setupSocket( QIODevice *device );

private slots:
    void removeHelper();
};


// ====================================================================


class QD_EXPORT QDConPlugin : public QDPlugin
{
    Q_OBJECT
public:
    QDConPlugin( QObject *parent = 0 );
    virtual ~QDConPlugin();

    // This is for TCP/IP based connections
    virtual int port() = 0;

    virtual QString conProperty( const QString &key ) = 0;
    virtual QDDevPlugin *device() = 0;
    virtual bool claim( QDDevPlugin *dev ) = 0;
    virtual void connected( QDDevPlugin *dev ) = 0;
    virtual bool tryConnect( QDLinkPlugin *link ) = 0;
    virtual void stop() = 0;

    enum State {
        Connected = 0,
        Disconnected = 1,
        Connecting = 2,
        Matching = 3,
    };

signals:
    void setState( QDConPlugin *plugin, int state );
};


// ====================================================================


class QD_EXPORT QDDevPlugin : public QDPlugin
{
    Q_OBJECT
public:
    QDDevPlugin( QObject *parent = 0 );
    virtual ~QDDevPlugin();

    virtual QString model() = 0;
    // This is typically "Qtopia" but a fork could use their own name (eg. OPIE)
    virtual QString system() = 0;
    virtual quint32 version() = 0;
    virtual QString versionString() = 0;
    virtual QPixmap icon() = 0;
    virtual QDConPlugin *connection() = 0;

    // Doers

    virtual void probe( QDConPlugin *con ) = 0;
    virtual void disassociate( QDConPlugin *con ) = 0;
};


// ====================================================================


class QD_EXPORT QDSyncPlugin : public QDPlugin
{
    Q_OBJECT
public:
    QDSyncPlugin( QObject *parent = 0 );
    virtual ~QDSyncPlugin();

    virtual QString dataset() = 0;
    virtual QByteArray referenceSchema();
    virtual void prepareForSync();
    virtual void finishSync();

signals:
    void readyForSync( bool ready );
    void finishedSync();
};


// ====================================================================


class QD_EXPORT QDClientSyncPlugin : public QDSyncPlugin
{
    Q_OBJECT
public:
    QDClientSyncPlugin( QObject *parent = 0 );
    virtual ~QDClientSyncPlugin();

public slots:
    virtual void serverSyncRequest(const QString &source) = 0;
    virtual void serverIdentity(const QString &server) = 0;
    virtual void serverVersion(int major, int minor, int patch) = 0;
    virtual void serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync) = 0;
    virtual void createServerRecord(const QByteArray &record) = 0;
    virtual void replaceServerRecord(const QByteArray &record) = 0;
    virtual void removeServerRecord(const QString &serverId) = 0;
    virtual void requestTwoWaySync() = 0;
    virtual void requestSlowSync() = 0;
    virtual void serverError() = 0;
    virtual void serverEnd() = 0;

signals:
    void clientIdentity(const QString &id);
    void clientVersion(int major, int minor, int patch);
    void clientSyncAnchors(const QDateTime &clientLastSync, const QDateTime &clientNextSync);
    void createClientRecord(const QByteArray &record);
    void replaceClientRecord(const QByteArray &record);
    void removeClientRecord(const QString &clientId);
    void mappedId(const QString &serverId, const QString &clientId);
    void clientError();
    void clientEnd();
};


// ====================================================================


class QD_EXPORT QDClientSyncPluginFactory : public QDPlugin
{
    Q_OBJECT
public:
    QDClientSyncPluginFactory( QObject *parent = 0 );
    virtual ~QDClientSyncPluginFactory();

    virtual QStringList datasets() = 0;
    virtual QDClientSyncPlugin *pluginForDataset( const QString &dataset ) = 0;
};



// ====================================================================


class QD_EXPORT QDServerSyncPlugin : public QDSyncPlugin
{
    Q_OBJECT
public:
    QDServerSyncPlugin( QObject *parent = 0 );
    virtual ~QDServerSyncPlugin();

    virtual void fetchChangesSince(const QDateTime &timestamp) = 0;
    virtual void createClientRecord(const QByteArray &record) = 0;
    virtual void replaceClientRecord(const QByteArray &record) = 0;
    virtual void removeClientRecord(const QString &identifier) = 0;

    virtual void beginTransaction(const QDateTime &timestamp) = 0;
    virtual void abortTransaction() = 0;
    virtual void commitTransaction() = 0;

signals:
    void mappedId(const QString &clientId, const QString &serverId);
    void createServerRecord(const QByteArray &record);
    void replaceServerRecord(const QByteArray &record);
    void removeServerRecord(const QString &identifier);
    void serverChangesCompleted();
    void serverError();
};


#endif
