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

#ifndef QTOPIASYSTEMTEST_P_H
#define QTOPIASYSTEMTEST_P_H

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

#include <qcircularbuffer_p.h>
#include <qtestprotocol_p.h>
#include <qtopiasystemtest.h>
#include <qtopiasystemtestmodem.h>
#include <recordevent_p.h>

class QtopiaSystemTestMail;

class QtopiaSystemTestPrivate : public QObject
{
Q_OBJECT
signals:
    void appGainedFocus(QString const &appName);
    void appBecameIdle(QString const &appName);

public:
    QtopiaSystemTestPrivate(QtopiaSystemTest *parent);
    virtual ~QtopiaSystemTestPrivate();

    virtual bool processMessage( QTestMessage *msg );

    QString realAppName(QString const &appName);

public:
    void startApp( const QString &applicationName, const QString &subMenu, QSystemTest::StartApplicationFlags flags );
    bool ensureEnvironment(QString const&, QString const&);

    QtopiaSystemTest::MemoryType stringToMemoryType(QString const&);

#ifdef QTUITEST_USE_PHONESIM
    QPointer<QtopiaSystemTestModem> test_modem;
    friend class QtopiaSystemTestModem;
    friend class QtopiaSystemTestModemPrivate;
    friend class PhoneTestFactory;
#endif

    friend class QtopiaSystemTestMail;
    QPointer<QtopiaSystemTestMail> test_mail;

    QTestMessage reply_msg;

    QMap<QString, QString> appNameToBinary;

    QList<RecordEvent> recorded_events;
    QString recorded_code;

    int display_id;
    QString device;
    bool mousePreferred;
    QRect screenGeometry;
    QString theme;
    QString config_id;
    bool got_startup_info;

    int sample_memory_interval;

    struct MemorySample {
        QString file;
        int line;
        QVariantMap values;
    };
    QList<MemorySample> memorySamples;

    QString qtopia_script;
    bool direct_log_read;
    QCircularBuffer<QByteArray> log_buffer;

    QtopiaSystemTest *p;
};

#endif
