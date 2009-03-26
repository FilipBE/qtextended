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

#include <qperformancelog.h>

/*
    The QPerformanceLog class is obsolete.
    For equivalent functionality, please see the section "Customizing Log Output" in the
    "Debugging Qt Extended Applications" document.
*/

struct QPerformanceLogData
{};

QPerformanceLog::QPerformanceLog(QString const&)
{}

QPerformanceLog::~QPerformanceLog()
{}

bool QPerformanceLog::enabled()
{ return false; }

QPerformanceLog &QPerformanceLog::operator<<(QString const&)
{ return *this; }

QPerformanceLog &QPerformanceLog::operator<<(Event const&)
{ return *this; }

QString QPerformanceLog::stringFromEvent(Event const&)
{ return QString(); }

void QPerformanceLog::adjustTimezone(QTime&)
{}

