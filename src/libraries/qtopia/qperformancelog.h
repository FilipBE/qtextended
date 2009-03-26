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

#ifndef QPERFORMANCELOG_H
#define QPERFORMANCELOG_H

#include <qtopiaglobal.h>

class QTime;
class QPerformanceLogData;

class QTOPIA_EXPORT QPerformanceLog
{
public:
    enum EventType {
        NoEvent = 0x00,

        Begin  = 0x01,
        End    = 0x02,

        LibraryLoading = 0x04,
        EventLoop      = 0x08,
        MainWindow     = 0x10
    };
    Q_DECLARE_FLAGS(Event, EventType)

    QPerformanceLog( QString const &applicationName = QString() );
    ~QPerformanceLog();
    QPerformanceLog &operator<<(QString const &string);
    QPerformanceLog &operator<<(Event const &event);

    static void adjustTimezone( QTime &preAdjustTime );
    static bool enabled();
    static QString stringFromEvent(Event const &event);

private:
    Q_DISABLE_COPY(QPerformanceLog)
    QPerformanceLogData *data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPerformanceLog::Event)

#endif
