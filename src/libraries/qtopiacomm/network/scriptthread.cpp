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

#include "scriptthread.h"

#include <QDebug>
#include <QProcess>

#include <qtopialog.h>

/*!
  \class ScriptThread
    \inpublicgroup QtBaseModule
  \brief The ScriptThread class provides serialized execution of applications/scripts.
  \internal


  The ScriptThread class is used by the Qt Extended network plug-ins. It allows the serialized execution 
  of scripts without blocking the caller. New scripts can be added via addScriptToRun() and
  are added to queue. The excution is based on the FIFO principle. Each time when 
  */

//This class is used by the Qtopia network plug-ins and is not a public class.
//In the future this might be replaced by Qt's improved concurrency API.

/*!
  Constructs a ScriptThread instance with the given \a parent.
  */
ScriptThread::ScriptThread( QObject * parent )
    : QThread( parent ), quit( false )
{

}

/*!
  Destroys the ScriptThread instance
  */
ScriptThread::~ScriptThread()
{
    quit = true;
    waitCond.wakeOne();
    wait();
}

/*!
  \internal
  \threadsafe
  Adds the script with the give \a scriptPath to the queue of executable scripts. \a parameter
  is the list of parameters that should be passed to the script.
  */
void ScriptThread::addScriptToRun( const QString& scriptPath, const QStringList& parameter )
{
    QMutexLocker lock(&mutex);
    qLog(Network) << "Adding new network script:" << scriptPath << parameter;
    scripts.append( scriptPath );
    params.append( parameter );

    if ( !isRunning() )
        start();
    else
        waitCond.wakeOne();
}

/*!
  \internal
  \threadsafe

  Returns the number of remaining jobs (including the currently running one) to be executed.
  */
int ScriptThread::remainingTasks() const
{
    QMutexLocker lock( &mutex );
    return scripts.count();
}

/*!
  \internal

  Executes the scripts queued up for this thread instance.
  */
void ScriptThread::run()
{
    mutex.lock();
    QString script = scripts.first();
    QStringList parameter = params.first();
    mutex.unlock();

    while ( !quit ) {
        qLog(Network) << "Executing network script" << script << parameter;
        QProcess::execute( script, parameter );

        mutex.lock();
        scripts.removeFirst();
        params.removeFirst();
        emit scriptDone();
        if ( scripts.isEmpty() ) {
            qLog(Network) << "All network scripts executed -> blocking";
            waitCond.wait( &mutex );
        }

        if ( !scripts.isEmpty() ) {
            script = scripts.first();
            parameter = params.first();
        } else if ( !quit ) {
            qLog(Network) << "Invalid state: No network scripts left to execute";
        }
        mutex.unlock();
    }

}
