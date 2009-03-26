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

#include "qabstractserverinterface.h"
#include <qtopiaapplication.h>
#include <qtimezone.h>
#include <custom.h>

#include <qfile.h>
#include <qdir.h>
#include <qimagereader.h>
#include <qsettings.h>
#ifdef Q_WS_QWS
#include <qscreen_qws.h>
#include <qwindowsystem_qws.h>
#endif
#include "qtopiainputevents.h"
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <QPainter>
#include <QThread>
#include <QBasicTimer>

#include <QImageIOHandler>

QSXE_APP_KEY

#include "qtopiaserverapplication.h"
#include <QDesktopWidget>
#include <QLibraryInfo>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <QValueSpaceItem>
#include <QKeyEvent>
#include <ThemedView>
#include "uifactory.h"

// Not currently supported, available for demonstration purposes.
//#define QTOPIA_ANIMATED_SPLASH

#if defined(QTOPIA_ANIMATED_SPLASH)
#include <QLabel>
#endif

static void initKeyboard()
{
    QSettings config("Trolltech","qpe");

    config.beginGroup( "Keyboard" );

/* FIXME
    int ard = config.value( "RepeatDelay" ).toInt();
    int arp = config.value( "RepeatPeriod" ).toInt();
    if ( ard > 0 && arp > 0 )
        qwsSetKeyboardAutoRepeat( ard, arp );
*/

    QString layout = config.value( "Layout", "us101" ).toString();
}


static bool firstUse()
{
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Startup" );

#ifdef QPE_NEED_CALIBRATION
    if (UIFactory::isAvailable("Calibrate")) {
        //if we don't want calibration we shouldn't even test pointercal
        if (config.value("NeedCalibrate", true).toBool()) {
            bool cal=false;
            QString calFile = QString::fromLocal8Bit(qgetenv("POINTERCAL_FILE"));
            if (calFile.isEmpty())
                calFile = QLatin1String("/etc/pointercal");

            QFile file(calFile);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream t(&file);
                int a,b,c,d,e,f,s = 0;
                t >> a >> b >> c >> d >> e >> f >> s;
                qWarning() << s << t.status();
                if (s == 0 || t.status() != QTextStream::Ok)
                    cal=true;
                file.close();
            } else {
                cal=true;
            }

            if(cal) {
                QDialog *calibrateDlg = UIFactory::createDialog("Calibrate");
                if (calibrateDlg) {
                    calibrateDlg->exec();
                    delete calibrateDlg;
                } else {
                    qLog(Component) << "Main: Calibrate not available";
                }
            }
        }
    }
#endif

    if (!config.value("FirstUse", true).toBool())
        return false;

    QDialog *firstUseDlg = UIFactory::createDialog("FirstUse");
    if (firstUseDlg) {
        firstUseDlg->exec();
        delete firstUseDlg;
        return true;
    } else {
        qLog(Component) << "Main: FirstUse component not available";
        return false;
    }

}

#if defined(QTOPIA_ANIMATED_SPLASH)
class DirectMovie : public QObject, public QImageReader {
public:
    DirectMovie() {}

    void play(const QString& filename)
    {
        setFileName(filename);

        if ( !canRead() )
            return;

        gettimeofday(&last,0);
        timer.start(0,this);
    }

    void timerEvent(QTimerEvent*)
    {
        QImage img = read();
        int delay = 0;

        if ( img.isNull() ) {
            setFileName(fileName()); // XXX restart
        } else {
            QRegion r(0,0,qt_screen->width(),qt_screen->height());
            qt_screen->blit(img,QPoint(0,0),r);
            qt_screen->exposeRegion(r,0);
            if ( imageCount() == 1 )
                return;
            struct timeval now;
            gettimeofday(&now,0);
            int period = nextImageDelay();
            delay = period -
                ((now.tv_sec-last.tv_sec)*1000+(now.tv_usec-last.tv_usec)/1000);
            last.tv_sec += period/1000;
            last.tv_usec += (period%1000)*1000;
            if ( delay < 0 )
                delay = 0;
        }

        timer.start(delay,this);
    }

private:
    struct timeval last;
    QBasicTimer timer;
};


class DirectMovieThread : public QThread {
public:
    DirectMovieThread(QObject *parent) : QThread(parent)
    {
    }

    void play(const QString& file)
    {
        filename = file;
        start();
    }

    void run()
    {
        movie = new DirectMovie;
        movie->play(filename);
        exec();
        delete movie;
    }

private:
    QString filename;
    DirectMovie *movie;
};


class SplashScreen : public QLabel {
public:
    SplashScreen() :
        QLabel(0),
        rep(0),
        thread(0)
    {
    }

    ~SplashScreen()
    {
        if ( thread ) {
            thread->terminate();
            thread->wait();
            delete thread;
        }
    }

    void splash(const QString& animfile)
    {
        setAttribute(Qt::WA_NoBackground);
        timer.start(2000,this); // minimum splash time

        // Conceptually, we want to paint "on this widget", but instead
        // we paint on the screen, since that can be done in a thread.
        //showFullScreen();

        thread = new DirectMovieThread(this);
        thread->play(animfile);
    }

    void setReplacement(QWidget *s)
    {
        rep = s;
    }

    void timerEvent(QTimerEvent*)
    {
        if ( rep ) {
            // showMaximized() doesn't work for frameless windows.
            QDesktopWidget *desktop = QApplication::desktop();
            QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
            rep->setGeometry(desktopRect);
            rep->show();
            hide();
            rep->lower();
            delete this;
        } else {
            timer.start(200,this); // keep waiting
        }
    }
private:
    QWidget* rep;
    QImage img;
    DirectMovieThread *thread;
    QBasicTimer timer;
};
#endif

class BootCharger : public ThemedView, public QtopiaKeyboardFilter
{
    Q_OBJECT
public:
    BootCharger() :
        ThemedView(),
        vsoCharging("/Hardware/Accessories/QPowerSource/DefaultBattery/Charging")
    {
        QSettings qpeCfg("Trolltech", "qpe");
        qpeCfg.beginGroup("Appearance");
        QString themeDir = Qtopia::qtopiaDir() + "etc/themes/";
        QString theme = qpeCfg.value("Theme").toString();

        QSettings themeCfg(themeDir + theme, QSettings::IniFormat);
        themeCfg.beginGroup("Theme");
        if (themeCfg.contains("BootChargerConfig")) {
            QString themeFile = themeCfg.value("BootChargerConfig").toString();

            setSourceFile(themeDir + themeFile);
            if (loadSource())
                layout();
            else
                qWarning("Invalid BootCharger theme.");
        } else {
            qWarning("Invalid BootCharger theme.");
        }

        connect(&vsoCharging, SIGNAL(contentsChanged()), SLOT(chargingStateChanged()));

        QtopiaInputEvents::addKeyboardFilter(this);
    }

    virtual bool filter(int, int keycode, int modifiers, bool press, bool autoRepeat)
    {
        //any key press will cause a start
        switch ( keycode ) {
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            case Qt::Key_Hangup:
            case Qt::Key_Context1:
            case Qt::Key_Back:
            case Qt::Key_Select:
            case Qt::Key_Asterisk:
            case Qt::Key_NumberSign:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
                if (!autoRepeat && !modifiers && press)
                    doActivate();
                break;
            default:
                break;
        }

        return true;
    }

signals:
    void finished();

private slots:
    void doActivate()
    {
        ThemeItem *item = findItem("battery", ThemedView::Item);
        if (item)
            item->setActive(false);

        item = findItem("loading", ThemedView::Item);
        if (item)
            item->setActive(true);

        repaint();
        emit finished();
    }

    void chargingStateChanged()
    {
        if (!vsoCharging.value(QByteArray(), false).toBool()) {
            ThemeItem *item = findItem("battery", ThemedView::Item);
            if (item)
                item->setActive(false);

            item = findItem("poweroff", ThemedView::Item);
            if (item)
                item->setActive(true);

            repaint();

            QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
        }
    }

private:
    QValueSpaceItem vsoCharging;
};


//
// IMPORTANT NOTE: This function implements part of the Qtopia startup
// processing.  The sequence of steps performed during Qtopia startup is
// documented in the file <depot-directory>/doc/src/qtopia-startup.qdoc.
// If you make changes or additions to the startup sequence, you must ensure
// that that document is also updated.
//
int initApplication( int argc, char ** argv )
{
    qLog(QtopiaServer) << "begin initApplication";

    if(!QtopiaServerApplication::startup(argc, argv, QList<QByteArray>() << "prestartup"))
        qFatal("Unable to initialize task subsystem.  Please check '%s' exists and its content is valid.", QtopiaServerApplication::taskConfigFile().toLatin1().constData());

    initKeyboard();

    firstUse();

    qLog(QtopiaServer) << "Keyboard initialisation";

    const QByteArray warmBootFile = "/tmp/warm-boot";
    if ( !QFile::exists( warmBootFile ) &&
         QValueSpaceItem("/Hardware/Accessories/QPowerSource/DefaultBattery/Charging", false).value().toBool() ) {

        QBootSourceAccessory bsa;
        if (bsa.bootSource() == QBootSourceAccessory::Charger) {

            BootCharger *bootCharger = new BootCharger;
            bootCharger->showFullScreen();

            QEventLoop eventloop;
            eventloop.connect(bootCharger, SIGNAL(finished()), SLOT(quit()));
            eventloop.exec();

            //remove bootcharger from keyboard filter list
            //removeKeyboardFilter() deletes the pointer to a keyboard filter
            //this happens to be the above bootcharger object (see qwindowsystem_qws.cpp)
            QtopiaInputEvents::removeKeyboardFilter();
        }
    }
    QFile warmBootMarker(warmBootFile);
    warmBootMarker.open( QIODevice::Append | QIODevice::Truncate );
    warmBootMarker.close();

#if defined(QTOPIA_ANIMATED_SPLASH)
    SplashScreen *splash = new SplashScreen;
    splash->splash(QString(":image/splash"));
    qLog(QtopiaServer) << "Splash screen creation";
#endif

    if(!QtopiaServerApplication::startup(argc, argv, QList<QByteArray>() << "startup"))
        qFatal("Unable to initialize task subsystem.  Please check '%s' exists and its content is valid.", QtopiaServerApplication::taskConfigFile().toLatin1().constData());


    // Load and show UI
    QAbstractServerInterface *interface =
        qtopiaWidget<QAbstractServerInterface>(0, Qt::FramelessWindowHint);

#if defined(QTOPIA_ANIMATED_SPLASH)
    splash->setReplacement(interface);
#else
    if(interface) {
        interface->show();
    } else {
        qWarning() << "Missing main widget of type QAbstractServerInterface";
    }
#endif

    if(!QtopiaServerApplication::startup(argc, argv, QList<QByteArray>() << "idle", QtopiaServerApplication::IdleStartup))
        qFatal("Unable to initialize task subsystem.  Please check '%s' exists and its content is valid.", QtopiaServerApplication::taskConfigFile().toLatin1().constData());

    qLog(QtopiaServer) << "begin event loop";

    int rv = static_cast<QtopiaApplication *>(qApp)->exec();

    qLog(QtopiaServer) << "end event loop";

    delete interface;

    qLog(QtopiaServer) << "end initApplication";
    return rv;
}

static void check_prefix()
{
    // Construct a string with the path to the binary that we'll expect to see at runtime
    QString prefix_bin = QDir(QString("%1/bin/%2").arg(QLibraryInfo::location(QLibraryInfo::PrefixPath)).arg(QTOPIA_TARGET)).absolutePath();
    QString prefix_check = QDir(prefix_bin).canonicalPath(); // handle symlinks
    // Get the path to the currently running binary
    QString proc_check = QDir(QString("/proc/%1/exe").arg(getpid())).canonicalPath();
    if ( proc_check.isEmpty() )
        proc_check = QString("CANNOT READ /proc/%1/exe").arg(getpid());
    if ( prefix_check != proc_check ) {
        // Figure out what the prefix should be based on the location of the currently running binary
        QString proc_prefix = QDir(QString("%1/../..").arg(proc_check)).canonicalPath();
        qWarning() << "**********************************************************" << endl
                   << "* ERROR: Expecting this binary to be located in" << endl
                   << "*  " << prefix_bin.toLocal8Bit().constData() << endl
                   << "* but it is being run from" << endl
                   << "*  " << proc_check.toLocal8Bit().constData() << endl
                   << "*" << endl
                   << "* This generally indicates that you have specified the wrong" << endl
                   << "* value for -prefix when configuring Qtopia. Based on the" << endl
                   << "* location of this binary, you should be using a prefix of" << endl
                   << "*  " << proc_prefix.toLocal8Bit().constData() << endl
                   << "**********************************************************" << endl;
    }
}

#ifdef SINGLE_EXEC
#include "../tools/quicklauncher/quicklaunch.h"
int main_quicklaunch( int argc, char **argv );

int main( int argc, char ** argv )
{
    check_prefix();
    QString executableName(argv[0]);
    executableName = executableName.right(executableName.length() - executableName.lastIndexOf('/') - 1);

    if ( executableName != "qpe" ) {
        qLog(ApplicationLauncher) << "Starting single-exec main";
        QPEMainMap *qpeMainMap();
        if ( qpeMainMap()->contains(executableName) ) {
            // This is a non-quicklaunch app
            return (*qpeMainMap())[executableName](argc, argv);
        } else {
            // Someone directly invoked a quicklaunch app
            // Pass control to quicklaunch's main function so that it can deal with it
            return main_quicklaunch(argc, argv);
        }
    }

#else // SINGLE_EXEC
int main( int argc, char ** argv )
{
    check_prefix();

#endif // SINGLE_EXEC
    qLog(QtopiaServer) << "begin qpe main";

    setpgrp();  // New process group

    signal( SIGCHLD, SIG_IGN );

    QByteArray restartFile = qgetenv("QTOPIA_RESTART_FILE");
    if ( restartFile.isEmpty() )
        restartFile = "/tmp/restart-qtopia";
    QFile restartMarker(restartFile);
    restartMarker.open( QIODevice::Append | QIODevice::Truncate );
    restartMarker.close();

    int retVal = initApplication( argc, argv );
    // Have we been asked to restart?
    if(!(QtopiaServerApplication::shutdownType() == QtopiaServerApplication::RestartDesktop)) {
        //we assume that the calling script is running
        //in a loop which will restart qpe automatically
        //if /tmp/restart-qtopia exists. The script may or
        //may not delete the file
        if ( QFile::exists( restartFile ) )
            QFile::remove( restartFile );
    }

    // Kill them. Kill them all.
    // Place ourselves in parent process group and kill all those in the
    // created process group
    pid_t parentGroup = getpgid(getppid());
    pid_t qpeGroup = getpgid(0);

    if (setpgid(0, parentGroup) != 0)
        qWarning("Failed to set process group %s", strerror(errno));
    else
    {
        // Ask first
        if (killpg(qpeGroup, SIGTERM ) != 0)
            qWarning("Unable to send SIGTERM %s", strerror(errno));

        Qtopia::sleep(1);

        // Die
        if (killpg(qpeGroup, SIGKILL ) != 0)
            qWarning("Unable to send SIGKILL %s", strerror(errno));

        // Force a reaping; something messing with sigchld => sig_ign?
        while (::wait(0) != -1 || errno == EINTR)
            ;
    }

    qLog(QtopiaServer) << "end qpe main";
    delete QtopiaServerApplication::instance();
    return retVal;
}

#include "main.moc"

