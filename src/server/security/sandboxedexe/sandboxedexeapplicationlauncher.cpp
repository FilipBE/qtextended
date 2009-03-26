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

#include "sandboxedexeapplicationlauncher.h"
#include "qtopiaserverapplication.h"

#include <QTimer>
#include <QtopiaServiceRequest>

#include <sys/resource.h>

// define SandboxedExeApplicationLauncherPrivate
class SandboxedExeApplicationLauncherPrivate
{
public:
    SandboxedExeApplicationLauncherPrivate();
    QString rlimiterExecutable();    
    QHash<int, unsigned long> resourceLimits;

private:
    QString m_rlimiterExecutable;
};

SandboxedExeApplicationLauncherPrivate::SandboxedExeApplicationLauncherPrivate()
    :m_rlimiterExecutable()
{
}

/*! \internal
    Returns the absolute path to the rlimiter executable 
*/
QString SandboxedExeApplicationLauncherPrivate::rlimiterExecutable()
{
    if(m_rlimiterExecutable.isEmpty()) {
        QStringList rv;
        QStringList paths = Qtopia::installPaths();
        for(int ii = 0; m_rlimiterExecutable.isEmpty() && ii < paths.count(); ++ii)
            if(QFile::exists(paths.at(ii) + "bin/rlimiter"))
                m_rlimiterExecutable = paths.at(ii) + "bin/rlimiter";
    }
    return m_rlimiterExecutable;
}

/*!
  \class SandboxedExeApplicationLauncher
    \inpublicgroup QtPkgManagementModule
  \ingroup QtopiaServer::AppLaunch
  \brief The SandboxedExeApplicationLauncher class supports launching untrusted
         downloaded application executables.

  \bold {Note:} This class is only relevant if SXE is enabled
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  The SanboxedExeApplicationLauncher class provides the ApplicationTypeLauncher
  implementation for simple but untrusted executable based applications (which
  have been downloaded and installed via packagemanager).  The executable is run under
  sandboxed conditions to minimize the potential damage the application
  may cause.

  The SandboxedExeApplicationLauncher class provides the
  SanboxedExeApplicationLauncher Task.
*/

QTOPIA_TASK(SandboxedExeApplicationLauncher, SandboxedExeApplicationLauncher);
QTOPIA_TASK_PROVIDES(SandboxedExeApplicationLauncher, ApplicationTypeLauncher);

// define SandboxedExeApplicationLauncher
/*!
  Constructs a new SandboxedExeApplicationLauncher instance.
 */
SandboxedExeApplicationLauncher::SandboxedExeApplicationLauncher()
: d( new SandboxedExeApplicationLauncherPrivate() )
{
    QTimer::singleShot( 0, this, SLOT(init()) );
}

/*!
  \fn QString SandboxedExeApplicationLauncher::name()

  \reimp
  */

/*!
  Destroys the SandboxedExeApplicationLauncher instance.
 */
SandboxedExeApplicationLauncher::~SandboxedExeApplicationLauncher()
{
    delete d;
    d = 0;
}

/*!
  \internal
  Obtains the resource limits to be used, currently the only limit
  is RLIMIT_AS. As a very rough measure, this memeory limit
  is a proportion of the available physical memory on the device.
*/
void SandboxedExeApplicationLauncher::init()
{
    QLatin1String sxeConfName( "Sxe" );
    QLatin1String limitsGroup( "Limits" );
    QLatin1String maxMemRatio( "MaxMemRatio" );
    QSettings conf( QSettings::SystemScope,"Trolltech", sxeConfName );
    conf.beginGroup( limitsGroup );
    bool ok = false;
    
    if ( conf.contains( maxMemRatio ) ) 
    {
        double memRatio = conf.value( maxMemRatio ).toDouble(&ok);
        if ( !ok )
        {
            qFatal( "SandboxedExeApplication::init(): Could not read value of key, %s/%s from "
                  "%s.conf.", limitsGroup.latin1(), maxMemRatio.latin1(), sxeConfName.latin1() ) ;
        }
        else if ( memRatio <= 0.0 )
        {
            qFatal( "SandboxedExeApplication::init():  Invalid config value from %s.conf. " 
                    "The value of %s/%s must be > 0", sxeConfName.latin1(),
                    limitsGroup.latin1(), maxMemRatio.latin1() ); 
        }
        
        qLog( SXE ) << "SandboxedExeApplicationLauncher::init() " << sxeConfName + QLatin1String(".conf") 
                    << limitsGroup << "/" << maxMemRatio << "=" << memRatio;
    
        QFile procFile("/proc/meminfo");
        unsigned int memTotal=0; //unit is kb 
        if ( !procFile.open( QIODevice::ReadOnly ) )
        {
            qFatal("SandboxedExeApplicationLauncher::init(): Could not open %s to get "
                 "total memory available on device. ", qPrintable(procFile.fileName()) );
        } 
        else
        {
            QByteArray line;
            bool memTotalFound = false;
            while( !((line = procFile.readLine()).isEmpty()) )
            {
                if ( line.startsWith("MemTotal:") )
                {
                    memTotalFound = true;
                    line = line.simplified();
                    line = line.split(' ').at(1);  //expected line format is MemTotal:   12345 kb
                    memTotal = line.toULong( &ok );
                    qLog( SXE ) << "SandboxedExeApplicationLauncher::init() /proc/meminfo MemTotal =" 
                                << memTotal << "kb";
                    if ( !ok )
                    {      
                        qFatal("SandboxedExeApplicationLauncher::init(): Could not obtain "
                            "value for total memory after reading %s", qPrintable(procFile.fileName())) ; 
                    }
                    break;
                }
            }
            if ( !memTotalFound )
            {
                qFatal("SandboxedExeApplicationlauncher::init(): Could not find MemTotal field in " 
                         "%s.", qPrintable(procFile.fileName()));  
            }
        }
        d->resourceLimits[ RLIMIT_AS ] = static_cast<unsigned long>(memRatio * memTotal * 1024);
        qLog( SXE ) << "SandboxedExeApplication::init() RLIMIT_AS=" 
                    << d->resourceLimits[ RLIMIT_AS ];
    }
}

/*! \internal */
bool SandboxedExeApplicationLauncher::canLaunch(const QString &app)
{
    // Check whether the executable exists
    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if( !QFile::exists( exes.at(ii) ) && QFile::symLinkTarget(exes.at(ii)).endsWith("__DISABLED") ) {
            QtopiaServiceRequest req( "SystemMessages", "showDialog(QString,QString)" );
            req << tr("Security Alert");
            QString msg = tr("<qt>This application has been <font color=\"#FF0000\">disabled</font></qt>");  
            req << msg;
            req.send();
            return false;
        }
        if(QFile::exists(exes.at(ii)))
            return true;
    }
    return false;
}


/*! \internal */
void SandboxedExeApplicationLauncher::launch(const QString &app)
{
    if(isRunning(app))
        return; // We're already launching/have launched this guy

    Q_ASSERT(canLaunch(app));

    // We need to launch it
    QProcess *proc = new QProcess(this);
    proc->setReadChannelMode(QProcess::ForwardedChannels);
    proc->closeWriteChannel();

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {
            QStringList args;
            args.append( exes.at(ii) );
            args.append( QString::number( d->resourceLimits.count() ) );
            QHash<int, unsigned long>::const_iterator iter = d->resourceLimits.constBegin();
            while (iter != d->resourceLimits.constEnd()) {
               args.append( QString::number(iter.key()) );
               args.append( QString::number(iter.value()) );
               ++iter;
            }
            args.append( "-noshow" );
            setupPackageLaunch(exes.at(ii), proc);
            proc->start( d->rlimiterExecutable(), args);
            addStartingApplication(app, proc);
            return; // Found and done
        }
    }
}

/*! \internal
  If the application requested is an absolute path containing Qtopia::packagePath(),
  it is run with restrictions;  otherwise Qtopia::packagePath()/bin is searched.
*/
QStringList SandboxedExeApplicationLauncher::applicationExecutable(const QString &app)
{
    if ( app.startsWith( "/" )  && app.contains(Qtopia::packagePath()) )
        return QStringList() << app;

    QStringList rv(Qtopia::packagePath() + "bin/" + app);

    return rv;
}
#
