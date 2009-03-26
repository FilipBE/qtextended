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
#include <QDawg>
#include <QStringList>
#include <QtopiaApplication>
#include <QFile>
#include <QIODevice>

#include <shared/qtopiaunittest.h>

class tst_QDawg : public QObject
{
    Q_OBJECT

private slots:
    // initTestCase called once before any tests are run
    void initTestCase()
    {

    }

    // Called before each individual test
    void init()
    { 
    }

    void tst_CreateFromWords_StringList() {
        // first test the simple single-word case
        QStringList sampleStringList;
        QString word("foo");
        sampleStringList.append(word);

        QDawg singleWordDawg;
        singleWordDawg.createFromWords(sampleStringList);
        QVERIFY( singleWordDawg.countWords() == 1 );
        QVERIFY( singleWordDawg.contains(word) );
        QVERIFY( singleWordDawg.allWords() == sampleStringList );
        // check for partial match
        while(word.length())
        {
            word.chop(1);
            QString errorString = QString("False positive checking for partial word ")+word;
            QVERIFY2( !singleWordDawg.contains(word) ,  errorString.toAscii().data());
        }

    };

    // Make sure that the words that are pulled from the file are the 
    // same words that are in the file.
    // also depends on/tests QDawg::allWords()
    void tst_create_fromWords_QIODevice(){
        QDawg dawg;
        
        QDir dataDir = QtopiaUnitTest::baseDataPath();
        
        QFile fileForDawg(dataDir.filePath("words"));
        QVERIFY(fileForDawg.open(QIODevice::ReadOnly | QIODevice::Text));
        dawg.createFromWords(&fileForDawg);
        QStringList fromDawg = dawg.allWords();
        QStringList fromFile;

        QFile fileForStringList(dataDir.filePath("words"));
        QVERIFY(fileForStringList.open(QIODevice::ReadOnly | QIODevice::Text));
        while (!fileForStringList.atEnd()) {
         fromFile.append(fileForStringList.readLine().simplified());
        }
        fromDawg.sort();
        fromFile.sort();
        QVERIFY(fromFile == fromDawg);
    };

    // test that writing to a qdawg file and reading from it recovers
    // the same wordlist
    
    void tst_read_write_writeFile() {
        QDawg dawg;
        
        QDir dataDir = QtopiaUnitTest::baseDataPath();
        
        QFile fileForDawg(dataDir.filePath("words"));
        QVERIFY(fileForDawg.open(QIODevice::ReadOnly | QIODevice::Text));
        dawg.createFromWords(&fileForDawg);
        fileForDawg.close();

        // create temporary directory to store data for tests.
        QDir dataPath(QDir::homePath() + "/autotestTmpData/");
        bool tmpDirectoryExists = dataPath.exists();
        if(!tmpDirectoryExists)
            dataPath.mkpath(dataPath.absolutePath());

        QFile *dawgFileOut = new QFile(dataPath.filePath("tst_qdawg.dawg"));
        // test that qdawg returns false for unopened file.
        QVERIFY(!dawg.write(dawgFileOut));
        QVERIFY(dawgFileOut->open(QIODevice::WriteOnly));
        // QVERIFY will end test if open failed, so no further check necessary
        // test for successful writing.
        QVERIFY(dawg.write(dawgFileOut));
        dawgFileOut->close();

        // Test QDawg::read(QIODevice * dev ) for reading without mmap
        QDawg readDawgNoMmap;
        QFile *dawgFileIn = new QFile(dataPath.filePath("tst_qdawg.dawg"));
        QVERIFY(dawgFileIn->open(QIODevice::ReadOnly));

        QVERIFY(readDawgNoMmap.read(dawgFileIn));
        QStringList readWordsNoMmap = readDawgNoMmap.allWords();
        dawgFileIn->close();

        // Setup reference stringlist for comparisons.
        QStringList fromFile;
        QFile fileForStringList(dataDir.filePath("words"));
        QVERIFY(fileForStringList.open(QIODevice::ReadOnly | QIODevice::Text));
        while (!fileForStringList.atEnd()) {
         fromFile.append(fileForStringList.readLine().simplified());
        }

        QVERIFY(fromFile.count() == readWordsNoMmap.count());

        // Lazy sorting
        fromFile.sort();
        readWordsNoMmap.sort();
        // Actual test that what came out is what went in
        QVERIFY(fromFile == readWordsNoMmap);

        // Test QDawg::readFile ( const QString & filename ) 
        // for reading with mmap
        QDawg readDawgWithMmap;

        QVERIFY(readDawgWithMmap.readFile(dawgFileIn->fileName()));
        QStringList readWordsWithMmap = readDawgWithMmap.allWords();

        QVERIFY(fromFile.count() == readWordsWithMmap.count());

        readWordsWithMmap.sort();
        // Test for qdawg with mmap
        QVERIFY(fromFile == readWordsWithMmap);

        // cleanup
        dawgFileOut->remove(); // QVERIFY?
        if(!tmpDirectoryExists)
            dataPath.rmpath(dataPath.absolutePath());
    }

    // Called after each test is executed
    void cleanup()
    { 
    }

    // Called once after all tests have executed
    void cleanupTestCase()
    { 
    }

};

QTEST_APP_MAIN(tst_QDawg, QtopiaApplication)
#include "tst_qdawg.moc"

