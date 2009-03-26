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

#include "method_p.h"

QByteArray Method::toCallgrindFormat(QString const& command) const
{
    QByteArray ret;
    ret.append(
"version: 1\n"
"creator: spygrind\n"
    );
    ret.append(QString("cmd: %1\n").arg(command));

    ret.append(
"positions: line\n"
"event:  Real : Real time (nanoseconds)\n"
"event:  UserAndSys = User+Sys : User + System CPU time (microseconds)\n"
"event:  User : User CPU time (microseconds)\n"
"event:  Sys  : System CPU time (microseconds)\n"
"event:  Malloc : Memory allocation by malloc/calloc/realloc (bytes)\n"
"event:  Free : Memory free by free/realloc (bytes)\n"
"event:  Leak : Local memory leak, UNRELIABLE (bytes)\n"
"events: Real User Sys Malloc Free Leak\n"
"\n"
    );

#define APPEND_COSTS_FOR_STAMP(stamp) do {  \
    qlonglong real   = qlonglong(stamp.tp.tv_nsec) + Q_INT64_C(1000000000)*qlonglong(stamp.tp.tv_sec);  \
    qlonglong system = qlonglong(stamp.ru.ru_stime.tv_usec) + Q_INT64_C(1000000)*qlonglong(stamp.ru.ru_stime.tv_sec);   \
    qlonglong user   = qlonglong(stamp.ru.ru_utime.tv_usec) + Q_INT64_C(1000000)*qlonglong(stamp.ru.ru_utime.tv_sec);   \
    ret.append(QByteArray::number(real));   \
    ret.append(" ");                        \
    ret.append(QByteArray::number(user));   \
    ret.append(" ");                        \
    ret.append(QByteArray::number(system)); \
    ret.append(" ");                        \
    ret.append(QByteArray::number(stamp.bytesAllocated));   \
    ret.append(" ");                                        \
    ret.append(QByteArray::number(stamp.bytesFreed));       \
    ret.append(" ");                                        \
    ret.append(QByteArray::number(stamp.bytesLeaked));      \
} while(0)

    QSet<Method*> methods;
    methods.insert(const_cast<Method*>(this));
    methods |= allCallees();

    foreach (Method* m, methods) {
        ret.append("\n");

        ret.append("fn=");
        ret.append(m->signature);
        ret.append("\n");

        ret.append("0 ");
        APPEND_COSTS_FOR_STAMP(m->exclusiveStamp);
        ret.append("\n");
        foreach (Method* cm, m->callees.keys()) {
            ret.append("cfn=");
            ret.append(cm->signature);
            ret.append("\n");

            ret.append("calls=");
            ret.append(QByteArray::number(m->callees[cm].callCount));
            ret.append(" 0\n"); // BOGUS location

            // cost
            ret.append("0 ");
            APPEND_COSTS_FOR_STAMP(m->callees[cm].stamp);
            ret.append("\n");
        }
    }

    return ret;
}

