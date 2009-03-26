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

#include <QDSData>
#include <QMimeType>
#include <QByteArray>
#include <QString>
#include <QUniqueId>
#include <QDebug>
#include <QtopiaApplication>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include "qdsdata_p.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// --------------------------- Constants --------------------------------------

static const QByteArray BYTEARRAY_TEST_DATA      = QByteArray( "Some test data" );
static const QByteArray BYTEARRAY_TEST_DATA_MOD1 = QByteArray( "Some modified data 1" );
static const QByteArray BYTEARRAY_TEST_DATA_MOD2 = QByteArray( "Some modified data 2" );
static const QString    BYTEARRAY_TEST_DATA_TYPE = QString( "text/plain" );

static const QString    BYTEARRAY_TEST_DATA_MOD2_TYPE = QString( "text/html" );

static const QByteArray BYTEARRAY_TEST_LARGE_DATA      = QByteArray( 1024, 'x' );
static const QByteArray BYTEARRAY_TEST_LARGE_DATA_MOD1 = QByteArray( 1024, 'y' );
static const QByteArray BYTEARRAY_TEST_LARGE_DATA_MOD2 = QByteArray( 1024, 'z' );

static const QString    STRING_TEST_DATA         = QString( "Some test string" );
static const QString    STRING_TEST_DATA_MOD1    = QString( "Some modified string 1" );
static const QString    STRING_TEST_DATA_MOD2    = QString( "Some modified string 2" );
static const QString    STRING_TEST_DATA_TYPE    = QString( "text/plain" );

static const QString    STRING_TEST_DATA_MOD2_TYPE  = QString( "text/html" );

static const QString    STRING_TEST_LARGE_DATA      = QString( BYTEARRAY_TEST_LARGE_DATA );
static const QString    STRING_TEST_LARGE_DATA_MOD1 = QString( BYTEARRAY_TEST_LARGE_DATA_MOD1 );
static const QString    STRING_TEST_LARGE_DATA_MOD2 = QString( BYTEARRAY_TEST_LARGE_DATA_MOD2 );

static const QString    FILE_TEST_FILENAME  = QString( "tst_QDSData.file" );
static const QString    FILE_TEST_FILENAME1 = QString( "tst_QDSData.file1" );
static const QString    FILE_TEST_FILENAME2 = QString( "tst_QDSData.file2" );
static const QString    FILE_TEST_FILENAME3 = QString( "tst_QDSData.file3" );
static const QString    FILE_TEST_FILENAME4 = QString( "tst_QDSData.file4" );
static const QString    FILE_TEST_FILENAME5 = QString( "tst_QDSData.file5" );
static const QString    FILE_TEST_DATA_TYPE = QString( "text/plain" );


//TESTED_CLASS=QDSData
//TESTED_FILES=src/libraries/qtopia/qdsdata.cpp

/*
    The tst_QDSData class is a unit test for the QDSData class.
*/
class tst_QDSData : public QObject
{
    Q_OBJECT
public:
    tst_QDSData();
    virtual ~tst_QDSData();

protected slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void qdslockedFileTestCase();
    void constructionTestCase();
    void storingTestCase();
    void modifyDataTestCase();
    void modifyStringTestCase();
    void modifyFileTestCase();
    void iodeviceTestCase();
    void dataStoreConsistencyTestCase();
    void transmissionSmallDataTestCase();
    void transmissionLargeDataTestCase();
    void comparisonOperatorTestCase();
    void broadcastTestCase();
};

QTEST_APP_MAIN( tst_QDSData, QtopiaApplication )
#include "tst_qdsdata.moc"

#define NICEREAD(A, B, C) {\
    int _bytes = 0, _ret;\
    do {\
        _ret = ::read(A, (B)+_bytes, (C) - _bytes);\
        if (-1 == _ret) {\
            if (errno != EINTR) {\
            QFAIL(QString("Error while reading: %1").arg(::strerror(errno)).toLatin1());\
            }\
        } else {\
            _bytes += _ret;\
        }\
    } while (_bytes < C);\
}

#define NICEWRITE(A, B, C) {\
    int _bytes = 0, _ret;\
    do {\
        _ret = ::write(A, (B)+_bytes, (C) - _bytes);\
        if (-1 == _ret) {\
            if (errno != EINTR) {\
            QFAIL(QString("Error while writing: %1").arg(::strerror(errno)).toLatin1());\
            }\
        } else {\
            _bytes += _ret;\
        }\
    } while (_bytes < C);\
}

#define FORK_BEGIN() {\
    int _f[2];\
    int _childPid;\
    int _err;\
    int _semId = ::semget(IPC_PRIVATE, 3, 0x600);\
    _err = errno;\
    if (-1 == _semId) {\
        QFAIL(QString("Couldn't obtain semaphores: %1").arg(::strerror(_err)).toLatin1());\
    }\
    union semun {\
        int val;\
        struct semid_ds *buf;\
        unsigned short  *array;\
        struct seminfo  *__buf;\
    } _semun;\
    struct sembuf _semop = { 0, 0, 0 };\
    {\
        unsigned short _init[3] = { 1, 1, 0 };\
        _semun.array = _init;\
        ::semctl(_semId, 0, SETALL, _semun);\
    }\
    char _childSection = 0;\
    char _parentSection = 0;\
    QVERIFY( 0 == ::pipe(_f) );\
    _childPid = ::fork();

#define FORK_CHILD_BEGIN() \
    _childSection = 1;\
    if ( 0 == _childPid ) {\
        ::close(_f[0]);\
        char _testChild[8192];\
        _testChild[8191] = '\0';\
        int _line;\
        int _len;\
        _semop.sem_num = 0;\
        _semop.sem_op = -1;\
        _semop.sem_flg = 0;\
        semop( _semId, &_semop, 1 );

#define FORK_CHILD_VERIFY(A) \
        _line = __LINE__;\
        _len = ::strlen(#A)+1;\
        if (_len > 8192) _len = 8192;\
        ::strncpy(_testChild, #A, 8192);\
        _testChild[8191] = '\0';\
        if (A) { NICEWRITE(_f[1], "y", 1); }\
        else   { NICEWRITE(_f[1], "n", 1); }\
        NICEWRITE(_f[1], &_line, sizeof(int));\
        NICEWRITE(_f[1], &_len, sizeof(int));\
        NICEWRITE(_f[1], _testChild, _len);\
        _semop.sem_num = 3;\
        _semop.sem_op = 1;\
        _semop.sem_flg = 0;\
        semop( _semId, &_semop, 1 );


#define FORK_CHILD_END() \
        _semop.sem_num = 1;\
        _semop.sem_op = -1;\
        _semop.sem_flg = 0;\
        semop( _semId, &_semop, 1 );\
        ::close(_f[1]);\
        ::exit(0);\
    }

#define FORK_PARENT_BEGIN() \
    _parentSection = 1;\
    if ( 0 != _childPid ) {\
        ::close(_f[1]);\
        char _c;\
        char _test[8192];\
        int _line;\
        int _len;\
        _semop.sem_num = 0;\
        _semop.sem_op = 0;\
        _semop.sem_flg = 0;\
        semop( _semId, &_semop, 1 );

#define FORK_PARENT_VERIFY()    \
        _semop.sem_num = 3;\
        _semop.sem_op = -1;\
        _semop.sem_flg = 0;\
        semop( _semId, &_semop, 1 );\
        NICEREAD(_f[0], &_c, 1);\
        NICEREAD(_f[0], &_line, sizeof(int));\
        NICEREAD(_f[0], &_len, sizeof(int));\
        NICEREAD(_f[0], &_test, _len);\
        if (!QTest::qVerify(_c == 'y', QString("forked test: %1").arg(_test).toLatin1(), "", __FILE__, _line))\
            return;



#define FORK_PARENT_END() \
        struct timespec _time = { 30, 0 };\
        _semop.sem_num = 1;\
        _semop.sem_op = 0;\
        _semop.sem_flg = 0;\
        if ( -1 == semtimedop( _semId, &_semop, 1, &_time ) && EAGAIN == errno ) {\
            QFAIL("Not enough calls to FORK_PARENT_VERIFY()");\
        }\
        ::close(_f[0]);\
        ::waitpid(_childPid, 0, NULL);\
    }


#define FORK_PARENT_VERIFY_ALL(A) \
    FORK_PARENT_BEGIN();\
    for (int _i = 0; _i < (A); ++_i) {\
        FORK_PARENT_VERIFY();\
    }\
    FORK_PARENT_END();

#define FORK_END() \
    ::semctl(_semId, 0, IPC_RMID);\
    if (!_childSection) QFAIL("FORK section did not contain FORK_CHILD_BEGIN()!");\
    if (!_parentSection) QFAIL("FORK section did not contain FORK_PARENT_BEGIN()!");\
}

tst_QDSData::tst_QDSData()
{
}

tst_QDSData::~tst_QDSData()
{
}

/*?
    Initialisation before all test cases.
*/
void tst_QDSData::initTestCase()
{
}

/*?
    Cleanup after all test cases.
*/
void tst_QDSData::cleanupTestCase()
{
}

void tst_QDSData::init()
{
}

void tst_QDSData::cleanup()
{
}

/*? Test the QDSLockedFile private class. */
void tst_QDSData::qdslockedFileTestCase()
{
    /* Open file in locked mode. */
    {
        QDSLockedFile testData( FILE_TEST_FILENAME );
        QVERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
        QVERIFY( testData.lock( QDSLockedFile::WriteLock, false ) );
        testData.write( BYTEARRAY_TEST_DATA );
    }

#if 0
    FORK_BEGIN() {
        FORK_CHILD_BEGIN() {
            /* Verify that the file was unlocked by destructor, and hence can be opened and locked again
            * from another process. */
            QDSLockedFile testData( FILE_TEST_FILENAME );
            FORK_CHILD_VERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
            FORK_CHILD_VERIFY( testData.lock( QDSLockedFile::WriteLock, false ) );
        } FORK_CHILD_END();
        FORK_PARENT_VERIFY_ALL(2);
    } FORK_END();
#endif

    /* Test write locks across one process and multiple processes. */
    {
        /* Open and lock file. */
        QDSLockedFile testData( FILE_TEST_FILENAME );
        QVERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
        QVERIFY( testData.lock( QDSLockedFile::WriteLock, false ) );
        testData.write( BYTEARRAY_TEST_DATA );

        /* Verify that file can be opened and locked again from this process. */
        QDSLockedFile testData2( FILE_TEST_FILENAME );
        QVERIFY( testData2.openLocked( QIODevice::WriteOnly, false ) );
        QVERIFY( testData2.lock( QDSLockedFile::WriteLock, false ) );

#if 0
        /* Verify that the file can be opened but not locked from another process. */
        FORK_BEGIN() {
            FORK_CHILD_BEGIN() {
                QDSLockedFile testData( FILE_TEST_FILENAME );
                FORK_CHILD_VERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
                FORK_CHILD_VERIFY( !testData.lock( QDSLockedFile::WriteLock, false ) );
            } FORK_CHILD_END();
            FORK_PARENT_VERIFY_ALL(2);
        } FORK_END();
#endif
    }

    /* Test write locks can be unlocked properly. */
    {
        /* Open and do not lock file. */
        QDSLockedFile testData( FILE_TEST_FILENAME );
        QVERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
        testData.write( BYTEARRAY_TEST_DATA );

        /* Verify that file can be opened again and locked from this process. */
        QDSLockedFile testData2( FILE_TEST_FILENAME );
        QVERIFY( testData2.openLocked( QIODevice::WriteOnly, false ) );
        QVERIFY( testData2.lock( QDSLockedFile::WriteLock, false ) );

        /* Unlock file. */
        QVERIFY( testData2.unlock() );

#if 0
        /* Verify that the file can be opened and locked from another process. */
        FORK_BEGIN() {
            FORK_CHILD_BEGIN() {
                QDSLockedFile testData( FILE_TEST_FILENAME );
                FORK_CHILD_VERIFY( testData.openLocked( QIODevice::WriteOnly, false ) );
                FORK_CHILD_VERIFY( testData.lock( QDSLockedFile::WriteLock, false ) );
            } FORK_CHILD_END();
            FORK_PARENT_VERIFY_ALL(2);
        } FORK_END();
#endif
    }

    /* Test read locks from single and multiple processes. */
    {
        /* Open and do not lock file. */
        QDSLockedFile testData( FILE_TEST_FILENAME );
        QVERIFY( testData.openLocked( QIODevice::ReadWrite, false ) );
        testData.write( BYTEARRAY_TEST_DATA );

        /* Verify that file can be opened again and locked from this process. */
        QDSLockedFile testData2( FILE_TEST_FILENAME );
        QVERIFY( testData2.openLocked( QIODevice::ReadWrite, false ) );
        QVERIFY( testData2.lock( QDSLockedFile::ReadLock, false ) );

#if 0
        /* Verify that the file can be opened and read locked from another process. */
        FORK_BEGIN() {
            FORK_CHILD_BEGIN() {
                QDSLockedFile testData( FILE_TEST_FILENAME );
                FORK_CHILD_VERIFY( testData.openLocked( QIODevice::ReadWrite, false ) );
                FORK_CHILD_VERIFY( testData.lock( QDSLockedFile::ReadLock, false ) );
                testData.close();
            } FORK_CHILD_END();
            FORK_PARENT_VERIFY_ALL(2);
        } FORK_END();

        /* Verify that the file can be opened but not write locked from another process. */
        FORK_BEGIN() {
            FORK_CHILD_BEGIN() {
                QDSLockedFile testData( FILE_TEST_FILENAME );
                FORK_CHILD_VERIFY( testData.openLocked( QIODevice::ReadWrite, false ) );
                FORK_CHILD_VERIFY( !testData.lock( QDSLockedFile::WriteLock, false ) );
                testData.close();
            } FORK_CHILD_END();
            FORK_PARENT_VERIFY_ALL(2);
        } FORK_END();
#endif
    }

    QDSLockedFile testData( FILE_TEST_FILENAME );
    QVERIFY( testData.remove() );
}

/*? Test the various QDSData constructors. */
void tst_QDSData::constructionTestCase()
{
    // Test constuction with a QByteArray and check that data is accessed correctly
    {
        QDSData byteData( BYTEARRAY_TEST_DATA, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( byteData.isValid() );
        QVERIFY( byteData.type().id() == BYTEARRAY_TEST_DATA_TYPE );
        QVERIFY( byteData.data() == BYTEARRAY_TEST_DATA );
        QVERIFY( byteData.toString() == QString( BYTEARRAY_TEST_DATA ) );
        QIODevice* byteIoPtr = byteData.toIODevice();
        QVERIFY( byteIoPtr != 0 );
        QByteArray checkData = byteIoPtr->readAll();
        QVERIFY( checkData == BYTEARRAY_TEST_DATA );
        delete byteIoPtr;
    }

    // Test constuction with a QString and check that data is accessed correctly
    {
        QDSData stringData( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( stringData.isValid() );
        QVERIFY( stringData.type().id() == STRING_TEST_DATA_TYPE );
        QVERIFY( stringData.data() == STRING_TEST_DATA.toLatin1() );
        QVERIFY( stringData.toString() == STRING_TEST_DATA );
        QIODevice* stringIoPtr = stringData.toIODevice();
        QVERIFY( stringIoPtr != 0 );
        QString checkString = stringIoPtr->readAll();
        QVERIFY( checkString == STRING_TEST_DATA );
        delete stringIoPtr;
    }

    // Test constuction with a QFile and check that data is accessed correctly
    {
        QFile testFile( FILE_TEST_FILENAME );
        QVERIFY( testFile.open( QIODevice::WriteOnly ) );
        testFile.write( BYTEARRAY_TEST_DATA );
        testFile.close();

        QDSData fileData( testFile, QMimeType( FILE_TEST_DATA_TYPE ) );
        QVERIFY( fileData.isValid() );
        QVERIFY( fileData.type().id() == FILE_TEST_DATA_TYPE );
        QByteArray test = fileData.data();
        QVERIFY( fileData.data() == BYTEARRAY_TEST_DATA );
        QVERIFY( fileData.toString() == QString( BYTEARRAY_TEST_DATA ) );

        QIODevice* fileIoPtr = fileData.toIODevice();
        QVERIFY( fileIoPtr != 0 );
        QByteArray checkData = fileIoPtr->readAll();
        QVERIFY( checkData == BYTEARRAY_TEST_DATA );
        delete fileIoPtr;

        testFile.remove();
    }

    // Test copy construction
    {
        QDSData byteData( BYTEARRAY_TEST_DATA, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( byteData.isValid() );
        QVERIFY( byteData.type().id() == BYTEARRAY_TEST_DATA_TYPE );
        QVERIFY( byteData.data() == BYTEARRAY_TEST_DATA );

        QDSData byteDataCopy( byteData );
        QVERIFY( byteDataCopy.isValid() );
        QVERIFY( byteDataCopy.type().id() == BYTEARRAY_TEST_DATA_TYPE );
        QVERIFY( byteDataCopy.data() == BYTEARRAY_TEST_DATA );
    }

    // Test assignment operator
    {
        QDSData empty;
        QVERIFY( !empty.isValid() );
        QVERIFY( empty.type().id() == QString() );
        QVERIFY( empty.data() == QByteArray() );
        QVERIFY( empty.toString() == QString() );

        QDSData stringData( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( stringData.isValid() );
        QVERIFY( stringData.type().id() == STRING_TEST_DATA_TYPE );
        QVERIFY( stringData.data() == STRING_TEST_DATA.toLatin1() );
        QVERIFY( stringData.toString() == STRING_TEST_DATA );

        empty = stringData;
        QVERIFY( empty.isValid() );
        QVERIFY( empty.type().id() == STRING_TEST_DATA_TYPE );
        QVERIFY( empty.data() == STRING_TEST_DATA.toLatin1() );
        QVERIFY( empty.toString() == STRING_TEST_DATA );
    }
}

/*? Ensure storing and retrieving data to and from the data store works correctly. */
void tst_QDSData::storingTestCase()
{
    // Create a data object, store it and recreate it using a QUniqueId key
    {
        QUniqueId key;
        {
            QDSData createData( BYTEARRAY_TEST_DATA,
                                QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
            QVERIFY( createData.isValid() );
            key = createData.store();
            QVERIFY( key != QUniqueId() );
        }

        // Retrieve stored data
        QDSData storedData( key );
        QVERIFY( storedData.isValid() );
        QVERIFY( storedData.data() == BYTEARRAY_TEST_DATA );
        QVERIFY( storedData.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        // Remove the data from the data store
        QVERIFY( storedData.remove() );
    }
}

/*?
    Ensure modification of small and large byte array data stored locally and in the data store
    works correctly.
*/
void tst_QDSData::modifyDataTestCase()
{
    // Test small locally stored data
    {
        QDSData data( BYTEARRAY_TEST_DATA, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA );

        QVERIFY( data.modify( BYTEARRAY_TEST_DATA_MOD1 ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD1 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        QVERIFY( data.modify( BYTEARRAY_TEST_DATA_MOD2,
                              QMimeType( BYTEARRAY_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD2 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_MOD2_TYPE );
    }

    // Test small persistent data
    {
        QDSData data( BYTEARRAY_TEST_DATA, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA );
        QVERIFY( data.store() != QUniqueId() );

        QVERIFY( data.modify( BYTEARRAY_TEST_DATA_MOD1 ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD1 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        QVERIFY( data.modify( BYTEARRAY_TEST_DATA_MOD2, 
                              QMimeType( BYTEARRAY_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD2 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_MOD2_TYPE );

        QVERIFY( data.remove() );
    }

    // Test large data
    {
        QDSData data( BYTEARRAY_TEST_LARGE_DATA, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA );

        QVERIFY( data.modify( BYTEARRAY_TEST_LARGE_DATA_MOD1 ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA_MOD1 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        QVERIFY( data.modify( BYTEARRAY_TEST_LARGE_DATA_MOD2,
                              QMimeType( BYTEARRAY_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA_MOD2 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_MOD2_TYPE );
    }

}

/*?
    Ensure modification of small and large string data stored locally and in the data store
    works correctly.
*/
void tst_QDSData::modifyStringTestCase()
{
    // Test small locally stored data
    {
        QDSData data( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == STRING_TEST_DATA );

        QVERIFY( data.modify( STRING_TEST_DATA_MOD1 ) );
        QVERIFY( data.data() == STRING_TEST_DATA_MOD1 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_TYPE );

        QVERIFY( data.modify( STRING_TEST_DATA_MOD2, 
                              QMimeType( STRING_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == STRING_TEST_DATA_MOD2 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_MOD2_TYPE );
    }

    // Test small persistent data
    {
        QDSData data( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == STRING_TEST_DATA );
        QVERIFY( data.store() != QUniqueId() );

        QVERIFY( data.modify( STRING_TEST_DATA_MOD1 ) );
        QVERIFY( data.data() == STRING_TEST_DATA_MOD1 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_TYPE );

        QVERIFY( data.modify( STRING_TEST_DATA_MOD2,
                              QMimeType( STRING_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == STRING_TEST_DATA_MOD2 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_MOD2_TYPE );

        QVERIFY( data.remove() );
    }

    // Test large data
    {
        QDSData data( STRING_TEST_LARGE_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == STRING_TEST_LARGE_DATA );

        QVERIFY( data.modify( STRING_TEST_LARGE_DATA_MOD1 ) );
        QVERIFY( data.data() == STRING_TEST_LARGE_DATA_MOD1 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_TYPE );

        QVERIFY( data.modify( STRING_TEST_LARGE_DATA_MOD2,
                              QMimeType( STRING_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == STRING_TEST_LARGE_DATA_MOD2 );
        QVERIFY( data.type().id() == STRING_TEST_DATA_MOD2_TYPE );
    }

    // Check that the data store is empty
}

/*?
    Ensure modification of small and large byte data stored in files
    works correctly.
*/
void tst_QDSData::modifyFileTestCase()
{
    // Create files of data
    QFile testData( FILE_TEST_FILENAME );
    {
        QVERIFY( testData.open( QIODevice::WriteOnly ) );
        testData.write( BYTEARRAY_TEST_DATA );
        testData.close();
    }

    QFile testDataMod1( FILE_TEST_FILENAME1 );
    {
        QVERIFY( testDataMod1.open( QIODevice::WriteOnly ) );
        testDataMod1.write( BYTEARRAY_TEST_DATA_MOD1 );
        testDataMod1.close();
    }

    QFile testDataMod2( FILE_TEST_FILENAME2 );
    {
        QVERIFY( testDataMod2.open( QIODevice::WriteOnly ) );
        testDataMod2.write( BYTEARRAY_TEST_DATA_MOD2 );
        testDataMod2.close();
    }

    QFile testDataLarge( FILE_TEST_FILENAME3 );
    {
        QVERIFY( testDataLarge.open( QIODevice::WriteOnly ) );
        testDataLarge.write( BYTEARRAY_TEST_LARGE_DATA );
        testDataLarge.close();
    }

    QFile testDataLargeMod1( FILE_TEST_FILENAME4 );
    {
        QVERIFY( testDataLargeMod1.open( QIODevice::WriteOnly ) );
        testDataLargeMod1.write( BYTEARRAY_TEST_LARGE_DATA_MOD1 );
        testDataLargeMod1.close();
    }

    QFile testDataLargeMod2( FILE_TEST_FILENAME5 );
    {
        QVERIFY( testDataLargeMod2.open( QIODevice::WriteOnly ) );
        testDataLargeMod2.write( BYTEARRAY_TEST_LARGE_DATA_MOD2 );
        testDataLargeMod2.close();
    }

    // Test small data
    {
        QDSData data( testData, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA );
        QVERIFY( data.store() != QUniqueId() );

        QVERIFY( data.modify( testDataMod1 ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD1 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        QVERIFY( data.modify( testDataMod2,
                              QMimeType( BYTEARRAY_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_DATA_MOD2 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_MOD2_TYPE );

        QVERIFY( data.remove() );
    }

    // Test large data
    {
        QDSData data( testDataLarge, QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA );

        QVERIFY( data.modify( testDataLargeMod1 ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA_MOD1 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_TYPE );

        QVERIFY( data.modify( testDataLargeMod2,
                              QMimeType( BYTEARRAY_TEST_DATA_MOD2_TYPE ) ) );
        QVERIFY( data.data() == BYTEARRAY_TEST_LARGE_DATA_MOD2 );
        QVERIFY( data.type().id() == BYTEARRAY_TEST_DATA_MOD2_TYPE );
    }

    // Remove files
    testData.remove();
    testDataMod1.remove();
    testDataMod2.remove();
    testDataLarge.remove();
    testDataLargeMod1.remove();
    testDataLargeMod2.remove();

}

/*?
    Verify that QDSData::toIODevice() returns a valid read-only QIODevice.
*/
void tst_QDSData::iodeviceTestCase()
{
    // Local case
    {
        QDSData data( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QIODevice* ioPtr = data.toIODevice();

        QVERIFY( ioPtr->openMode() == QIODevice::ReadOnly );

        delete ioPtr;
    }

    // Persistent case
    {
        QDSData data( STRING_TEST_DATA, QMimeType( STRING_TEST_DATA_TYPE ) );
        QVERIFY( data.isValid() );
        QVERIFY( data.store() != QUniqueId() );
        QIODevice* ioPtr = data.toIODevice();

        QVERIFY( ioPtr->openMode() == QIODevice::ReadOnly );

        delete ioPtr;
        QVERIFY( data.remove() );
    }
}

/*?
    Store and retrieve data to and from the data store and ensure data
    remains consistent.
*/
void tst_QDSData::dataStoreConsistencyTestCase()
{
    // Create some data
    QDSData* origData = new QDSData( STRING_TEST_DATA,
                                     QMimeType( STRING_TEST_DATA_TYPE ) );
    QVERIFY( origData->isValid() );
    QVERIFY( origData->data() == STRING_TEST_DATA.toLatin1() );
    QVERIFY( origData->type().id() == STRING_TEST_DATA_TYPE );

    // Create some copies
    QDSData* copyData1 = new QDSData( *origData );
    QVERIFY( copyData1->isValid() );
    QVERIFY( copyData1->data() == STRING_TEST_DATA.toLatin1() );
    QVERIFY( copyData1->type().id() == STRING_TEST_DATA_TYPE );

    QDSData* copyData2 = new QDSData( *origData );
    QVERIFY( copyData2->isValid() );
    QVERIFY( copyData2->data() == STRING_TEST_DATA.toLatin1() );
    QVERIFY( copyData2->type().id() == STRING_TEST_DATA_TYPE );

    // On one of the copies do a store
    QUniqueId key = copyData2->store();
    QVERIFY( key != QUniqueId() );

    // Delete both copies and the original
    delete origData;
    origData = 0;
    delete copyData1;
    copyData1 = 0;
    delete copyData2;
    copyData2 = 0;

    // Create the data with the key
    QDSData* storedData = new QDSData( key );
    QVERIFY( storedData->isValid() );
    QVERIFY( storedData->data() == STRING_TEST_DATA.toLatin1() );
    QVERIFY( storedData->type().id() == STRING_TEST_DATA_TYPE );

    // Remove the data and delete the object
    QVERIFY( storedData->remove() );
    delete storedData;
    storedData = 0;
}

/*?
    Ensure small (locally stored) data can be streamed in and out of
    a QDSData object successfully.
*/
void tst_QDSData::transmissionSmallDataTestCase()
{
    // Test transmission of small data (make sure it isn't stored in the data store)
    QDSData* origData = new QDSData( STRING_TEST_DATA,
                                     QMimeType( STRING_TEST_DATA_TYPE ) );
    QVERIFY( origData->isValid() );
    QVERIFY( origData->data() == STRING_TEST_DATA );
    QVERIFY( origData->type().id() == STRING_TEST_DATA_TYPE );

    QByteArray streamData;
    {
        QDataStream stream( &streamData, QIODevice::WriteOnly );
        stream << *origData;
    }
    delete origData;
    origData = 0;

    // Recreate the data object on receive, and check that it's the same data
    QDSData* receivedData = new QDSData();
    {
        QDataStream stream( &streamData, QIODevice::ReadOnly );
        stream >> *receivedData;
    }
    QVERIFY( receivedData->isValid() );
    QVERIFY( receivedData->data() == STRING_TEST_DATA );
    QVERIFY( receivedData->type().id() == STRING_TEST_DATA_TYPE );

    // Destroy the received data
    delete receivedData;
    receivedData = 0;

}

/*?
    Ensure large (stored in data store) data can be streamed in and out of
    a QDSData object successfully.
*/
void tst_QDSData::transmissionLargeDataTestCase()
{
    // Test transmission of large data (make sure it is stored in the data store)
    QDSData* origData = new QDSData( STRING_TEST_LARGE_DATA,
                                     QMimeType( STRING_TEST_DATA_TYPE ) );
    QVERIFY( origData->isValid() );
    QVERIFY( origData->data() == STRING_TEST_LARGE_DATA );
    QVERIFY( origData->type().id() == STRING_TEST_DATA_TYPE );

    QByteArray streamData;
    {
        QDataStream stream( &streamData, QIODevice::WriteOnly );
        stream << *origData;
    }
    delete origData;
    origData = 0;

    // Recreate the data object on receive, and check that it's the same data
    QDSData* receivedData = new QDSData();
    {
        QDataStream stream( &streamData, QIODevice::ReadOnly );
        stream >> *receivedData;
    }
    QVERIFY( receivedData->isValid() );
    QVERIFY( receivedData->data() == STRING_TEST_LARGE_DATA );
    QVERIFY( receivedData->type().id() == STRING_TEST_DATA_TYPE );

    // Destroy the received data
    delete receivedData;
    receivedData = 0;

}

/*? Test operator== */
void tst_QDSData::comparisonOperatorTestCase()
{
    // Empty data objects
    {
        QDSData empty1;
        QDSData empty2;
        QDSData empty1Copy( empty1 );
        QDSData local( BYTEARRAY_TEST_DATA,
                       QMimeType( BYTEARRAY_TEST_DATA_TYPE ));
        QDSData stored( BYTEARRAY_TEST_LARGE_DATA,
                        QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );

        QVERIFY( empty1 == empty2 );
        QVERIFY( empty2 == empty1Copy );
        QVERIFY( empty1 != local );
        QVERIFY( empty1 != stored );
        QVERIFY( local != stored );
    }

    // Local data objects
    {
        QDSData local1( BYTEARRAY_TEST_DATA,
                        QMimeType( BYTEARRAY_TEST_DATA_TYPE ));
        QDSData local2( BYTEARRAY_TEST_DATA_MOD1,
                        QMimeType( BYTEARRAY_TEST_DATA_TYPE ));
        QDSData local1Copy( local1 );

        QVERIFY( local1 != local2 );
        QVERIFY( local1 == local1Copy );
    }

    // Stored data objects
    {
        QDSData stored1( BYTEARRAY_TEST_LARGE_DATA,
                         QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QDSData stored2( BYTEARRAY_TEST_LARGE_DATA_MOD1,
                         QMimeType( BYTEARRAY_TEST_DATA_TYPE ) );
        QDSData stored1Copy( stored1 );

        QVERIFY( stored1 != stored2 );
        QVERIFY( stored1 == stored1Copy );
    }
}

/*?
    Ensure large (stored in data store) data can be streamed in and out of
    a QDSData object multiple times.
*/
void tst_QDSData::broadcastTestCase()
{
    // Test transmission of large data (make sure it is stored in the data store)
    QDSData* origData = new QDSData( STRING_TEST_LARGE_DATA,
                                     QMimeType( STRING_TEST_DATA_TYPE ) );
    QVERIFY( origData->isValid() );
    QVERIFY( origData->data() == STRING_TEST_LARGE_DATA );
    QVERIFY( origData->type().id() == STRING_TEST_DATA_TYPE );

    // Store the original data
    QUniqueId key = origData->store();
    QVERIFY( key != QUniqueId() );

    // Create the simulated QCop channel and stream the data onto the channel, and 
    // delete the data object
    QByteArray streamData;
    {
        QDataStream stream( &streamData, QIODevice::WriteOnly );
        stream << *origData;
    }
    delete origData;
    origData = 0;


    // Recreate the data object on receive, and check that it's the same data
    {
        QDSData* receivedData = new QDSData();
        {
            QDataStream stream( &streamData, QIODevice::ReadOnly );
            stream >> *receivedData;
        }
        QVERIFY( receivedData->isValid() );
        QVERIFY( receivedData->data() == STRING_TEST_LARGE_DATA );
        QVERIFY( receivedData->type().id() == STRING_TEST_DATA_TYPE );

        // Destroy the received data
        delete receivedData;
        receivedData = 0;
    }

    // Recreate the data object on receive again, and check that it's the same data
    {
        QDSData* receivedData = new QDSData();
        {
            QDataStream stream( &streamData, QIODevice::ReadOnly );
            stream >> *receivedData;
        }
        QVERIFY( receivedData->isValid() );
        QVERIFY( receivedData->data() == STRING_TEST_LARGE_DATA );
        QVERIFY( receivedData->type().id() == STRING_TEST_DATA_TYPE );

        // Destroy the received data
        delete receivedData;
        receivedData = 0;
    }

    // Now check that the original data is still in the store
    QDSData* storedData = new QDSData( key );
    QVERIFY( storedData->isValid() );
    QVERIFY( storedData->data() == STRING_TEST_LARGE_DATA.toLatin1() );
    QVERIFY( storedData->type().id() == STRING_TEST_DATA_TYPE );

    // Remove the data and delete the object
    QVERIFY( storedData->remove() );
    delete storedData;
    storedData = 0;

}
