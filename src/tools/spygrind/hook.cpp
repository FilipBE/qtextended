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

#include "qsignalspycollector.h"

#include <QCoreApplication>

void dump_to_file()
{
    QSignalSpyCollector::instance()->dumpToFile();
}

/*
    Main hook to load QSignalSpyCollector.
    A two-pronged attack: both qt_startup_hook and __attribute__((constructor))
    sometimes fail to do their job.  By applying both, hopefully we increase the
    chances of this function being called.
*/
extern "C" __attribute__((constructor)) void qt_startup_hook()
{
    static bool created = false;
    if (!created) {
        created = true;
        QSignalSpyCollector::instance();
        qAddPostRoutine(dump_to_file);
    }
}

