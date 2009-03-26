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

#include <QDir>
#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include "qmemoryfile_p.h"

// Constants
static const int MAX_MEMORYFILES = 50;

//TESTED_CLASS=QMemoryFile
//TESTED_FILES=src/libraries/qtopiabase/qmemoryfile_p.h

/*
    The tst_QMemoryFile class provides unit tests for the QMemoryFile class.
    Note that QMemoryFile is not a part of the public Qt Extended API.
*/
class tst_QMemoryFile : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void useMemoryFile_data();
    void useMemoryFile();

private:
    QMemoryFile *createMemoryFile(const QString &fileName, int flags, int size);
    void destroyMemoryFiles();

    QMemoryFile **memFiles;
    QMemoryFile *currentFile;
    QString dataPath;
    QString fileName1, fileName2, fileName3, fileName4, fileName5;
    QString testData;
    int memFileCount;
    bool dataWasCreated;
    bool cleanupMemFiles;
};

QTEST_APP_MAIN(tst_QMemoryFile, QtopiaApplication)
#include "tst_qmemoryfile.moc"

/*?
    Initialisation prior to the first test.
    This function initialises private members and creates files
    for later use by QMemoryFile.
*/
void tst_QMemoryFile::initTestCase()
{
    // Allocate our buffer
    memFileCount = 0;
    memFiles = new QMemoryFile* [MAX_MEMORYFILES];
    for (int index = 0; index < MAX_MEMORYFILES; index++)
        memFiles[index] = 0;

    // Setup and create the test files
    dataPath = QDir::homePath() + "/autotestTmpData/";

    fileName1 = dataPath + "testdata1.txt";
    fileName2 = dataPath + "testdata2.txt";
    fileName3 = dataPath + "testdata3.txt";
    fileName4 = "\\\\block1";
    fileName5 = "\\\\block2";

    testData = QString(
        " Text file line 1\n" \
        " Text file line 2\n" \
        " Text file line 3\n");
    QFile f1(fileName1), f2(fileName2);

    QDir d;
    d.mkpath(dataPath);
    // Make sure there is no filename3 - needed for test
    d.remove(fileName3);

    dataWasCreated = false;
    if ( f1.open(QIODevice::WriteOnly) ) {
        if ( f1.write(testData.toLatin1()) && f2.open( QIODevice::WriteOnly ) ) {
            if ( f2.write(testData.toLatin1()) ) {
                dataWasCreated = true;
            }
            f2.close();
        }
        f1.close();
    }

    currentFile = 0;
}

/*?
    Cleanup after last test.
    Destroys all memory files, deallocates memory, and removes all
    files created for the tests.
*/
void tst_QMemoryFile::cleanupTestCase()
{
    destroyMemoryFiles();
    delete [] memFiles;

    // Remove the created files/directories
    QFile::remove(fileName1);
    QFile::remove(fileName2);
    QFile::remove(fileName3);
    QDir d;
    d.rmdir(dataPath);
}

/*?
    Initialisation before each test.
    Sets a private member indicating that memory files shouldn't be
    destroyed after the next test; if the test wants memory files to
    be destroyed, it must set cleanupMemFiles to true before the test
    completes.
*/
void tst_QMemoryFile::init()
{
    cleanupMemFiles = false;
}

/*?
    Cleanup after each test.
    Destroys memory files if and only if cleanupMemFiles is true.
*/
void tst_QMemoryFile::cleanup()
{
    if (cleanupMemFiles)
        destroyMemoryFiles();
}

/*?
    Helper function; creates a new QMemoryFile with the given filename,
    flags and size, and stores a pointer to it in a private buffer.
*/
QMemoryFile *tst_QMemoryFile::createMemoryFile(const QString &fileName,
    int flags, int size)
{
    QMemoryFile *f = 0;

    if (memFileCount < MAX_MEMORYFILES) {
        f = new QMemoryFile(fileName, flags, size);
        memFiles[memFileCount++] = f;
    }

    return f;
}

/*?
    Helper function; destroys all existing memory file objects.
    However, it does not delete the underlying files when they
    exist.
*/
void tst_QMemoryFile::destroyMemoryFiles()
{
    if (memFiles) {
        for (int index = 0; index < memFileCount; index++) {
            delete memFiles[index];
            memFiles[index] = 0;
        }
        memFileCount = 0;
    }
    if (currentFile)
        currentFile = 0;
}

/*?
    Data for the acc_useMemoryFile test function.
    Order is very important for these test cases; test cases
    often depend on data written in previous test cases.
*/
void tst_QMemoryFile::useMemoryFile_data()
{
    const int BLOCKSIZE = 32;

    // Define the test elements we're going to use
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("flags");
    QTest::addColumn<int>("size");
    QTest::addColumn<char>("baseChar");
    QTest::addColumn<QString>("expected");  // Expected file contents
                                            // (usually null/empty)
    QTest::addColumn<bool>("writeToFile");  // Try to write to file?
    QTest::addColumn<bool>("destroyFiles"); // Destroy created files after test?

    // Test the ability to read a file
    // Expect "testData" because we have not overwritten it yet
    QTest::newRow("data0") << fileName1
        << 0 << BLOCKSIZE << ' ' << testData << false << false;

    // Destroy file
    // Expect "testData" because we have not overwritten it yet
    QTest::newRow("data1") << fileName1
        << 0 << BLOCKSIZE << ' ' << testData << false << true;

    // Test writing to a shared file
    QTest::newRow("data2") << fileName1
        << ( QMemoryFile::Write | QMemoryFile::Shared )
        << BLOCKSIZE << '0' << QString() << true << false;

    // Check what was written
    QTest::newRow("data3") << fileName1
        << ( QMemoryFile::Write | QMemoryFile::Shared )
        << BLOCKSIZE << '0' << QString() << false << false;

    // Write another character
    QTest::newRow("data4") << fileName1
        << ( QMemoryFile::Write | QMemoryFile::Shared )
        << BLOCKSIZE << 'a' << QString() << true << false;

    // Check what was written
    QTest::newRow("data5") << fileName1
        << ( QMemoryFile::Write | QMemoryFile::Shared )
        << BLOCKSIZE << 'a' << QString() << false << true;

    // Test writing to as a private copy
    QTest::newRow("data6") << fileName2
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << '0' << QString() << true << false;

    // Check what was written
    QTest::newRow("data7") << fileName2
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << '0' << QString() << false << false;

    // Write another character
    QTest::newRow("data8") << fileName2
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << 'a' << QString() << true << false;

    // Check what was written
    QTest::newRow("data9") << fileName2
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << 'a' << QString() << false << true;

    // Test writing to non-existing file - should be created
    QTest::newRow("data10") << fileName3
        << ( QMemoryFile::Write |  QMemoryFile::Shared | QMemoryFile::Create )
        << BLOCKSIZE << '0' << QString() << true << false;

    // Check what was written
    QTest::newRow("data11") << fileName3
        << ( QMemoryFile::Write  |  QMemoryFile::Shared )
        << BLOCKSIZE << '0' << QString() << false << false;

    // Write another character
    QTest::newRow("data12") << fileName3
        << ( QMemoryFile::Write  |  QMemoryFile::Shared )
        << BLOCKSIZE << 'a' << QString() << true << false;

    // Check what was written
    QTest::newRow("data13") << fileName3
        << ( QMemoryFile::Write |  QMemoryFile::Shared )
        << BLOCKSIZE << 'a' << QString() << false << true;

    // Test writing to a named block of memory
    QTest::newRow("data14") << fileName4
        << ( QMemoryFile::Write  | QMemoryFile::Create )
        << BLOCKSIZE << '0' << QString() << true << false;

    // Check what was written
    QTest::newRow("data15") << fileName4
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << '0' << QString() << false << false;

    // Test writing to a named block of memory
    QTest::newRow("data16") << fileName5
        << ( QMemoryFile::Write  | QMemoryFile::Create )
        << BLOCKSIZE << 'a' << QString() << true << false;

    // Check what was written
    QTest::newRow("data17") << fileName5
        << static_cast<int>(QMemoryFile::Write)
        << BLOCKSIZE << 'a' << QString() << false << false;
}

/*?
    Test function for reading and writing from and to a QMemoryFile.
    This test:
        * Creates a new QMemoryFile with a given filename if necessary,
          or uses the last created QMemoryFile if instructed to do so.
        * Verifies that the QMemoryFile was constructed correctly.
        * If so instructed, writes a special sequence of characters to
          the QMemoryFile; otherwise, reads from the QMemoryFile and
          ensures data matches this special sequence of characters.
*/
void tst_QMemoryFile::useMemoryFile()
{
    // Verify that the files were created by initTestCase()
    QCOMPARE(dataWasCreated, true);

    QFETCH(QString, fileName);
    QFETCH(int, flags);
    QFETCH(int, size);
    QFETCH(char, baseChar);
    QFETCH(QString, expected);
    QFETCH(bool, writeToFile);
    QFETCH(bool, destroyFiles);

    // Leave the actual destroy operation to cleanup(), if we
    // want to destroy after this test.
    cleanupMemFiles = destroyFiles;

    // Create memory file, will be destroyed later
    QMemoryFile *f;
    if (currentFile) {
        f = currentFile;
    } else {
        f = createMemoryFile(fileName, flags, size);
        currentFile = f;
    }

    QVERIFY(f != 0);
    QVERIFY(f->data() != 0);
    char *data = f->data();
    
    if (writeToFile) {
        // fill the file with a sequence of 10 repeating characters
        for (uint i = 0; i < (f->size() - 1); i++)
            data[i] = baseChar + ( i % 10 );
        data[f->size() - 1 ] = '\0';
    } else {
        // Check every written character
        if ( expected.isEmpty() ) {
            expected = "";
            // if expected is empty, the file was filled by us
            // with a sequence of 10 repeating characters
            for (uint i = 0; i < (f->size() - 1); i++)
                expected.append(baseChar + ( i % 10 ));
        }
        QCOMPARE(QString(data), expected);
    }
}
