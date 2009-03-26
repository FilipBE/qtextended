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
#ifndef QPACKAGEREGISTRY_H
#define QPACKAGEREGISTRY_H

#include <QStringList>
#include <QSettings>
#include <QHash>
#include <QCache>

#ifndef SXE_INSTALLER
#include <qtransportauth_qws.h>
#endif

#include <stdio.h>
# include <sys/types.h>
# include <sys/ipc.h>

#include <qtopiaglobal.h>
#include <qtopiasxe.h>

class QSettings;
class AuthCookie;
class ProgramInfo;
class QStorageMetaInfo;
class AuthRecord;

/**
  Manages central package binary information store.
*/
class QTOPIASECURITY_EXPORT QPackageRegistry : public QObject
{
    Q_OBJECT
public:
    enum RegistryFile {
        Keyfile,
        KeyfileSequence,
        Manifest,
        Installs,
        Policy
    };

    QPackageRegistry();
    virtual ~QPackageRegistry();

    static const QString packageDirectory;
    static const QString installsFileName;
    static const QString manifestFileName;
    static const QString policyFileName;
    static const QString profilesFileName;
    static const QString binInstallPath;
    static const QString qlLibInstallPath;
    static const QString procLidsKeysPath;
    static const int maxProgId;

    static QPackageRegistry* getInstance();

    virtual QString sxeConfPath() const;

    void initProgramInfo( SxeProgramInfo &pi );
    bool unregisterPackageBinaries( const QString &packagePath );

    int bootstrap( const QString & );
#ifndef SXE_INSTALLER
    bool isInstalledProgram( QTransportAuth::Data & );
#endif
    const unsigned char *getClientKey( unsigned char );
    QCache<unsigned char, unsigned char> clientKeys;

public slots:
    void registerBinary( SxeProgramInfo &pi );
    void lockManifest( bool & );
    void unlockManifest();

private:
    void outputPolicyFile();
    void initialiseInstallDicts();
    bool openSystemFiles();
    void closeSystemFiles();
    void readManifest();
    void readInstalls();
    void clearInstalls();
    void updateProcKeyFile( const SxeProgramInfo &, int );
    void addToManifest( const SxeProgramInfo &, int );
    static QString getQtopiaDir();
    QVariant removeFromRegistry( const QVariant &checkList, RegistryFile type, bool &result );

    int manifestFd, installFd;
    int curInstallId;
    FILE *installStrm;
    int connectionCount;

    QHash<unsigned short, ProgramInfo*> installs;
    QHash<unsigned char,AuthRecord*> progDict;
    QHash<QString,unsigned char> profileDict;
    QMap<QString,ProgramInfo*> installsByPath;

#ifdef SXE_INSTALLER
    static QString currentInstallRoot;
#else
    QStorageMetaInfo *storageInfo;
#endif
    QFile *manifestLockFile;
    friend class SXEPolicyManager;
};

#endif
