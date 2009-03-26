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

#ifndef QTOPIASYSTEMTEST_H
#define QTOPIASYSTEMTEST_H

#if ! defined Q_QDOC

#include <qsystemtest.h>

class QtopiaSystemTestModem;
class QtopiaSystemTestPrivate;

#ifdef Q_QDOC
namespace QtopiaSystemTest
#else
class QTUITEST_EXPORT QtopiaSystemTest : public QSystemTest
#endif
{
Q_OBJECT
Q_ENUMS(VersionType)
Q_ENUMS(MemoryType)
Q_FLAGS(MemoryTypes)
Q_ENUMS(SampledMemoryInfoFlag)
Q_FLAGS(SampledMemoryInfoFlags)
public:
    QtopiaSystemTest();
    virtual ~QtopiaSystemTest();

    enum VersionType
    {
        QtVersion                    = 0x00,
        QtExtendedVersion            = 0x01,
        KernelVersion                = 0x02
    };

    enum MemoryType
    {
        NoMemoryType                 = 0x00,
        TotalMemory                  = 0x01,
        FreeMemory                   = 0x02,
        BuffersMemory                = 0x04,
        CacheMemory                  = 0x08,
        ActiveMemory                 = 0x10,
        InactiveMemory               = 0x20,
        EffectiveFreeMemory          = FreeMemory|CacheMemory|BuffersMemory
    };
    Q_DECLARE_FLAGS(MemoryTypes, MemoryType)

    enum SampledMemoryInfoFlag
    {
        NoSampledMemoryInfoFlags = 0x00000000,

        SampleMinimum            = 0x00000001,
        SampleMaximum            = 0x00000002,
        SampleMean               = 0x00000004,

        SampleFile               = 0x00010000,
        SampleLine               = 0x00020000
    };
    Q_DECLARE_FLAGS(SampledMemoryInfoFlags, SampledMemoryInfoFlag)

public slots:
    bool runsOnDevice();

    // widget shortcuts and accessors
    QString launcherMenu();
    QString optionsMenu();
    QString softMenu();
    QString callAccept();
    QString callHangup();

    void setPerformanceTest(bool,bool = false);

    int getMemoryInfo(MemoryTypes);
    void startSamplingMemory(int = 500);
    void stopSamplingMemory();
    QVariant getSampledMemoryInfo(MemoryTypes,SampledMemoryInfoFlags);

    // application management functions
    virtual void startApplication( const QString &application, int timeout = 30000, QSystemTest::StartApplicationFlags flags = QSystemTest::WaitForFocus);
    void gotoHome();
    void backgroundAndGotoHome();
    void restartQtopia();
    void shutdownQtopia();
    void reboot();
    void ipcSend(const QString&,const QString&,
                    const QVariant& = QVariant(), const QVariant& = QVariant(), const QVariant& = QVariant(),
                    const QVariant& = QVariant(), const QVariant& = QVariant(), const QVariant& = QVariant(),
                    const QVariant& = QVariant(), const QVariant& = QVariant(), const QVariant& = QVariant(),
                    const QVariant& = QVariant() );

    // verification mechanisms
    void waitForQtopiaStart(int = 60000);
    void waitForCurrentApplication(QString const&, int = 10000);

    // environment manipulation functions
    QString documentsPath();
    QString getVersion(VersionType=QtExtendedVersion);
    QVariant getValueSpace(const QString&);
    bool waitForValueSpace(const QString&,const QVariant&,int);

    // helper classes
    QtopiaSystemTestModem *testModem();

    void startDirectLogRead();

protected:
    virtual void printUsage(int,char*[]) const;
    virtual void processCommandLine(int&,char*[]);

    virtual void processMessage(const QTestMessage&);

    virtual void applicationStandardOutput(QList<QByteArray> const&);
    virtual void applicationStandardError(QList<QByteArray> const&);

    bool launchQtopia();

    virtual void selectMessageBoxOption(QString const&,QString const&);

private:
    QtopiaSystemTestPrivate *d;
    friend class QtopiaSystemTestPrivate;
    friend class QtopiaSystemTestModemPrivate;
    friend class QtopiaSystemTestModem;
};

#endif

#endif
