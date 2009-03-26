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

#ifndef BSCIDRMCONTENTPLUGIN_H
#define BSCIDRMCONTENTPLUGIN_H

#include <QDrmContentPlugin>
#include "bscidrm.h"
#include <qtopiaglobal.h>
#include <QValueSpaceItem>


class BSciContentLicense : public QDrmContentLicense
{
    Q_OBJECT
public:
    BSciContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options, TFileHandle f, SBSciContentAccess *ops );
    virtual ~BSciContentLicense();

    virtual QContent content() const;
    virtual QDrmRights::Permission permission() const;

    virtual QDrmContent::RenderState renderState() const;

public slots:
    virtual void renderStateChanged( const QContent &content, QDrmContent::RenderState state );

protected:
    void startConstraintUpdates();
    void pauseConstraintUpdates();
    void stopConstraintUpdates();
    void expireLicense();

    void timerEvent( QTimerEvent * event );

private:
    QContent m_content;
    QDrmRights::Permission m_permission;
    QDrmContent::RenderState m_renderState;
    QDrmContent::LicenseOptions m_options;

    TFileHandle file;
    SBSciContentAccess *fileOps;

    int timerId;
    QDateTime lastUpdate;
};

class BSciPreviewLicense : public QDrmContentLicense
{
    Q_OBJECT
public:
    BSciPreviewLicense( const QContent &content );
    virtual ~BSciPreviewLicense();

    virtual QContent content() const;
    virtual QDrmRights::Permission permission() const;

    virtual QDrmContent::RenderState renderState() const;

public slots:
    virtual void renderStateChanged( const QContent &content, QDrmContent::RenderState state );

private:
    QContent m_content;
    QDrmContent::RenderState m_renderState;
};

class BSciReadDevice : public QIODevice
{
    Q_OBJECT
public:
    BSciReadDevice( const QString &drmFile, QDrmRights::Permission permission );

    ~BSciReadDevice();

    bool open( QIODevice::OpenMode mode );
    void close();
    bool seek( qint64 pos );
    qint64 size() const;

protected:
    qint64 readData( char * data, qint64 maxlen );
    qint64 writeData( const char * data, qint64 len );
private:
    const QByteArray fileName;
    QDrmRights::Permission permission;

    TFileHandle file;
    SBSciContentAccess *fileOps;
};

class BSciDrmContentPlugin : public QDrmContentPlugin
{
    Q_OBJECT
public:
    BSciDrmContentPlugin( QObject *parent = 0 );

    virtual ~BSciDrmContentPlugin();

    virtual QStringList keys() const;

    virtual QStringList types() const;

    virtual QList< QPair< QString, QString > > httpHeaders() const;

    virtual bool isProtected( const QString &filePath ) const;

    virtual QDrmRights::Permissions permissions( const QString &filePath );

    virtual bool activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus );

    virtual void activate( const QContent &content, QWidget *focus );

    virtual void reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus );

    virtual QDrmRights getRights( const QString &filePath, QDrmRights::Permission permission );

    virtual QDrmContentLicense *requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options );

    virtual QIODevice *createDecoder( const QString &filePath, QDrmRights::Permission permission );

    virtual bool canActivate( const QString &filePath );

    virtual qint64 unencryptedSize( const QString &filePath );

    virtual QAbstractFileEngine *create( const QString &fileName ) const;

    virtual bool deleteFile( const QString &filePath );

    virtual bool installContent( const QString &filePath, QContent *content );

    virtual bool updateContent( QContent *content );

protected:
    enum MetaData
    {
        ContentType,
        ContentUrl,
        ContentVersion,
        Title,
        Description,
        Copyright,
        Author,
        IconUri,
        InfoUrl,
        RightsIssuerUrl
    };

    bool installMessageFile( const QString &filePath );

    bool registerFile( const QString &filePath );

    QMap< MetaData, QString > getMetaData( const QString &filePath );

private slots:
    void transactionTrackingChanged();

private:
    QValueSpaceItem m_transactionTracking;
};

#endif
