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
#ifndef TRACE_H
#define TRACE_H

#include <qdglobal.h>
#include <qdebug.h>

class QD_EXPORT _TraceDebugMethodPrefix : public QDebug
{
public:
    _TraceDebugMethodPrefix( const QString &indentspaces );
    _TraceDebugMethodPrefix( const _TraceDebugMethodPrefix &other );
    ~_TraceDebugMethodPrefix();

#define TRACEDEBUG_BODY\
    {\
        if ( _tstate == 0 ) {\
            _tstate++;\
            nospace();\
            QDebug::operator<<(t);\
            QDebug::operator<<("()");\
            space();\
        } else {\
            QDebug::operator<<(t);\
        }\
        return (*this);\
    }

    inline QDebug &operator<<(QChar t) TRACEDEBUG_BODY
    inline QDebug &operator<<(bool t) TRACEDEBUG_BODY
    inline QDebug &operator<<(char t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed short t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned short t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed int t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned int t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed long t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned long t) TRACEDEBUG_BODY
    inline QDebug &operator<<(qint64 t)TRACEDEBUG_BODY
    inline QDebug &operator<<(quint64 t)TRACEDEBUG_BODY
    inline QDebug &operator<<(float t) TRACEDEBUG_BODY
    inline QDebug &operator<<(double t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const char* t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QString & t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QLatin1String &t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QByteArray & t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const void * t) TRACEDEBUG_BODY
#undef TRACEDEBUG_BODY

private:
    int _tstate;
    QString indentspaces;
};

class QD_EXPORT _Trace
{
public:
    _Trace( const char *category, bool (*enabledHook) () );
    ~_Trace();
    bool enabled();
    _TraceDebugMethodPrefix methodLog();
    QDebug log();
private:
    void init();

    const char *category;
    int indent;
    int threadNumber;
    bool mEnabled;
};

template<class T> inline int _trace_opt(const T &) { return 1; }
/* Catch compile time enums (at most one symbol) */
template<> inline int _trace_opt<int>(const int & v) { return v; }

#define TRACE_OPTION(dbgcat,expr,regfunc) \
    class dbgcat##_TraceLog { \
    public: \
        static inline bool enabled() { static char mem=0; if (!mem) { regfunc(&mem); mem=(expr)?3:2; } return mem&1; }\
        static inline _Trace trace() { return _Trace(#dbgcat, dbgcat##_TraceLog::enabled); }\
    };

// So that TRACE() can work.
class _TraceLog {
public: \
    static inline bool enabled() { return true; }
    static inline _Trace trace() { return _Trace("", _TraceLog::enabled); }
};

#ifdef QTOPIA_DESKTOP
// Circular dependency :(
#include <qtopiadesktoplog.h>

#define TRACE(dbgcat)\
    /*qLog(TRACE) << "TRACE(" << #dbgcat << ") called in file" << __FILE__ << "line" << __LINE__;*/\
    _Trace _trace_object = dbgcat##_TraceLog::trace();\
    if (!_trace_object.enabled()); else _trace_object.methodLog()

#else
#include <qtopialog.h>

namespace qdsync {
    QD_EXPORT bool registerQtopiaLog(const char *category);
}

#define QD_LOG_OPTION(x)\
    QLOG_OPTION_SEMI_VOLATILE(QDSync_##x,qtopiaLogRequested("QDSync_" #x),qtopiaLogSemiVolatile)\
    TRACE_OPTION(QDSync_##x,qtopiaLogRequested("QDSync_" #x),qtopiaLogSemiVolatile)\
    static bool x##_Tracelog_reg = qdsync::registerQtopiaLog("QDSync_" #x);

// So that TRACE() can work.
QLOG_OPTION_SEMI_VOLATILE(QDSync_,qtopiaLogRequested("QDSync"),qtopiaLogSemiVolatile);
TRACE_OPTION(QDSync_,qtopiaLogRequested("QDSync"),qtopiaLogSemiVolatile);
static bool _Tracelog_reg = qdsync::registerQtopiaLog("QDSync");

#define TRACE(dbgcat)\
    /*qLog(TRACE) << "TRACE(" << #dbgcat << ") called in file" << __FILE__ << "line" << __LINE__;*/\
    _Trace _trace_object = QDSync_##dbgcat##_TraceLog::trace();\
    if (!_trace_object.enabled()); else _trace_object.methodLog()

#endif

#define LOG()\
    /*qLog(TRACE) << "LOG() called in file" << __FILE__ << "line" << __LINE__;*/\
    if (!_trace_object.enabled()); else _trace_object.log()

#define WARNING()\
    /*qLog(TRACE) << "WARNING() called in file" << __FILE__ << "line" << __LINE__;*/\
    _trace_object.log()

// Redefine Q_ASSERT to something USEFUL!
#ifndef Q_OS_UNIX
#undef Q_ASSERT
#define Q_ASSERT(x) if ( x ); else qFatal(QString("Q_ASSERT failed! %3 at %1 line %2").arg(__FILE__).arg(__LINE__).arg(#x).toLocal8Bit().constData())
#endif

#endif
