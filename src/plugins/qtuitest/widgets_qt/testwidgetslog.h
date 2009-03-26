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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef TESTWIDGETSLOG_H
#define TESTWIDGETSLOG_H

#include <QApplication>
#include <QStringList>

#ifdef QTOPIA_TARGET
# include <qtopialog.h>
#else
# include <QDebug>
# define qLog(A) if (1); else qDebug() << #A
#endif

/*
    This header provides a slight convenience layer on top of qLog.

    By using the TestWidgetsLog macro in the same way qLog would normally be used,
    a log message will be output which automatically includes the name of the current
    function.
*/

namespace TestWidgetsLogImpl
{
    /*
        Returns a log header to be output at the beginning of each TestWidgetsLog.
    */
    inline QByteArray header(char const*,int,char const* function)
    {
        QString ret = qApp->applicationName();

        // Extract just the name of the function (discard return type and arguments)
        QString id;
        QStringList l1 = QString(function).split(" ");
        QStringList l2 = l1.filter("(");
        if (l2.count()) id = l2.at(0);
        else            id = l1.at(0);
        if (id.contains("(")) id = id.left(id.indexOf("("));

        ret += " " + id;

        return ret.toLocal8Bit();
    }
};

#define TestWidgetsLog() qLog(QtUitestWidgets) << TestWidgetsLogImpl::header(__FILE__,__LINE__,Q_FUNC_INFO).constData()

#endif

