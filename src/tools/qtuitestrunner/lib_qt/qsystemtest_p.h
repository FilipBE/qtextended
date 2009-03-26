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

#ifndef QSYSTEMTEST_P_H
#define QSYSTEMTEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include "qtestremote_p.h"
#include "qtestprotocol_p.h"
#include "recordevent_p.h"
#include <QStringList>
#include <qsystemtest.h>

class QProcess;
class QSystemTestMail;
class QSystemTestMaster;
class QTextEdit;

class QSystemTestPrivate : public QObject
{
Q_OBJECT
public slots:
    void recordPrompt();

    bool recentEventRecorded();
    void resetEventTimer();

signals:
    void appGainedFocus(QString const &appName);
    void appBecameIdle(QString const &appName);

public:
    QSystemTestPrivate(QSystemTest *parent);
    virtual ~QSystemTestPrivate();

    virtual bool processMessage( QTestMessage *msg );

    QString realAppName(QString const &appName);

    bool recordEvents( const QString &manualSteps = QString() );

public slots:
    QTestMessage query( const QTestMessage &message, const QString &queryPath = QString(), int timeout = DEFAULT_QUERY_TIMEOUT );
    bool queryPassed( const QStringList &passResult, const QStringList &failResult,
                        const QTestMessage &message, const QString &queryPath = QString(), QTestMessage *reply = 0, int timeout = DEFAULT_QUERY_TIMEOUT );
    bool queryPassed( const QString &passResult, const QString &failResult,
                        const QTestMessage &message, const QString &queryPath = QString(), QTestMessage *reply = 0, int timeout = DEFAULT_QUERY_TIMEOUT );

private slots:
    void showUnexpectedDialog();

public:
    void modifyStateKey( bool pressed, const QString &key, const QString &appName );
    void recordedEvent( QTestMessage *msg );
    void recordEvents( QList<RecordEvent> const& );
    void parseKeyEventToCode( QTestMessage *msg);

    QVariant getSetting(const QString&,const QString&,const QString&,const QString&,const QString&);
    void     setSetting(const QString&,const QString&,const QString&,const QString&,const QString&,const QVariant&);

    bool imagesAreEqual( const QImage &actual, const QImage &expected, bool strict = false );
    bool learnImage( const QImage &actual, const QImage &expected, const QString &comment = QString());

    void onMessageBox( QTestMessage const *message );
    void onDialog( QTestMessage const *message );
    void resetUnexpectedDialog( const QString &title );

    bool ensureEnvironment(QString const&, QString const&);

    QTime *event_timer;
    QList<QPointer<QProcess> > aut;
    QTime key_hold_started;
    QSystemTestMaster *test_app;
    QTestMessage error_msg;
    QTestMessage error_msg_sent;
    QTestMessage last_msg_sent;

    QTimer *key_enter_timer;

    QPointer<QTextEdit> recorded_events_edit;

    bool recorded_events_as_code;
    bool record_prompt;
    bool query_failed;
    bool query_warning_mode;
    int fatal_timeouts;
    int timeouts;

    Qt::Key keyclickhold_key;
    QString keyclickhold_path;
    QString current_application;

    QString loc_fname;
    int loc_line;
    class ExpectedMessageBox {
    public:
        QString test_function;
        QString data_tag;
        QString title;
        QString text;
        QString option;
    };
    QList<ExpectedMessageBox*> expected_msg_boxes;
    bool ignore_msg_boxes;

    bool auto_mode;

    // the following parameters are used to start an Application_Under_Test
    QString aut_host;
    quint16 aut_port;
    bool keep_aut;
    bool silent_aut;
    bool no_aut;
    bool demo_mode;
    bool verbose_perf;
    bool verbose;

    QStringList env;

    QTestRemote qtest_ide;

    QMap<QString, QString> appNameToBinary;

    QMap<QString, int> filteredMessages;

    static QSystemTest* singleton;
    bool wait_message_shown;
    bool strict_mode;
    int visible_response_time;

    QList<RecordEvent> recorded_events;
    QString recorded_code;

    int display_id;
    QString device;
    bool mousePreferred;
    QRect screenGeometry;
    QString theme;
    QString config_id;
    bool got_startup_info;
    bool recording_events;

    QString unexpected_dialog_title;
    bool ignore_unexpected_dialogs;
    bool expect_app_close;

    int sample_memory_interval;

    QSystemTest *p;
};

#endif
