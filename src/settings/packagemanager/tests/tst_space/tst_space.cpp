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

#include <QtopiaApplication>
#include <QObject>
#include "../../utils.h"
#include <shared/qtopiaunittest.h>
#include <QVariant>

qulonglong g_installSpace;

class tst_Space:public QObject
{
    Q_OBJECT
    private slots:
        void isSufficientSpace();
        void isSufficientSpace_data();
    private:
        static const qulonglong MaxControlSize;
};

QTEST_APP_MAIN(tst_Space, QtopiaApplication)
#include "tst_space.moc"

const qulonglong tst_Space::MaxControlSize = 10 * 1024;

void tst_Space::isSufficientSpace()
{
    QFETCH(QString, pkgSize);
    QFETCH(QString, installedSize);
    QFETCH(qulonglong, installSpace);
    QFETCH(bool, Result);
    InstallControl::PackageInfo pkgInfo;
    pkgInfo.downloadSize = pkgSize;
    pkgInfo.installedSize = installedSize;
    g_installSpace = installSpace;
    QVERIFY( SizeUtils::isSufficientSpace(pkgInfo, NULL) == Result);
}

void tst_Space::isSufficientSpace_data()
{
    QTest::addColumn<QString>("pkgSize"); //P
    QTest::addColumn<QString>("installedSize");//I
    QTest::addColumn<qulonglong>("installSpace");//is

    QTest::addColumn<bool>("Result");

    qulonglong pkgSize = 1000;
    QString pkgSizeStr = QString::number(pkgSize);
    qulonglong installedSize = 2000;
    QString installedSizeStr = QString::number(installedSize);

    //R
    qulonglong requiredSize = installedSize + pkgSize +
                                MaxControlSize;

    //These test whether there is enough install space
    //for install a package
    QTest::newRow("is << R")
            << pkgSizeStr << installedSizeStr
            << requiredSize/2
            << false;
    QTest::newRow("is < R")
            << pkgSizeStr << installedSizeStr
            << requiredSize - 1
            << false;
    QTest::newRow("is == R" )
            << pkgSizeStr << installedSizeStr
            << requiredSize
            << true;
    QTest::newRow("is > R")
            << pkgSizeStr << installedSizeStr
            << requiredSize + 1
            << true;
    QTest::newRow("is >> R" )
            << pkgSizeStr << installedSizeStr
            << requiredSize * 2
            << true;
}


namespace SizeUtils
{
    qulonglong availableSpace(QString path)
    {
            return g_installSpace;
    }
}
