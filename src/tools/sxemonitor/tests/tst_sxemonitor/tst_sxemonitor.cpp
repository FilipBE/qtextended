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

#define TEST_SXEMONITOR //used in sxemonitor to declare friend class

#define private public
#include "../../../../../src/tools/sxemonitor/sxemonqlog.h"
#include "../../../../../src/tools/sxemonitor/sxemonitor.h"
#include <qtopianamespace.h>
#include <qfile.h>
#include <stdio.h>
#include <unistd.h>
#include <qapplication.h>
#include <iostream.h>
#include <qeventloop.h>
#include <sys/syslog.h>
#include <qtimer.h>
#include <qthread.h>
#include <qdebug.h>
#include <qprocess.h>
#include <quuid.h>
#include <qtopiaapplication.h>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QTemporaryFile>
#include <QHostInfo>

#undef private

//TESTED_CLASS=SxeMonitor
//TESTED_FILES=src/tools/sxemonitor/sxemonitor.cpp

QByteArray lastLogMessage;
QString syslog_log_path;

void syslog (int /*pri*/, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *log_message;
    vasprintf(&log_message, fmt, args);
    lastLogMessage = log_message;
    lastLogMessage.prepend((QDateTime::currentDateTime().toString("MMM dd hh:mm:ss") + " " + QHostInfo::localHostName() + " tst_sxemonitor: ").toLatin1());
    lastLogMessage.append("\n");
    free(log_message);
    QFile log(syslog_log_path);
    log.open( QIODevice::WriteOnly );
    log.write(lastLogMessage);
}


//NEED TO TEST INVALID CONDITIONS eg invalid security log format etc
  
/*
  The LoadThread class is designed to be a thread which continually
  updates the security log file as specified in tst_SxeMonitor::logPath;
  doing so allows one to evaluate how well SxeMonitor copes under
  a heavy load.
*/
class LoadThread: public QThread
{
    Q_OBJECT
public:
   void run();
};

/*
  The tst_SxeMonitor class performs a number of unit tests
  on SxeMonitor to ensure that logging and log rotations
  occur correctly.  
*/
class tst_SxeMonitor : public QObject 
{
    Q_OBJECT

public:
    static QStringList outputs; //stores output messages such as qLogs, qDebugs etc
    static bool stopRecord; //indicates whether to stop recording output messages 

    static int numIterations;//used specifically for handleRotation
    static const QString sxeTag; //tag that identifies log entry as sxe related
    static const QString testRequest; //dummy test request
    static const int basePID; //base dummy pid
    
    tst_SxeMonitor();

private slots:
    void initTestCase();

#ifdef QT_NO_QWS_VFB
    void forceRotation();
    void handleRotation();
    void handleLoad();
    void selfAudit(); 
#endif
    void lockdown();

private:
    void waitForCondition( int ms );
    bool verifyOutputs( const QStringList &expectedOutputs, int waitTime, QString &error);

#ifdef QT_NO_QWS_VFB
    void fillLog( long size );
    void calcLogEntrySizes(); 
#endif

    QTemporaryFile logFile;
    QString logPath;
    int minLogEntrySize; //size of empty log entry
    int sxeLogEntrySize; //size of dummy sxe log entry
    LoadThread *loadThread;
    QtMsgHandler originalMsgHandler;
    
    bool oldLogsExist; //indicates whether to generate/keep pre-existing old logs eg messages.0.old
    bool rotatedLogsExist; //indicates whether to generate/keep pre-existing rotated logs eg messsages.0, etc
};

/*?
  \internal
  listens to and accumulates outputs of sxemonitor for initTestCase() 
  into \c tst_SxeMonitor::outputs.
*/
void initTestCaseListener( QtMsgType type, const char *msg )
{
    QString message( msg );
    
    tst_SxeMonitor::outputs << message; 
    //if last expected output is processed
    if ( message.contains( SxeMonQLog::getQLogString(SxeMonQLog::SuccessInit) ) )
        tst_SxeMonitor::stopRecord = true;
}

/*?
  \internal
  listens to and records outputs of sxemonitor during forceRotation()
  into \c tst_SxeMonitor::outputs.
*/
void forceRotationListener( QtMsgType type, const char *msg )
{
    QString message( msg );
    static int count = 0;

    tst_SxeMonitor::outputs << message;
    if ( message.contains( SxeMonQLog::getQLogString(SxeMonQLog::Processing) ) )
    {
        count++;
        
        //2nd time is last expected output
        if ( count == 2 )
            tst_SxeMonitor::stopRecord = true;
    }
}

/*?
  \internal
  listens to and records log outputs of sxemonitor
  into \c tst_SxeMonitor::outputs.
*/
void breachListener( QtMsgType type, const char *msg )
{
    QString message( msg );

     //ignore the processing message
    if ( message.contains( SxeMonQLog::getQLogString(SxeMonQLog::Processing) ) )
        return;

    tst_SxeMonitor::outputs << message;
   
    static QString lastExpectedStr( SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail, 
                                    QStringList() << QString::number(tst_SxeMonitor::numIterations - 1) 
                                                  << tst_SxeMonitor::testRequest ) );
    if ( message.contains(lastExpectedStr) )
        tst_SxeMonitor::stopRecord = true;
}

/*?
  \internal
  listens to and records outputs of sxemonitor for verifyLockdown()
  into \c tst_SxeMonitor::outputs.
*/
void lockdownListener( QtMsgType type, const char *msg )
{
    QString message( msg );
    
    //ignore the KillAllBinary message once
    static int count = 0; 
    if ( message.contains( SxeMonQLog::getQLogString(SxeMonQLog::KillApplication, QStringList("")) ) )
    {    
        if ( count > 0 ) 
             return;
        else
            count++;
    }
    tst_SxeMonitor::outputs << message;
    if ( message.contains( SxeMonQLog::getQLogString(SxeMonQLog::DelayMsgDispatched) ) )
            tst_SxeMonitor::stopRecord = true;
}

/*?
  \internal
  ignores the log outputs of sxemonitor
*/
void ignore( QtMsgType type, const char *msg )
{
}

QStringList tst_SxeMonitor::outputs;
bool tst_SxeMonitor::stopRecord= false;
int tst_SxeMonitor::numIterations = 10;

const QString tst_SxeMonitor::testRequest = "Test_Request";
const int tst_SxeMonitor::basePID = 100000;
const QString tst_SxeMonitor::sxeTag = SxeLogEntry::sxeTag;

/*?
  \internal
  Constructor
*/
tst_SxeMonitor::tst_SxeMonitor(): logFile("/tmp/tst_sxemonitor"),
                                  oldLogsExist( true ), rotatedLogsExist( true ) 
{
    logFile.open();
    logPath = logFile.fileName();
    syslog_log_path = logPath;
    qLog(Autotest) << "Using SXE log:" << logPath;
}

/*?
  \internal
  Ensures that an instance of sxemonitor has been correctly started.
  Also sets up or deletes pre-existing old and/or rotated logs as required.  
  By default old logs and rotated logs are generated if they do
  not already exist.  The initTestCase() also determines the
  minLogEntry and sxeLogEntry sizes.

  Terminology: Old log files refer to those saved with a 
  .old extension.  Rotated logs refer to those with a numerical
  extension eg messages.0, messages.1 etc.
*/
void tst_SxeMonitor::initTestCase()
{
    QValueSpace::initValuespaceManager();

    QSettings sxeConf("Trolltech", "Sxe");
    sxeConf.beginGroup("Log");
    sxeConf.setValue("Path", logPath);
    sxeConf.setValue("StampFormat", "^([A-Z][a-z]{2}\\s+\\d{1,2}\\s+\\d{2}:\\d{2}:\\d{2}[^:]*tst_sxemonitor:\\s)(.*)$");
    QSettings("Trolltech", "Log").setValue("SxeMonitor/Enabled", true);

    QStringList expectedOutputs;
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::LogPathSet, QStringList(logPath) );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::SuccessInit );

    originalMsgHandler = qInstallMsgHandler( initTestCaseListener );
    SxeMonitor *monitor = new SxeMonitor;
    monitor->setParent(this);
    
    QString error; 
    if ( !verifyOutputs( expectedOutputs, 10000, error ) )
        QFAIL( qPrintable(error) );

#ifdef QT_NO_QWS_VFB
    calcLogEntrySizes();
#endif

}


#ifdef QT_NO_QWS_VFB

/*?
  \internal

  Populates the security log file such that the sxemonitor must force
  a rotation.  Checks are made to ensure that the roation has occurred.   
*/
void tst_SxeMonitor::forceRotation()
{
    QStringList expectedOutputs;
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::Processing );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::ForceRot );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::Processing);

    qInstallMsgHandler( ignore );
    fillLog( SxeMonitor::maxLogSize );
    
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    
    QCOMPARE( logFile.size(), SxeMonitor::maxLogSize );

    qInstallMsgHandler( forceRotationListener );
    syslog ( LOG_ERR | LOG_LOCAL6, "z" ); // make log file greater than SxeMonitor::maxLogSize
  
    Qtopia::sleep( 2 );
    QCoreApplication::processEvents(QEventLoop::AllEvents);
  
    QVERIFY( logFile.size() < SxeMonitor::maxLogSize );  //verify a rotation has occurred

    QString error; 
    if ( !verifyOutputs( expectedOutputs, 10000, error ) )
        QFAIL( qPrintable(error) );
}

/*?
  \internal

  Populates the security log file so that a rotation automatically occurs
  and checks whether the sxemonitor has processed the security log entries 
  at the end of the old and beginning of the new log file.
*/
void tst_SxeMonitor::handleRotation()
{
    numIterations = 10;

    QStringList expectedOutputs;
    for ( int i = 0; i < numIterations; i++ )
    {
        expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail, 
                                    QStringList() << QString::number(tst_SxeMonitor::basePID + i) 
                                                  << testRequest );
    }

    //fill the log file until it is (numIterations/2) entries from being full
    QtMsgHandler oldMsgHandler =  qInstallMsgHandler( ignore );
    int size = SxeMonitor::maxLogSize - (numIterations / 2 * sxeLogEntrySize) + ( 0.5 * sxeLogEntrySize );
    fillLog( size );
  
    QCoreApplication::processEvents(QEventLoop::AllEvents);
        
    //make numIteration log entries
    qInstallMsgHandler( breachListener );
    for ( int i = 0; i < numIterations; i++)
        syslog( LOG_ERR | LOG_LOCAL6, "%s %u %s", qPrintable(tst_SxeMonitor::sxeTag), 
                (tst_SxeMonitor::basePID + i), qPrintable(testRequest) );

    //confirm that all expected outputs are caught and that a rotation did occur    
    QString error;
    if ( !verifyOutputs( expectedOutputs, 10000, error ))
        QFAIL( qPrintable(error) ); 
    
    QVERIFY( logFile.size() < size );
}

/*?
  \internal

  Continually updates the security log and checks whether all the entries
  have been processed by sxemonitor.
*/
void tst_SxeMonitor::handleLoad()
{
    numIterations = 3 * SxeMonitor::maxLogSize / sxeLogEntrySize;  //number of entries to make 
    
    QStringList expectedOutputs;
    for ( int i = 0; i < numIterations; i++ )
    {
        expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail, 
                                    QStringList() << QString::number(tst_SxeMonitor::basePID + i) 
                                                  << testRequest );
    }

    qInstallMsgHandler( breachListener );

    //let the load thread update the security log file
    loadThread = new LoadThread();
    loadThread->start();

    QString error; 
    if ( !verifyOutputs( expectedOutputs, 30000, error ) )
        QFAIL( qPrintable(error) );        
}

/*?
  \internal

  Test to make sure that verifyOutputs() will detect an error
  if all expected outputs are not present in tst_SxeMonitor::outputs   
*/
void tst_SxeMonitor::selfAudit()
{
    numIterations = 20;

    //numIteration+1 entries will be expected 
    QStringList expectedOutputs;
    for ( int i = 0; i <= numIterations; i++ )
    {
        expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::BreachDetail, 
                                    QStringList() << QString::number(tst_SxeMonitor::basePID + i) 
                                                  << testRequest );
    }
    
    if ( loadThread->isRunning() )
        loadThread->terminate();    
    
    //numIteration entries will be generated
    qInstallMsgHandler( breachListener );
    loadThread->start();

    QString error;
    if ( verifyOutputs( expectedOutputs, 10000, error ) )
        QFAIL( "verifyOutputs was expected to fail but passed" );

    if( !error.contains(expectedOutputs.last()) )
        QFAIL( qPrintable( QString( "verifyOutputs produced the following error:" ) 
               + error + "\n the expected error should have contained "
               + expectedOutputs.last())); 
}
#endif //QT_NO_QWS_VFB

/*?
  \internal
  
  Initiates a lockdown and ensures that it is appropriately 
  responded to by the SxeMonitor.  
*/
void tst_SxeMonitor::lockdown()
{
    QStringList expectedOutputs;
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::Processing );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::Lockdown );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::KillApplication, QStringList("") << "");
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::MsgDispatched );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::DelayMsgPrep );
    expectedOutputs << SxeMonQLog::getQLogString( SxeMonQLog::DelayMsgDispatched );
 
    qInstallMsgHandler( lockdownListener );
    syslog( LOG_ERR | LOG_LOCAL6, "%s %s", 
        qPrintable(tst_SxeMonitor::sxeTag), qPrintable(SxeLogEntry::farExceeded) );
    
    QString error;
    if ( !verifyOutputs( expectedOutputs, 10000, error ) )
        QFAIL( qPrintable( error ) );
}

/*?
  \internal

  waits for a specified interval or until the condition,
  tst_SxeMonitor::stopRecord == true, is met
*/
void tst_SxeMonitor::waitForCondition (int ms)
{
    Q_ASSERT(QCoreApplication::instance());

    QTime timer;
    timer.start();
    QEventLoop loop;
    do {
        QTimer::singleShot(ms, &loop, SLOT(quit()));
        loop.exec();
        if ( tst_SxeMonitor::stopRecord )
           return; 
        
        QTest::qSleep(10);
    } while (timer.elapsed() < ms);
}

/*?
  \internal

  Verifies that all the elements in \a expectedOutputs have been outputted in
  order and within the time frame (in millisecond) specified by \a waitTime.  
  Precondition:  The appropriate output message handler/listener should 
                 be setup prior to calling this method.  
  Postcondition: The parameter  \a error is modified to describe any 
                 errors, that occurred.  The messagehandler is restored 
                 to the original messagehandler.  \c tst_SxeMonitor::outputs
                 is cleared
  
  Implementation details:
  The outputs are accumulated by the listener methods into \c tst_SxeMonitor::outputs.
  /c verifyOutputs() will wait until the last expected output is processed by the 
  listener method or until \a waitTime milliseconds have passed.  At this point 
  all of the elements in \expectedOutputs are checked against \c tst_sxeMonitor::outputs.
  Warnings will be generated for outputs in \c tst_sxeMonitor::outputs not found in 
  \a expectedOutputs.   
*/
bool tst_SxeMonitor::verifyOutputs( const QStringList &expectedOutputs, int waitTime, QString &error ) 
{
    //wait for all outputs to be accumulated by listener methods 
    if ( stopRecord == false )
        waitForCondition( waitTime );          
    stopRecord = false;
    qInstallMsgHandler( originalMsgHandler );

    int prevPos = 0;
    int currPos = 0;
    QRegExp regExp;
    QString emptyTag="_EMPTY_"; 
    
    // look for each expectedOutput in tst_SxeMonitor::outputs 
    foreach( QString str, expectedOutputs )
    {
        
        regExp.setPattern( "*" + str + "*" );
        regExp.setPatternSyntax( QRegExp::Wildcard );    
        
        // if the expected output is not found in order, generate error
        if ( ( currPos = tst_SxeMonitor::outputs.indexOf( regExp , prevPos) ) < 0  )
        {
            error = QString("The following expected output has not been found ") 
                  +  "or is not in it's expected position:\n" 
                  +   str;
            tst_SxeMonitor::outputs.clear();
            return false;
        } 
        else //else tag the element for removal 
        {
            tst_SxeMonitor::outputs.replace( currPos, emptyTag ); 
        }
        prevPos = currPos;
    }
    outputs.removeAll( emptyTag );
   
    //generate warning for all ouputs not found in expectedOutputs 
    if ( tst_SxeMonitor::outputs.size() != 0 )
        QWARN( "More qLogged outputs than expected, unexpected qLogs listed below" );

    foreach( QString str, tst_SxeMonitor::outputs )
        QWARN( qPrintable(str) ) ;   
    
    tst_SxeMonitor::outputs.clear();
    return true;
}

#ifdef QT_NO_QWS_VFB
/*?
  \internal
  Fills the security log to a specified size.
  Precondition: \a size <= \c SxeMonitor::maxLogSize  
*/
void tst_SxeMonitor::fillLog( long size )
{
    if ( size > SxeMonitor::maxLogSize )
    {
        QWARN( "fillLog() invoked with parameter size > SxeMonitor::maxLogSize" );
        return;
    }
    else if ( size == logFile.size() ) 
    {
        return;
    }
    // remove the log file if the size cannot be reached by appending to the log file
    else if ( (size < ( logFile.size() + minLogEntrySize )) && !logFile.remove() )
    {
        QWARN( qPrintable( QString("fillLog() cannot remove ") + logFile.fileName() ) );  
    }
    
    int numEntries =  ( size - logFile.size() ) / minLogEntrySize;
    int remainderBytes = ( size - logFile.size() ) % minLogEntrySize;
    
    for ( int i = 0; i < numEntries -1; i++ )
        syslog( LOG_ERR | LOG_LOCAL6, "%s", "");
    
    QString str;
    str.fill( 'a', remainderBytes );
    syslog( LOG_ERR | LOG_LOCAL6, "%s", qPrintable(str) );
    Qtopia::sleep( 2 );
}

/*?
  \internal
  
  Calculates and sets minLogEntrySize and sxeLogEntrySize.  An empty
  log entry is inserted into the security log file to make the 
  calculations.
  Precondition: the security log file must already exist and should
  not be near its maximum size.  
*/
void tst_SxeMonitor::calcLogEntrySizes()
{
    if ( !logFile.exists() )
        QFAIL( "Cannot calculate log entry sizes, log file does not exit" );
    
    int initialLogSize = logFile.size();
    
    syslog( LOG_ERR | LOG_LOCAL6, "%s", "");
    Qtopia::sleep( 2 );
    if ( logFile.size() == initialLogSize )
        QFAIL( "Cannot calculate log entry sizes, logging is not occurring" ); 
    
    minLogEntrySize = logFile.size() - initialLogSize;
    sxeLogEntrySize = minLogEntrySize  
                        + ( tst_SxeMonitor::sxeTag + " " 
                        + QString::number( tst_SxeMonitor::basePID) + " " + tst_SxeMonitor::testRequest ).length();  
}

#endif //QT_NO_QWS_VFB

/*?
  \internal
  
  LoadThread's run method that continually updates the security log 
  file with dummy sxe log entries.
*/
void LoadThread::run()
{
    for ( int i = 0; i < tst_SxeMonitor::numIterations; i++)
        syslog( LOG_ERR | LOG_LOCAL6, "%s %s", 
            qPrintable(tst_SxeMonitor::sxeTag), qPrintable(tst_SxeMonitor::testRequest) );
}

QTEST_APP_MAIN(tst_SxeMonitor, QtopiaApplication)
#include "tst_sxemonitor.moc"
