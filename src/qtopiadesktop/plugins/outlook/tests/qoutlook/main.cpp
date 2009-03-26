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

#include <QtTest>
#include <qtopiadesktop>
#include <qdebug.h>
#include <QSignalSpy>
#include <windows.h>
#include "outlooksync.h"
#include <desktopsettings.h>
#include "outlookthread.h"
#include <QMessageBox>
#include <private/qdplugin_p.h>
#include <QProcess>
#include <QDateTime>
#include <qd_common.h>

class qoutlook: public QObject
{
    Q_OBJECT
private:
    double date;
    QDateTime dt;
private slots:
    void init()
    {
        QD_COMMON_INIT_TEST_CASE_BODY(
                false, // enable TRACE
                false, // wait for debugger
                );
    }

    void convert_and_back()
    {
        dt = QDateTime( QDate(2007, 1, 1), QTime(13, 0, 0) );
        date = qdatetime_to_date( dt );
        QDateTime back = date_to_qdatetime( date );
        QVERIFY( dt == back );
    }

    void check_sentinel()
    {
        date = 949998;
        dt = date_to_qdatetime( date );
        QVERIFY( dt == QDateTime() );
        double back = qdatetime_to_date( dt );
        QVERIFY( date == back );
    }
};
QTEST_MAIN(qoutlook)

#include "main.moc"
