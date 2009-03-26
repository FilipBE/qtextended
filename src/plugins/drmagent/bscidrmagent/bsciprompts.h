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
#ifndef BSCIPROMPTS_H
#define BSCIPROMPTS_H
#include <bsci.h>
#include <bsciMMI.h>
#include <QContent>
#include <QValueSpaceItem>

class BSciPrompts : public QObject
{
    Q_OBJECT
public:
    enum DomainAction
    {
        JoinDomain,
        UpgradeDomain,
        LeaveDomain
    };

    enum ActivationReason
    {
        Open,
        Expired,
        SingleRemainingUse
    };

    static BSciPrompts *instance();

    SBSciCallbacks *callbacks();

    void notifySuccess( SBSciRoapStatus *triggerStatus ) const;
    void notifyFailure( SBSciRoapStatus *triggerStatus, int error ) const;

    // Activation failure messages.
    void notifyCannotActivate( const QContent &content, QDrmRights::Permission permission, ActivationReason reason ) const;
    void notifyUseEmbeddedPreview( const QContent &content ) const;
    void notifyFutureRights( const QContent &content, const QDateTime &date, QDrmRights::Permission permission ) const;

    // Unprompted activation messages.
    void notifyContentAvailable( const QStringList &fileNames ) const;

    void notifyRightsObjectsReceived( const QStringList &aliases ) const;
    void notifyRegistrationSuccess( const QString &alias, const QString domain ) const;
    void notifyDomainSuccess( const QString &alias, DomainAction action ) const;

    void notifyRegistrationFailure( const QString &alias, const QString domain, const QString &error ) const;
    void notifyDomainFailure( const QString &alias, DomainAction action, const QString &error ) const;

    void notifyError( const QString &error ) const;

    bool requestPreviewPermission( const QContent &content ) const;
    bool requestDomainPermission( const QString &alias, DomainAction action ) const;
    bool requestRegistrationPermission( const QString &alias, const QString &url ) const;
    bool requestRightsObjectPermission( const QString &object, const QString &issuer ) const;
    bool requestOpenUrl( const QContent &content, const QString &url ) const;
    bool requestOpenUrl( const QContent &content, const QString &url, QDrmRights::Permission permission, ActivationReason reason ) const;
    bool requestOpenDomainUrl( const QContent &content, const QString &url ) const;
    bool requestOpenRegistrationUrl( const QContent &content, const QString &url ) const;
    bool requestSilentROPermission( const QContent &content, const QString &url ) const;
    bool requestPreviewROPermission( const QContent &content, const QString &url ) const;

    bool silentRoapEnabled() const;

private slots:
    void _question( const QString &title, const QString &message );
    void _information( const QString &title, const QString &message );
    void silentRoapChanged();

private:
    BSciPrompts();
    int question( const QString &title, const QString &message ) const;
    void information( const QString &title, const QString &message ) const;

    static QString formatError( int error );

    QString permissionString( QDrmRights::Permission ) const;

    static int notify_sd_ro(const char *fileName, SBSciRights *rights);

    static int notify_v2_install(SBSciRoapStatus *triggerStatus, int success, int silent);

    static int allow_domain_join(const char *filename, const char *domainID, const char *domainAlias);

    static int allow_domain_leave(const char *domainID, const char *domainAlias);

    static int allow_register_agent(const char *riID, const char *riAlias,  const char *riUrl);

    static int allow_acquire_ro(const char *roID, const char *roAlias,  const char *riAlias);

    static int store_dcf(const char *tmp_filename, char *stored_filename);

    static int browse_rights_issuer( const char *fileURI, const char *rightsIssuerURL, ERightsStatus status );

    static int browse_join_domain( const char *fileURI, const char *domainID, const char *riURL, ERightsStatus *status );

    static int browse_register( const char *fileURI, const char *riURL, ERightsStatus status );

    static int allow_preview_download( const char *fileURI, const char *previewURL );

    static int allow_silent_download( const char *fileUri, const char *silentURL );

    static int allow_time_sync( int onRORITS, SBSciTime *currentDRMTime );

    static void time_sync_status( int  status );

    static SBSciCallbacks m_callbacks;

    int m_response;
    QValueSpaceItem m_silentRoap;
    bool m_silentRoapEnabled;
};


#endif
