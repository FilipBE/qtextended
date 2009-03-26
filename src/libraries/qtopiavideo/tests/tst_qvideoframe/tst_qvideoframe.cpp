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

#include <QVideoFrame>
#include <QObject>
#include <QTest>
#include <QSignalSpy>

class tst_QVideoFrame : public QObject
{
Q_OBJECT
public:
    tst_QVideoFrame() {};
    virtual ~tst_QVideoFrame() {};

private slots:
    void initTestCase();
    void newFrame_data();
    void newFrame();
    void bufferHelper();
    void constData();
    void nonConstData();
    void copyFrame();
};

class BufferHelper : public QVideoFrame::BufferHelper
{
public:
    BufferHelper()
        :isLocked(false)
    {};

    virtual void lock() { isLocked = true; }
    virtual void unlock() { isLocked = false; }

    bool isLocked;
};

void tst_QVideoFrame::initTestCase()
{
    qRegisterMetaType<QVideoFrame::PixelFormat>("QVideoFrame::PixelFormat");
}

Q_DECLARE_METATYPE(QVideoFrame::PixelFormat);
void tst_QVideoFrame::newFrame_data()
{
    QTest::addColumn<QVideoFrame::PixelFormat>("pixelFormat");
    QTest::addColumn<int>("colorDepth0");
    QTest::addColumn<int>("colorDepth1");
    QTest::addColumn<QSize>("planeSize0");
    QTest::addColumn<QSize>("planeSize1");

    QTest::newRow("RGB32") << QVideoFrame::Format_RGB32 << 32 << 0 << QSize(10,10) << QSize(0,0);
    QTest::newRow("YUV420P") << QVideoFrame::Format_YUV420P << 8 << 8 << QSize(10,10) << QSize(5,5);
}

void tst_QVideoFrame::newFrame()
{
    QFETCH( QVideoFrame::PixelFormat, pixelFormat );
    QFETCH( int, colorDepth0 );
    QFETCH( int, colorDepth1 );
    QFETCH( QSize, planeSize0 );
    QFETCH( QSize, planeSize1 );

    QVideoFrame frame( pixelFormat, QSize(10,10) );

    QVERIFY( frame.planeData(0) != 0 );
    if ( colorDepth1 ) {
        QVERIFY( frame.planeData(1) != 0 );
        QVERIFY( frame.planeData(2) != 0 );
    } else {
        QVERIFY( frame.planeData(1) == 0 );
        QVERIFY( frame.planeData(2) == 0 );
    }

    QCOMPARE( QVideoFrame::colorDepth( frame.format(), 0), colorDepth0 );
    QCOMPARE( QVideoFrame::colorDepth( frame.format(), 1), colorDepth1 );
    QCOMPARE( frame.planeSize(0), planeSize0 );
    QCOMPARE( frame.planeSize(1), planeSize1 );
}

void tst_QVideoFrame::bufferHelper()
{
    BufferHelper helper;
    uchar data[1024];

    QCOMPARE( helper.isLocked, false );
    QVideoFrame frame( QVideoFrame::Format_RGB32, QSize(10,10), data, &helper );
    QCOMPARE( helper.isLocked, true );
    frame = QVideoFrame();
    QCOMPARE( helper.isLocked, false );


    frame = QVideoFrame( QVideoFrame::Format_RGB32, QSize(10,10), data, &helper );
    const uchar* frameData = frame.planeData(0);
    QCOMPARE( helper.isLocked, true );
    frame.planeData(0)[0] = 1;
    QCOMPARE( helper.isLocked, true );
    QCOMPARE( frameData, data );

    frame = QVideoFrame();
    QCOMPARE( helper.isLocked, false );

}


void tst_QVideoFrame::constData()
{
    BufferHelper helper;
    uchar data[1024];

    QVideoFrame frame( QVideoFrame::Format_RGB32, QSize(10,10), (const uchar*)data, &helper );
    frame.constPlaneData(0);
    QCOMPARE( helper.isLocked, true );
    frame.planeData(0)[0] = 1;
    QCOMPARE( helper.isLocked, false );
    QVERIFY( frame.constPlaneData(0) != data );
}

void tst_QVideoFrame::nonConstData()
{
    BufferHelper helper;
    uchar data[1024];

    QVideoFrame frame( QVideoFrame::Format_RGB32, QSize(10,10), data, &helper );
    frame.constPlaneData(0);
    QCOMPARE( helper.isLocked, true );
    frame.planeData(0)[0] = 1;
    QCOMPARE( helper.isLocked, true );
    QVERIFY( frame.constPlaneData(0) == data );
}


void tst_QVideoFrame::copyFrame()
{
    uchar data[1024];

    QVideoFrame frame( QVideoFrame::Format_RGB32, QSize(10,10), data );
    frame.planeData(0)[0] = 1;

    QVideoFrame frame2 = frame;
    const uchar* frame2Data = frame2.constPlaneData(0);
    QCOMPARE( frame2Data, data );

    frame2.planeData(0)[0] = 2;
    QCOMPARE( frame.planeData(0)[0], uchar(1) );
    QCOMPARE( frame2.planeData(0)[0], uchar(2) );

    QVERIFY( frame2.planeData(0) != data );
}

QTEST_MAIN(tst_QVideoFrame)

#include "tst_qvideoframe.moc"
