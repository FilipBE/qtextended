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

#include "sxemonitor.h"
#include "sxemonqlog.h"
#include <qsettings.h>
#include <qtopianamespace.h>
#include <sys/syslog.h>
#include <qtopialog.h>
#include <qprocess.h>
#include <qcontent.h>
#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <signal.h>

static int sxe_kill_pid( pid_t pid )
{
#ifdef SXE_SOFT_KILL
    static int sig = 0;
    if ( sig == 0 ) {
        QByteArray soft_kill = qgetenv("SXE_SOFT_KILL");
        if ( soft_kill.count() )
            sig = QVariant(soft_kill).toInt();
        if ( sig == 0 )
            sig = 9;
    }
    return kill( pid, sig );
#else
    return kill( pid, 9 );
#endif
}

const QString SxeLogEntry::sxeTag ="<SXE Breach>"; //used for log entries that are sxe related
const QString SxeLogEntry::farExceeded = "FAR_Exceeded";

SxeLogEntry::SxeLogEntry()
{
    reset();
}
/*
   \internal
   Modifies this SxeLogEntry based on the /a logEntry
   from the security log file
*/
SxeLogEntry& SxeLogEntry::parseLogEntry( QString logEntry )
{
    reset();
    QString stamp;
    QString details;

    if( !logEntry.contains( sxeTag ) )
    {
        logEntryType = SxeLogEntry::NonSxe;
        return *this;
    }
    QRegExp regExp( stampFormat() );
    regExp.setMinimal( true );
    regExp.indexIn( logEntry );
    QStringList matches =  regExp.capturedTexts();
    if ( matches.count() != 3 )
    {
        qWarning( "SxeMonitor:- could not parse log entry, entry not of expected format");
        return *this;
    }
    stamp = matches[1];
    details = matches[2];
    qLog(SxeMonitor) << "SxeLogEntry::parseLogEntry stamp="  << stamp << "  details=" << details;

    if ( !details.startsWith( sxeTag ) )
    {
            qLog(SxeMonitor) << "Sxe Tag being logged but not in security breach context";
            logEntryType = SxeLogEntry::NonSxe;
            return *this;
    }
    details = details.mid( sxeTag.length() );

    if ( details.isEmpty() )
    {
        qWarning() << QLatin1String("SxeMonitor:- security sxe log entry invalid, "
                                    "no breach details:" );
        logEntryType = SxeLogEntry::Incomplete;
        return *this;
    }

    if ( details.simplified() == farExceeded )
    {
        logEntryType = SxeLogEntry::FARExceeded;
        return *this;
    }

    QStringList parts;
    parts = details.split( " // ", QString::SkipEmptyParts );
    if ( parts.count() < 4 )
    {
        logEntryType = SxeLogEntry::Incomplete;
        qWarning( "SxeMonitor:- log entry does not have the requisite number of parts" );
        return *this;
    }

    logEntryType = SxeLogEntry::Breach;
    QString field;
    bool ok=false;
    bool isError=false;

    for( int i = 0; i < 5; ++i )
    {
        field = parts[i].mid( parts[i].indexOf(":") + 1 );
        switch (i)
        {
            case 0: //PID
                pid = field.toLong(&ok);
                if ( !ok || pid < 0 )
                {
                    qWarning() << QLatin1String("SxeMonitor:- log entry PID field is invalid: " );
                    pid = -1;
                    isError = true;
                }
                break;
            case 1://ProgId
                progId = field.toLong( &ok );
                if ( !ok || progId < 0 )
                {
                    qWarning() << QLatin1String("SxeMonitor:- log entry ProgId field is invalid: ");
                    progId = -1;
                    isError = true;
                }
                break;
            case 2://Exe
                exe = field;
                if ( exe.isEmpty() )
                {
                    qWarning() << QLatin1String( "SxeMonitor:- log entry Exe field is empty:" );
                    isError = true;
                }
                break;
            case 3://Request
                request = field;
                //note: an empty request field is a valid but odd circumstance
                if ( request.isEmpty() )
                {
                    qWarning() << QLatin1String( "SxeMonitor:- log entry Request field is empty: " );
                }
                break;
            case 4://cmdline
                cmdline = field;
                //note: an empty cmdline field is a valid but odd circumstance
                if ( cmdline.isEmpty() )
                {
                    qWarning() << QLatin1String( "SxeMonitor:- log entry cmdline field is empty: " );
                }
                break;
        }
        if ( isError )
        {
            logEntryType = SxeLogEntry::Incomplete;
            break;
        }
    }
    return *this;
}

void SxeLogEntry::reset()
{
    logEntryType = SxeLogEntry::Incomplete;
    pid = -1;
    progId = -1;
    request = QString::null;
    exe = QString::null;
    cmdline = QString::null;
}

QString SxeLogEntry::stampFormat()
{
    static QString stampFmt;
    if ( stampFmt.isEmpty() )
    {
        // For unit tests, use the settings set in user scope, so the
        // unit test can modify it.
        QSettings sxeMonitorConf(
#ifndef QTOPIA_TEST
               Qtopia::qtopiaDir() + "etc/default/Trolltech/Sxe.conf",
#endif
               QSettings::NativeFormat
#ifdef QTOPIA_TEST
               , QSettings::UserScope, "Trolltech", "Sxe"
#endif
        );
        sxeMonitorConf.beginGroup( "Log" );
        stampFmt = sxeMonitorConf.value( "StampFormat" ).toString();
        if ( stampFmt.isEmpty() )
            qWarning() << "Sxe configuration file does not have a LogStampFormat";
    }
    return stampFmt;
}

const QString SxeMonitor::lidsTag= "LIDS:"; //used for log entries that are LIDS related
const QString SxeMonitor::disabledTag = "__DISABLED"; //used for tagging symlinks as disabled
const int SxeMonitor::maxRetries = 10;

#ifdef QT_NO_QWS_VFB
qint64 SxeMonitor::maxLogSize = 1048576; //number of bytes
#endif

/*!
  \internal
  \class SxeMonitor
    \inpublicgroup QtPkgManagementModule
  \brief Monitors sxe policy breaches and system resources

  The sxemonitor is a Qt Extended daemon process that monitors application
  level sxe policy breaches and takes appropriate action upon detection
  of a breach.  The current response for breaches is to kill all instances
  of the errant application, disable the application, and notify
  what happened via dialog and system SMS message.

  In future it is expected the sxemonitor's responsibilities will
  expand to monitoring system resources such as the framebuffer write lock,
  memory usage etc.
*/

/**
  Implementation Notes:
  The sxemonitor carries out its duties by monitoring a log file
  (usually /var/log/sxe.log) which is  updated whenever a security breach occurs.
  Other events are also sent to this log and so the sxemonitor must
  filter these out.  Those log entries which are security related will
  have a have a tag, <SXE Breach>, as part of the entry.  A security log entry
  will look similar what is shown below.

Sep  7 01:08:20 localhost qpe: <SXE Breach> // PID:19047 // ProgId:29 // Exe:/home/user/build/qtextended/image/bin/mediaserver
// Request:QCopRegisterChannel/QCop/RegisterChannel/QPE/QStorage // Cmdline:/home/user/build/qtextended/image/bin/mediaserver

  For development on desktop using qvfb, the expected log file
  is /var/log/sxe.log.  Log messages of priority local6.err
  should be redirected into this log file.  A different log file
  may be specified as a value for LogPath in Sxe.conf
*/


/*!
  \internal
  \enum SxeMonitor::MessageType
  This enum specifies different message types that can be dispatched
  \value DialogBreach
         Informs user of application level breach via dialog
  \value DialogLockdown
         Informs user of lockdown via dialog
  \value SmsBreach
         Informs user of details of security breach via SMS
  \value SmsLockdown
         Informs user of lockdown via SMS

  \sa dispatchMessage()
*/

/*!
  \internal
  Constructs a new SxeMonitor.  Client code should never construct this,
  instead use the getInstance() method.
*/
SxeMonitor::SxeMonitor() : QObject(), sxeVso("/Sxe",this )
{
    QTimer::singleShot(0, this, SLOT(init()) );
}

/*!
 \internal
  This desctructor should never be called
*/
SxeMonitor::~SxeMonitor()
{
}

/*!
  \internal
  The init method's main duties is to setup a log watcher on the
  security log file.  On a device, the maximum size of the log
  is setup so that rotations are handled correctly.
*/
void SxeMonitor::init()
{
   // For unit tests, use the settings set in user scope, so the
   // unit test can modify it.
    QSettings sxeMonitorConf(
#ifndef QTOPIA_TEST
            Qtopia::qtopiaDir() + "etc/default/Trolltech/Sxe.conf",
#endif
            QSettings::NativeFormat
#ifdef QTOPIA_TEST
            , QSettings::UserScope, "Trolltech", "Sxe"
#endif
    );
    sxeMonitorConf.beginGroup( "Log" );


    logPath = sxeMonitorConf.value( "Path" ).toString();

    if ( logPath.isEmpty() )
    {
        qWarning() << "Sxe.Conf file does not have specify a path to the security log file "
                    "under the Log/Path key.  The SxeMonitor will not respond to security breaches";
        return;
    }

    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::LogPathSet, QStringList( logPath ) );
    logFile.setFileName( logPath );

#ifdef QT_NO_QWS_VFB
    maxLogSize = sxeMonitorConf.value( "Size", 1048576 ).toLongLong();
    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::LogPathSize,
                                                     QStringList() << QString::number(maxLogSize) );
    // force log file creation or rotation if necessary
    if ( (!logFile.exists() || logFile.size() > SxeMonitor::maxLogSize))
    {

        syslog( LOG_ERR | LOG_LOCAL6, "Creating/rotating SXE Security Log" );
        qWarning( "SxeMonitor: Attempting to create/rotate SXE Security Log: %s", qPrintable(SxeMonitor::logPath) );
    }

    lidsStampFormat = sxeMonitorConf.value("LidsStampFormat").toString();
    if ( lidsStampFormat.isEmpty() )
    {
        qWarning( "Sxe.conf does not specify a stamp format for lids messages under the "
                "Log/LidsStampFormat key.  The SxeMonitor will not respond to security breaches");
        return;
    }

#endif

    if( !logFile.open(QFile::ReadOnly) )
    {
        qWarning( "SxeMonitor could not open security log file: %s, it may not have sufficent permissions "
                "or file does not exist.  The SxeMonitor will not respond to security breaches",
                qPrintable(SxeMonitor::logPath) );
        return;
    }

    logFile.seek( logFile.size() );

    logWatcher = new QFileSystemWatcher();
    logWatcher->addPath( logPath );
    connect( logWatcher, SIGNAL(fileChanged(QString)), this, SLOT(logUpdated()) );

    sxeMonitorConf.endGroup();

    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::SuccessInit );
}

/*!
  \internal
  Invoked whenever the security log file has been modified.
  It takes into account log rotations, and if necessary
  forces a rotation and resets the log watcher.
*/
void SxeMonitor::logUpdated()
{
    if ( !logFile.exists() )
        qFatal( "SxeMonitor: security log, %s, does not exist, cannot respond to policy breach",
                 qPrintable( logFile.fileName()));

    processLogFile();

#ifdef QT_NO_QWS_VFB
    static long prevPos = -1;

    //special case of forcing rotation
    if ( logFile.pos() > SxeMonitor::maxLogSize || logFile.pos() == prevPos)
    {
        logFile.close();
        int retries = 2;
        while ( logFile.size() > SxeMonitor::maxLogSize && retries <= SxeMonitor::maxRetries )
        {
            if ( retries == SxeMonitor::maxRetries )
                qWarning( "SxeMonitor: security log could not be rotated" );

            syslog( LOG_ERR | LOG_LOCAL6, "Security Log rotation attempted");

            qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::ForceRot );
            Qtopia::sleep( retries + 2 );
            retries++;
        }

        logWatcher->removePath( SxeMonitor::logPath );
        logWatcher->addPath( SxeMonitor::logPath );

        if ( !logFile.open(QFile::ReadOnly) )
            qFatal( "Could not open security log file: %s", qPrintable(SxeMonitor::logPath) );

        processLogFile();
    }
    prevPos = logFile.pos();
#endif
}

/*!
  \internal

  Sends a message to be displayed by the SystemMessages service.
  The message could be a dialog or an SMS message. The message is delayed
  by 5 seconds if \a delay is true. The types of messages that can be sent
  and their arguments are as follows:

  \table
  \header
    \o \a type
    \o \a args
    \o purpose
  \row
    \o DialogBreach
    \o Application Name
    \o Display dialog to user informing him/her of a security breach and
       imminent termination of the errant application.
  \row
    \o DialogLockdown
    \o none
    \o Display dialog informing user that lockdown is in effect
  \row
    \o SmsBreach
    \o Application name, denied request
    \o Send SMS informing user of details of the security breach
  \row
    \o SmsLockdown
    \o Name of packagemanager
    \o Send SMS informing user that all untrusted applications were
     terminated and disabled during lockdown.
  \endtable
*/
void SxeMonitor::dispatchMessage( const MessageType type, const QStringList &args, bool delay = false )
{
    QtopiaServiceRequest req;
    req.setService( "SystemMessages" );
    req.setArguments( QList<QVariant>() );
    if ( type <= DialogLockdown )
        req.setMessage( "showDialog(QString,QString)" );
    else if ( type <= SmsLockdown )
        req.setMessage( "sendMessage(QString,QString)" );
    else
    {
        qWarning( "SxeMonitor: attempt to dispatch message with invalid message type" );
        return;
    }

    QString msg;
    switch ( type )
    {
        case ( DialogBreach ):
            req << tr("Security Breach");
            msg =  tr("<qt>The application <b>%1</b>"
            ", has breached security policy; <b>it has been terminated</b>.</qt>", "%1 = Application Name"); break;
        case ( DialogLockdown ):
            req << tr("Security Alert");
            msg =  tr("<qt>Suspicious activity detected. All downloaded applications have been <b>shutdown and disabled</b>"
                    " </qt>");
            break;
        case ( SmsPackageAppBreach ):
            req << tr("Security Breach");
            msg =   tr("<qt>The application <b>%1</b> has breached security policy; "
            "it has been <b>shutdown and disabled</b>.<br><br>"
            "Executable: %2 <br><br>"
            "The violation is as follows: %3 </qt>",
            "%1 = Application Name, %2 = Executable, %3 Violation details");
            break;
        case ( SmsSystemAppBreach ):
            req << tr("Security Breach");
            msg =   tr("<qt>The application <b>%1</b> has breached security policy; "
            "it has been <b>shutdown</b>.<br><br>"
            "Executable: %2<br><br>"
            "The violation is as follows: %3</qt>",
           "%1 = Application Name, %2 =Executable, %3 = Violation details");
            break;
        case ( SmsLockdown ):
            req << tr( "Security Alert" );
            msg = tr("<qt>Suspicious activity has been detected. "
                  "All downloaded applications have been terminated and disabled. You can "
                   "uninstall or re-enable them using %1.</qt>" );
            break;
        default:
            qWarning( "SxeMonitor: attempt to dispatch message with invalid message type" );
            return;
    }
    for ( int i = 0; i < args.count(); i++ )
        msg = msg.arg( args[i] );

    req << msg;

    if ( delay )
    {
        delayedRequests.append( req );
        QTimer::singleShot( 5000, this, SLOT(sendDelayedReq()) );
        qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::DelayMsgPrep );

    }
    else
    {
        req.send();
        qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::MsgDispatched );
    }
}

/*!
  \internal

  Helper method for dispatchMessages to send delayed messages which
  have been stored up in the class member \c delayedRequests.
*/
void SxeMonitor::sendDelayedReq()
{
    delayedRequests.takeFirst().send();
    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::DelayMsgDispatched );
}

/*!
  \internal
  Removes the first element off the killedPid list so the list doesn't grow too big
*/
void SxeMonitor::discardKilledPid()
{
    killedPids.removeFirst();
    sxeVso.setAttribute( "killedPids", killedPids );
}

/*!
  \internal
  Given a path to an executable, \a binaryPath, all processes running
  that executable are killed.  If no path is given, all untrusted processes
  are killed
*/
void SxeMonitor::killApplication( AppInfo appInfo )
{
    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::KillApplication,
                            QStringList() << appInfo.executable << appInfo.identifier );

    if ( appInfo.appType ==AppInfo::Incomplete )
    {
        qWarning() << "Sxemonitor:- trying to kill application but have incomplete application info";
        return;
    }

    //if non-qtopia application kill single instance only
    if ( appInfo.appType == AppInfo::Other )
    {
        sxe_kill_pid( appInfo.pid );
        return;
    }
    else if ( appInfo.appType == AppInfo::QuicklaunchSystem
              || appInfo.appType == AppInfo::System )
    {   //kill single instance for system applications but suppress app terminated dialog
        //using valuespace
        killedPids.append( QVariant(appInfo.pid) );
        sxeVso.setAttribute( "killedPids", killedPids );
        sxeVso.sync();

        sxe_kill_pid( appInfo.pid );
        QTimer::singleShot( 1500, this, SLOT(discardKilledPid()) );

        return;
    }

    QString prefixPath;
    if (appInfo.appType == AppInfo::Sandboxed )
    {
        prefixPath = appInfo.executable.left( Qtopia::packagePath().length() + 32 );
    }
    else if ( appInfo.appType == AppInfo::AllSandboxed )
    {
        prefixPath =  Qtopia::packagePath();
    }
    else
    {
        qWarning( "SxeMonitor:- Trying to kill application of invalid type" );
        return;
    }
    QDir proc( "/proc" );
    QStringList procDirs = proc.entryList( QDir::AllDirs, QDir::Name | QDir::Reversed );
    QString exePath;
    bool ok;
    int pid;
    const int numIterations = 3;
    for ( int i = 0; i < numIterations; i++ )
    {
        foreach ( QString dir, procDirs )
        {
            pid = dir.toLong( &ok ); //only want processes
            if ( ok )
            {
                exePath = QFile::symLinkTarget( "/proc/" + dir + "/exe" );
                if ( exePath.startsWith(prefixPath) )
                {
                    killedPids.append( QVariant(pid) );
                    sxeVso.setAttribute( "killedPids", killedPids );
                    sxeVso.sync();
                    sxe_kill_pid( pid );
                    QTimer::singleShot( 1500, this, SLOT(discardKilledPid()) );
                }

            }
        }
    }
}

/*!
  \internal
  Disables a sandboxed program so it cannot start unless it is explicitly enabled,
  The \a md5Sum is the program's md5Sum, if no particular md5Sum is supplied
  then this method will disable all untrusted applications.
*/
void SxeMonitor::disableSandboxedApplication( const AppInfo appInfo ) const
{
    if ( !(appInfo.appType == AppInfo::Sandboxed || appInfo.appType == AppInfo::AllSandboxed) )
    {
        qWarning() << QLatin1String( "SxeMonitor:- Trying to disable non-sandboxed application AppInfo:" )
            << appInfo.toString();
        return;
    }

    QString md5Sum;
    if ( appInfo.appType == AppInfo::Sandboxed )
    {
        md5Sum = appInfo.executable;
        md5Sum = md5Sum.remove( Qtopia::packagePath() ).left( 32 );
    }
    else if ( appInfo.appType == AppInfo::AllSandboxed )
    {
        md5Sum = "";
    }

    QDir packageBin( Qtopia::packagePath() + "bin/" );
    QFileInfoList links = packageBin.entryInfoList( QStringList( md5Sum + "*" ), QDir::Files );

    QFile linkFile;
    if ( !md5Sum.isEmpty() && links.count() == 0 )
    {
        qWarning( "SxeMonitor: Could not disable package, "
                    "no package executable symlinks not found md5=%s", qPrintable(md5Sum) );
        return;
    }

    foreach ( QFileInfo link, links )
    {
        linkFile.setFileName( link.absoluteFilePath() );

        QString oldTarget  = linkFile.symLinkTarget();

        if ( !linkFile.remove() )
            qWarning( "SxeMonitor: Could not disable package, package exectable symlink could not be removed" );

        if ( !QFile::link( oldTarget + SxeMonitor::disabledTag, linkFile.fileName() ) )
            qWarning( "SxeMonitor: Could not disable package, package executable symlink could not be created" );
    }
}


/*!
  \internal

  Performs a lockdown by terminating and disabling all untrusted applications.
*/
void SxeMonitor::lockdown()
{
    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::Lockdown );
    AppInfo appInfo;
    appInfo.appType = AppInfo::AllSandboxed;

    killApplication( appInfo );

    disableSandboxedApplication( appInfo );

    killApplication( appInfo );//redundant safeguard

    dispatchMessage( DialogLockdown, QStringList() );

    dispatchMessage( SmsLockdown, QStringList(packagemanagerName()), true );
}

/*!
  \internal
  Processes the security log file
*/
void SxeMonitor::processLogFile()
{
    qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::Processing );

    QString buf;
    int prevPid = -1;
    int currPid = -1;
    QStringList logEntryParts;
    SxeLogEntry sxeLogEntry;
    QString violation;
    AppInfo appInfo;

    while ( !logFile.atEnd() )
    {
        buf = logFile.readLine().trimmed();

#ifdef QT_NO_QWS_QVFB
        // uid/gid "uniquely" identifies a lids message as security breach related
        if ( buf.contains(SxeMonitor::lidsTag) && buf.contains("uid/gid") ){
            qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::KernelBreach, QStringList(buf) );

            QRegExp regExp( lidsStampFormat );
            regExp.setMinimal( true );
            regExp.indexIn( buf );
            QStringList matches =  regExp.capturedTexts();
            if ( matches.count() != 4 )
            {
                qWarning( "SxeMonitor:- could not parse lids log entry, entry not of expected format");
                return;
            }

            bool ok;
            currPid = matches[2].toInt(&ok);
            if ( !ok || currPid < 0 )
            {
                qWarning( "Sxemonitor: Security LIDS log entry found of invalid format - PID invalid" );
                continue;
            }
            violation = matches[3];

        }
        else
#endif
        if ( sxeLogEntry.parseLogEntry(buf).isValid())
        {
            qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::AppBreach, QStringList(buf) );

            if ( sxeLogEntry.logEntryType == SxeLogEntry::FARExceeded )
            {
                lockdown();
                continue;
            }

            currPid = sxeLogEntry.pid;
            appInfo.getAppInfo( currPid, sxeLogEntry.exe, sxeLogEntry.cmdline );

            violation = tr( "Request for %1 was denied ", "%1 qcop request");
            violation = violation.arg( sxeLogEntry.request );

            qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail,
                                          QStringList() << QString::number(currPid) << sxeLogEntry.request );
        } else
        {
            continue;
        }

        qWarning() << "SxeMonitor detected breach: " << buf;

        if ( currPid != prevPid )
        {
            QFile errantProcExe( QLatin1String("/proc/")  + QString::number( currPid ) + QLatin1String("/exe") );

            if ( errantProcExe.exists() )
            {
                if ( !appInfo.isValid() ) //invalid appInfo at this point means there was a lids breach
                {                         //obtain the information to create an AppInfo
                    QString exePath = errantProcExe.symLinkTarget(); //target of proc/some_pid/exe
                    if ( exePath.isEmpty() )
                    {
                        qWarning( "SxeMonitor:- Could not read exe link target of errant process, "
                                    "process may have already been terminated" );
                        continue;
                    }
                    appInfo.getAppInfo( currPid, exePath, readCmdline(currPid) );
                }

                if( (appInfo.appType== AppInfo::System || appInfo.appType == AppInfo::QuicklaunchSystem ) &&
                    (appInfo.identifier == "sxemonitor"
                     || appInfo.identifier == "sysmessages" || appInfo.identifier == "qpe"))
                {
                    return;
                }
                killApplication( appInfo );
                if ( appInfo.appType == AppInfo::Sandboxed )
                    disableSandboxedApplication( appInfo );
 
                dispatchMessage( DialogBreach, QStringList( appInfo.userVisibleName) );

                if ( appInfo.appType == AppInfo::Sandboxed )
                    dispatchMessage( SmsPackageAppBreach, ( QStringList() << appInfo.userVisibleName
                                                                          << appInfo.executable
                                                                          << violation), true );
                else
                    dispatchMessage( SmsSystemAppBreach, ( QStringList() << appInfo.userVisibleName
                                                                         << appInfo.executable
                                                                         << violation ), true );

                qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail2,
                                                        QStringList() << appInfo.executable << appInfo.userVisibleName );
            }
            else
            {
                qLog( SxeMonitor ) << SxeMonQLog::getQLogString( SxeMonQLog::ProcAlreadyTerminated,
                                                        QStringList() << QString::number(currPid) );
            }
        }
        prevPid= currPid;
    }
}

QString SxeMonitor::readCmdline( int pid ) const
{
    QFile cmdlineFile( QLatin1String("/proc/") + QString::number(pid) + QLatin1String("/cmdline"));
    if ( !cmdlineFile.open( QIODevice::ReadOnly ) )
        qWarning( "SxeMonitor:- Could not read %s", qPrintable(cmdlineFile.fileName()) );
    QString cmdline = cmdlineFile.readAll();

    return cmdline;
}

QString SxeMonitor::packagemanagerName() const
{
    static QString ret="";
    if ( ret.isEmpty() )
    {
        QContentId contentId;
        contentId = QContent::execToContent( "packagemanager" );
        if ( contentId == QContent::InvalidId )
            ret = "packagemanager"; 
        else
           ret = QContent(contentId).name();
    }
    return ret;
}


SxeMonitor::AppInfo::AppInfo()
{
    reset();
}

SxeMonitor::AppInfo& SxeMonitor::AppInfo::getAppInfo(int procId, const QString exePath, QString cmdline )
{
    reset();
    QString contentName;

    if ( exePath.isEmpty() )
    {
        qWarning( "SxeMonitor::getAppInfo exePath parameter is empty" );
        appType = AppInfo::Incomplete;
        return *this;
    }

    if ( procId < 2 )
    {
        qWarning("SxeMonitor::getAppInfo invalid pid: %i", procId );
        appType = AppInfo::Incomplete;
        return *this;
    }
    executable = exePath;
    pid = procId;
    if ( exePath.startsWith( Qtopia::packagePath() ) )
    {
        appType =  AppInfo::Sandboxed;
        QString md5Sum = exePath;
        md5Sum = (md5Sum.remove(Qtopia::packagePath())).left(32);
        identifier = md5Sum;
        contentName = md5Sum + "_" + exePath.mid( exePath.lastIndexOf("/") + 1 );
    }
    else if( exePath.startsWith( Qtopia::qtopiaDir() ) || exePath.startsWith( Qtopia::updateDir() ) )
    {
        if ( exePath.endsWith( QLatin1String("quicklauncher") ) )
        {
            appType = AppInfo::QuicklaunchSystem;
            identifier = cmdline; //note: empty cmdline for quicklaunched apps is a valid but
                                  //odd circumstance, only the process with matching pid is killed
            if ( identifier.contains( "/" ) )
                    identifier = identifier.mid(identifier.lastIndexOf("/") + 1);
            contentName  = identifier;
        }
        else
        {
            appType = AppInfo::System;
            identifier = exePath.mid( exePath.lastIndexOf("/") + 1 );
            contentName = identifier;
        }
    }
    else
    {
        appType = AppInfo::Other;
        identifier = exePath.mid( exePath.lastIndexOf("/") + 1 );
        userVisibleName = identifier;
        return *this;
    }

    QContentId contentId;
    contentId = QContent::execToContent( contentName );
    if ( contentId == QContent::InvalidId )
        userVisibleName = identifier;
    else
        userVisibleName = QContent(contentId).name();

    return *this;
}

QString SxeMonitor::AppInfo::toString() const
{
    return QLatin1String("AppType: " ) +  appType + QLatin1String("\t") +
           QLatin1String("Identifier: " ) + identifier + QLatin1String( "\t" ) +
           QLatin1String("User visible name: ") + userVisibleName + QLatin1String("\t") +
           QLatin1String("Executable: " ) + executable + QLatin1String("\t") +
           QLatin1String("Pid: " ) +  QString::number( pid );
}


void SxeMonitor::AppInfo::reset()
{
    appType = AppInfo::Incomplete;
    pid = -1;
    identifier = QString::null;
    userVisibleName = QString::null;
    executable = QString::null;
}
