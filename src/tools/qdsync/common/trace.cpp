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
#include "trace.h"

#include <QMutex>
#include <QThreadStorage>

#include <string.h>
#include <time.h>

#ifdef QTOPIA_DESKTOP
#include <desktopsettings.h>
#else
#include <QSettings>
#endif

//#define TRACE_DEBUG
//#define NO_THREAD_STORAGE

QD_EXPORT bool &qdTraceEnabled();
bool &qdTraceEnabled()
{
    static bool enabled = true;
    return enabled;
}

static bool timestampEnabled()
{
    static int enabled = -1;
    if ( enabled == -1 ) {
#ifdef QTOPIA_DESKTOP
        DesktopSettings settings;
        enabled = settings.value("/settings/TraceTimestamp", 0).toInt();
#else
        QSettings settings("Trolltech", "qdsync");
        enabled = settings.value("/TraceTimestamp", 0).toInt();
#endif
    }
    return (enabled == 1);
}

/*!
  \headerfile <trace.h>
  \title <trace.h>
  \ingroup headers
  \brief The <trace.h> header contains some useful debugging macros.

  The <trace.h> header contains some useful debugging macros.

  You use the macros in this file like this.

  \code
    void MyClass::MyFunc()
    {
        TRACE(TRACE) << "MyClass::MyFunc";

        otherfunc( 1 );
    }
    void MyClass::otherfunc( int arg )
    {
        TRACE(TRACE) << "MyClass::otherfunc" << "arg" << arg;
        LOG() << "Message";
    }
  \endcode

  The output looks like this.

  \code
    T1 MyClass::MyFunc()
    T1 {
    T1     MyClass::otherfunc() arg 1
    T1     {
    T1         Message
    T1     }
    T1 }
  \endcode

  For each new thread seen, the number is incremented (eg. T2, T3, etc.).
*/

/*!
  \macro TRACE(CATEGORY)
  \relates <trace.h>

  Register the logging category to use (TRACE if \a CATEGORY is null).
  This statement must appear before calls to LOG() so that indenting happens correctly.
  See \l <qtopiadesktoplog.h> for the available categories.
*/

/*!
  \macro LOG()
  \relates <trace.h>

  Use this macro like qDebug() and the output will be indented.
*/

/*!
  \macro WARNING()
  \relates <trace.h>

  Use this macro like qWarning() and the output will be indented.
  Note that calls to WARNING() cannot be turned off like calls to LOG().
*/

#ifndef NO_THREAD_STORAGE
struct indentData {
    int threadNumber;
    int indent;
};
static int _threadNumber = 0;
static QThreadStorage<indentData*> _indent;
#else
static int _indent = 0;
#endif

static QMutex mutex;

_Trace::_Trace( const char *_category, bool (*enabledHook) () )
    : category( _category )
{
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_Trace::_Trace";
#endif
    mEnabled = qdTraceEnabled() && enabledHook();
    if ( mEnabled ) {
#ifdef TRACE_DEBUG
        qLog(TRACE) << "enabled";
#endif
        mutex.lock();
#ifndef NO_THREAD_STORAGE
        if ( ! _indent.hasLocalData() ) {
#ifdef TRACE_DEBUG
            qLog(TRACE) << "no thread local data";
#endif
            indentData *data = new indentData;
            data->threadNumber = ++_threadNumber;
            data->indent = 0;
            _indent.setLocalData( data );
        }
#else
        indent = _indent++;
#endif
        mutex.unlock();
#ifndef NO_THREAD_STORAGE
        indent = (_indent.localData()->indent)++;
        threadNumber = (_indent.localData()->threadNumber);
#endif
#ifdef TRACE_DEBUG
        qLog(TRACE) << "indent" << indent;
        qLog(TRACE) << "thread number" << threadNumber;
#endif
    } else {
        indent = 0;
        threadNumber = 0;
    }
}

_Trace::~_Trace()
{
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_Trace::~_Trace";
#endif
    if ( mEnabled ) {
#ifdef TRACE_DEBUG
        qLog(TRACE) << "enabled";
#endif
#ifndef NO_THREAD_STORAGE
        (_indent.localData()->indent)--;
#else
        mutex.lock();
        _indent--;
        mutex.unlock();
#endif

        indent--;
        log() << "}";
    }
}

bool _Trace::enabled()
{
    return mEnabled;
}

_TraceDebugMethodPrefix _Trace::methodLog()
{
    if ( indent < 0 || !mEnabled )
        indent = 0;
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_Trace::methodLog() indent == " << indent;
#endif
    int ind = indent;
    indent++;

    QString indentspaces;
    if ( ind ) {
        char buf[1000];
        int i = 0;
        for ( i = 0; i < ind; i++ ) {
            int x = i*4;
            buf[x] = ' ';
            buf[x+1] = ' ';
            buf[x+2] = ' ';
            buf[x+3] = ' ';
        }
#ifndef NO_THREAD_STORAGE
        buf[i*4-1] = ' ';
        buf[i*4] = '\0';
#else
        buf[i*4-1] = '\0';
#endif
        indentspaces = buf;
    }

#ifndef NO_THREAD_STORAGE
    QString arg;
    if ( timestampEnabled() ) {
        time_t timestamp = ::time(0);
        arg = QString("%1-T%2%3").arg(timestamp).arg(threadNumber, 1).arg(indentspaces);
    } else {
        arg = QString("T%1%2").arg(threadNumber, 1).arg(indentspaces);
    }
    _TraceDebugMethodPrefix r( arg );
#else
    _TraceDebugMethodPrefix r( indentspaces );
#endif
    //r << category << ":";

    return r;
}

QDebug _Trace::log()
{
    if ( indent < 0 || !mEnabled )
        indent = 0;
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_Trace::log() indent ==" << indent;
#endif
    int ind = indent;
    QDebug r(QtDebugMsg);
    //r << category << ":";

#ifndef NO_THREAD_STORAGE
    if ( timestampEnabled() ) {
        time_t timestamp = ::time(0);
        r << QString("%1-T%2").arg(timestamp).arg(threadNumber, 1).toLocal8Bit().constData();
    } else {
        r << QString("T%1").arg(threadNumber, 1).toLocal8Bit().constData();
    }
#endif
    if ( ind ) {
        char buf[1000];
        int i = 0;
        for ( i = 0; i < ind; i++ ) {
            int x = i*4;
            buf[x] = ' ';
            buf[x+1] = ' ';
            buf[x+2] = ' ';
            buf[x+3] = ' ';
        }
        buf[i*4-1] = '\0';
        r << buf;
    }
    return r;
}

// =====================================================================

_TraceDebugMethodPrefix::_TraceDebugMethodPrefix( const QString &_indentspaces )
    : QDebug( QtDebugMsg ), _tstate( 0 ), indentspaces( _indentspaces )
{
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_TraceDebugMethodPrefix";
#endif
    if ( indentspaces.count() )
        QDebug::operator<<(indentspaces.toLocal8Bit().constData());
}

_TraceDebugMethodPrefix::_TraceDebugMethodPrefix( const _TraceDebugMethodPrefix &other )
    : QDebug( other ), _tstate( other._tstate ), indentspaces( other.indentspaces )
{
    ((_TraceDebugMethodPrefix*)&other)->_tstate = -1;
#ifdef TRACE_DEBUG
    qLog(TRACE) << "_TraceDebugMethodPrefix(other)";
#endif
}

_TraceDebugMethodPrefix::~_TraceDebugMethodPrefix()
{
    if ( _tstate == -1 ) {
#ifdef TRACE_DEBUG
        qLog(TRACE) << "~_TraceDebugMethodPrefix(orphaned)";
#endif
    } else {
#ifdef TRACE_DEBUG
        qLog(TRACE) << "~_TraceDebugMethodPrefix";
#endif
        QDebug::operator<<(endl);
        if ( indentspaces.count() )
            QDebug::operator<<(indentspaces.toLocal8Bit().constData());
        QDebug::operator<<("{");
    }
}

// =====================================================================

#ifndef QTOPIA_DESKTOP
Q_GLOBAL_STATIC_WITH_ARGS(QSettings, logSettings, ("Trolltech", "Log"));

bool qdsync::registerQtopiaLog(const char *category)
{
    logSettings()->setValue(QLatin1String(category)+"/Registered", 1);
    return qtopiaLogRequested(category);
}
#endif

