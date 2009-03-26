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

#ifndef SXEMONITOR_H
#define SXEMONITOR_H

#include <qobject.h>
#include <qstring.h>
#include <qfilesystemwatcher.h>
#include <qfile.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>

struct SxeLogEntry
{
        SxeLogEntry();
        enum LogEntryType{ Breach, FARExceeded, Incomplete, NonSxe};
        int pid;
        int progId;
        QString request;
        QString exe;
        QString cmdline;
        LogEntryType logEntryType;
        bool isValid()
        {
            return ((pid != -1 &&
                     progId != -1 &&
                     !exe.isEmpty() &&
                     logEntryType == Breach)
                     ||
                     (logEntryType == FARExceeded)
                     );
        }
        void reset();
    SxeLogEntry& parseLogEntry( QString logEntry );

    private:
        static QString stampFormat();
        static const QString sxeTag;
        static const QString farExceeded;
};

class SxeMonitor : public QObject
{
    Q_OBJECT
public:
    SxeMonitor();
    ~SxeMonitor();

    struct AppInfo
    {
        enum AppType
        {
            System,
            Sandboxed,
            QuicklaunchSystem,
            //QuicklaunchSandboxed (not supported yet)
            AllSandboxed,
            Other,
            Incomplete
        };

        AppInfo();
        AppType appType;
        QString identifier;
        QString userVisibleName;
        QString executable;
        int pid;
        bool isValid() {
            return ( (appType != AllSandboxed
                        && appType != Incomplete
                        && !identifier.isEmpty()
                        && !userVisibleName.isEmpty()
                        && !executable.isEmpty()
                        && pid != -1)
                     ||
                     (appType == AllSandboxed)
                   );
        }

        AppInfo& getAppInfo( const int procId, const QString exePath,
                                            const QString cmdline="" );
        QString toString() const;
        void reset();
    };


private slots:
    void init();
    void logUpdated();
    void sendDelayedReq();
    void discardKilledPid();

private:
    enum MessageType
    {
      DialogBreach,
      DialogLockdown,
      SmsPackageAppBreach,
      SmsSystemAppBreach,
      SmsLockdown
    };

    SxeMonitor( const SxeMonitor & );
    SxeMonitor &operator=( SxeMonitor const & );

    void killApplication( AppInfo app );
    void disableSandboxedApplication( AppInfo app ) const;
    void lockdown();
    void dispatchMessage( const MessageType type, const QStringList &args, bool delay );

    void processLogFile();
    QString readCmdline( int pid ) const;

    QString packagemanagerName() const;
    QFileSystemWatcher *logWatcher;

    QString logPath;
    QFile logFile;

    QList<QtopiaServiceRequest> delayedRequests;
    QString smsProgName;
    QString smsRequest;

    QList<QVariant> killedPids;
    QValueSpaceObject sxeVso;


    static const QString lidsTag;
    static const QString disabledTag;

    static const int maxRetries;
#ifdef QT_NO_QWS_VFB
    static qint64 maxLogSize;
    QString lidsStampFormat;
#endif

#ifdef TEST_SXEMONITOR
    friend class tst_SxeMonitor;
#endif
};

#endif
