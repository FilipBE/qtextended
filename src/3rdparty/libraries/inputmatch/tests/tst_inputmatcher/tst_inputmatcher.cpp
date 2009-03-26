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

#include <QObject>
#include <QTest>
#include <qstring.h>
#include <pkimmatcher.h>

//TESTED_CLASS=InputMatcher
//TESTED_FILES=src/3rdparty/libraries/inputmatch/pkimmatcher.cpp

class tst_InputMatcher : public QObject
{
    Q_OBJECT

private slots:
    void collate();
    void collate_data();
};

QTEST_MAIN( tst_InputMatcher )

/*
    \req QTOPIA-271

    \groups
*/
void tst_InputMatcher::collate()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    InputMatcher m("text");
    QString result = m.collate(input);

    QCOMPARE(result, expected);
}

void tst_InputMatcher::collate_data()
{
    QTest::addColumn<QString>( "input");
    QTest::addColumn<QString>( "expected");

    QTest::newRow("Letters Only") << QString("home") << QString("4663");
    QTest::newRow("Mixed Case") << QString("HomE") << QString("4663");
    QTest::newRow("Empty String") << QString("") << QString("");
    QTest::newRow("Contains Punctuation") << QString("o. tempus") << QString("61 836787");
    QTest::newRow("Contains Numbers") << QString("9a55") << QString("9255");
}

#include "tst_inputmatcher.moc"
