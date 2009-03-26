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

#ifndef QSYSTEMTEST_H
#define QSYSTEMTEST_H

#if ! defined Q_QDOC

#include <qabstracttest.h>
#include <qtestprotocol_p.h>

#include <QDir>

#define DEFAULT_QUERY_TIMEOUT 120000

class QSystemTestPrivate;
class QTestMessage;

#ifdef Q_QDOC
namespace QSystemTest
#else
class QTUITEST_EXPORT QSystemTest : public QAbstractTest
#endif
{
Q_OBJECT
Q_ENUMS(SkipMode)
Q_ENUMS(EnterMode)
Q_ENUMS(TimeSynchronizationMode)
Q_ENUMS(StartApplicationFlag)
Q_FLAGS(StartApplicationFlags)
public:
    QSystemTest();
    virtual ~QSystemTest();

    enum SkipMode
    {
        SkipSingle                   = 0x01,
        SkipAll                      = 0x02
    };

    enum EnterMode
    {
        Commit                       = 0x00,
        NoCommit                     = 0x01
    };

    enum TimeSynchronizationMode
    {
        HostTimeSynchronization      = 0x00,
        ManualTimeSynchronization    = 0x01,
        AutoTimeSynchronization      = 0x02
    };

    enum StartApplicationFlag
    {
        NoFlag                       = 0x00,
        WaitForFocus                 = 0x01,
        BackgroundCurrentApplication = 0x02
    };
    Q_DECLARE_FLAGS(StartApplicationFlags, StartApplicationFlag)

public slots:
    // commands to control text execution
    void strict( bool = true );
    void setDemoMode( bool );
    static QString userName();
    bool shouldFilterMessage(char const*);
    QMap<QString, int> filteredMessages() const;
    void clearFilteredMessages();
    bool runsOnDevice();

    virtual void skip(const QString&,SkipMode);
    virtual void expectFail(const QString&);

    // commands to handle 'unexpected' messageboxes
    void addExpectedMessageBox(const QString&,const QString&,const QString& = QString());
    void clearExpectedMessageBoxes();
    void clearExpectedMessageBox(const QString&,const QString&);
    bool waitExpectedMessageBox(uint,bool = true,const QString& title = QString(),const QString& text = QString() );
    void ignoreMessageBoxes(bool);

    // widget shortcuts and accessors
    QString optionsMenu();
    QString tabBar();
    QString progressBar();
    QString signature(const QString&, int = 0);

    // widget checks
    bool isVisible(const QString&);
    bool isEnabled(const QString&);

    // checkBox manipulators
    bool isChecked(const QString&);
    void setChecked(bool,const QString&);

    // text verification functions
    QString currentApplication();
    QString currentTitle(const QString& = QString());
    QString focusWidget(const QString& = QString());
    QString getSelectedText(const QString& = QString() );
    QString getText(const QString& = QString() );
    QStringList getList(const QString& = QString() );
    QStringList getLabels(const QString& = QString() );

    // clipboard
    QString getClipboardText();
    void setClipboardText(const QString&);

    // time management functions
    QDateTime getDateTime();
    void setDateTime(const QDateTime&,const QString& = QString());
    void synchronizeDateTime();
    int setTimeSynchronization(TimeSynchronizationMode);
    void setDateFormat(const QString& = "D/M/Y");
    QString dateFormat();
    void set12HourTimeFormat(bool = true);
    QString timeFormat();
    void setTimeZone(const QString&);
    QString timeZone();

    int visibleResponseTime();
    void setVisibleResponseTime(int);

    // application management functions
    virtual void startApplication(const QString&,int timeout = 5000,QSystemTest::StartApplicationFlags = QSystemTest::WaitForFocus);
    void expectApplicationClose(bool);

    // low level simulators
    void keyClick(Qt::Key,const QString& = QString());
    void keyClickHold(Qt::Key,int,const QString& = QString());
    void keyPress(Qt::Key,const QString& = QString());
    void keyRelease(Qt::Key,const QString& = QString());

    void mouseClick(int,int);
    void mouseClick(const QString&);
    void mouseClickHold(int,int,int);
    void mouseClickHold(const QString&,int);
    void mousePress(int,int);
    void mousePress(const QString&);
    void mouseRelease(int,int);
    void mouseRelease(const QString&);

    bool mousePreferred();

    // text input and selection functions
    void enter(const QString&,const QString& = QString(),EnterMode = Commit);
    void enter(const QDate&,const QString& = QString(),EnterMode = Commit);
    void enter(const QTime&,const QString& = QString(),EnterMode = Commit);

    void select(const QString&,const QString& = QString());

    // verification mechanisms
    void waitForTitle(const QString&,int = 0 );
    void verifyImage(const QString&,const QString& = QString(),
                        const QString& = QString(),const QStringList& = QStringList());
    bool compareImage( const QString&, const QString&, const QStringList& = QStringList());
    void prompt(const QString& = QString());
    bool recordEvents(const QString&, bool = true);
    QString stopRecordingEvents();

    // testcase synchronisation
    void wait(int);

    // graphical manipulation
    void saveScreen(const QString&, const QString& = "qpe:");
    QImage grabImage(const QString& = QString(),const QStringList& = QStringList());

    QString runProcess(const QString&,const QStringList&,const QString&);
    QString getenv(const QString&);

    // environment manipulation functions
    bool isBuildOption(const QString&);
    void putFile(const QString&,const QString&,QFile::Permissions=0);
    void putData(const QByteArray&,const QString&,QFile::Permissions=0);
    void getFile(const QString&,const QString&);
    QString getData(const QString&);
    void deletePath(const QString&);
    QStringList getDirectoryEntries(const QString&,QDir::Filters = QDir::NoFilter);

    QSize getImageSize(const QString&);
    QRect getGeometry(const QString& = QString());

    void setProperty(const QString&,const QString&,const QVariant&);
    QVariant getProperty(const QString&,const QString&);

    bool invokeMethod(const QString&,const QString&,
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant());

    bool invokeMethod(const QString&,const QString&,Qt::ConnectionType,
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant(),const QVariant& = QVariant(),const QVariant& = QVariant(),
                        const QVariant& = QVariant());

    QVariant getSetting(const QString&,const QString&,const QString&);
    QVariant getSetting(const QString&,const QString&,const QString&,const QString&);
    void setSetting(const QString&,const QString&,const QString&,const QVariant&);
    void setSetting(const QString&,const QString&,const QString&,const QString&,const QVariant&);

    // debug helper functions
    QString activeWidgetInfo();

    void print(QVariant const&);

    static QSystemTest* lastInstance();

protected:
    virtual void printUsage(int,char*[]) const;
    virtual void processCommandLine(int&,char*[]);
    virtual int  runTest(int,char*[]);

    virtual void applicationStandardOutput(QList<QByteArray> const&);
    virtual void applicationStandardError(QList<QByteArray> const&);

    bool connectToAut(int timeout = 10000);
    void disconnectFromAut();
    bool isConnected() const;
    bool demoMode() const;
    QString autHost() const;
    int autPort() const;

    virtual void selectMessageBoxOption(QString const&, QString const&);

    void resetCurrentApplication();

    void setIgnoreUnexpectedDialogs(bool);
    bool ignoreUnexpectedDialogs() const;
    void clearUnexpectedDialog();

    virtual QString currentFile();
    virtual int currentLine();

    QString configurationIdentifier() const;
    void setConfigurationIdentifier(QString const&);

    bool verbose() const;


#ifndef Q_QDOC
    virtual bool fail(QString const&);
    bool doQuery(const QTestMessage& message, const QString& queryPath = QString(), QTestMessage* reply = 0, int timeout = DEFAULT_QUERY_TIMEOUT, const QStringList& pass = QStringList("OK"), const QStringList& fail = QStringList());
    inline bool doQuery(const QTestMessage& message, const QString& queryPath, QTestMessage* reply, int timeout, const QString& pass, const QString& fail = QString())
    { return doQuery(message,queryPath,reply,timeout,QStringList(pass),QStringList(fail)); }

    template <typename T> inline
    T queryWithReturn(T const& ret, QString const& msg, QString const& queryPath)
    {
        QTestMessage reply;
        QTestMessage testMessage(msg);
        if (!doQuery(testMessage, queryPath, &reply)) return ret;
        if (!reply[msg].isValid()) {
            reply["status"] = "ERROR: no data in reply to " + msg + "; status: " + reply["status"].toString();
            setQueryError(reply);
            return ret;
        }
        return reply[msg].value<T>();
    }
#endif

    virtual bool setQueryError(const QTestMessage&);
    virtual bool setQueryError(const QString&);

    virtual bool queryFailed(QTestMessage* = 0,QTestMessage* = 0);
    virtual void enableQueryWarnings(bool,const QString& = QString(),int = -1);
    virtual void setLocation(const QString& = QString(),int = -1);

    virtual void processMessage(const QTestMessage&);

protected slots:
    void abortPrompt();
    void abortTest();

protected:
    bool abort_prompt;

private:
    QSystemTestPrivate *d;
    friend class QSystemTestMaster;
    friend class QSystemTestPrivate;
    friend class QSystemTestMail;
    friend class TestProcess;

#ifdef Q_QDOC
/* Functions where implementation is in QtScript */
public:
    void waitFor(Number = 10000,Number = 20,String = null);
    void expectMessageBox(String,String,String,Number);
    void compare(Variant,Variant);
    void verify(Boolean,String = null);
    void fail(String);
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSystemTest::StartApplicationFlags)

#ifdef Q_QDOC
typedef Nothing String;
typedef Nothing StringArray;
typedef Nothing QVariantArray;
typedef Nothing Boolean;
typedef Nothing Number;
typedef Nothing Array;
typedef Nothing Function;
#endif

#endif

#endif
