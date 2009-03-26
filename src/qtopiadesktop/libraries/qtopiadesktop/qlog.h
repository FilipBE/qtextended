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

#ifndef QLOG_H
#define QLOG_H

#include <qdebug.h>
#include <qdglobal.h>

#ifndef QT_NO_LOG_STREAM

template<class T> inline int qLog_opt(const T &) { return 1; }

enum QLogUncategorized { _QLog=true }; // So "uncategorized" qLog() works.

/* Catch compile time enums (at most one symbol) */
template<> inline int qLog_opt<int>(const int & v) { return v; }

class QD_EXPORT QLogBase {
public:
    static QDebug log(const char*);
};

# define QLOG_DISABLE(dbgcat) \
    class dbgcat##_QLog : public QLogBase { \
    public: \
	static inline bool enabled() { return 0; }\
    };
# define QLOG_ENABLE(dbgcat) \
    class dbgcat##_QLog : public QLogBase { \
    public: \
	static inline bool enabled() { return 1; }\
    };
# define QLOG_UNCATEGORIZED() \
    class _QLog : public QLogBase { \
    public: \
	static inline bool enabled() { return 1; }\
    };
# define QLOG_OPTION_VOLATILE(dbgcat,expr) \
    class dbgcat##_QLog : public QLogBase { \
    public: \
	static inline bool enabled() { return expr; }\
    };
# define QLOG_OPTION(dbgcat,expr) \
    class dbgcat##_QLog : public QLogBase { \
    public: \
	static inline bool enabled() { static char mem=0; return (mem ? mem : (mem=(expr)?3:2))&1; } \
    };

# define QLOG_OPTION_SEMI_VOLATILE(dbgcat,expr,regfunc) \
    class dbgcat##_QLog : public QLogBase { \
    public: \
        static inline bool enabled() { static char mem=0; if (!mem) { regfunc(&mem); mem=(expr)?3:2; } return mem&1; } \
    };

# define qLog(dbgcat) if(!dbgcat##_QLog::enabled()); else dbgcat##_QLog::log(#dbgcat)
# define qLogEnabled(dbgcat) (dbgcat##_QLog::enabled())
#else
# define QLOG_DISABLE(dbgcat)
# define QLOG_UNCATEGORIZED()
# define QLOG_ENABLE(dbgcat)
# define QLOG_OPTION(dbgcat,expr)
# define QLOG_OPTION_VOLATILE(dbgcat,expr)
# define QLOG_OPTION_SEMI_VOLATILE(dbgcat,expr,regfunc)
# define qLog(dbgcat) if(1); else QNoDebug()
# define qLogEnabled(dbgcat) false
#endif

#endif
