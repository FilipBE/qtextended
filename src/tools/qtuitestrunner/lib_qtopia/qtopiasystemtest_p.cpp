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

#include <qtopiasystemtest.h>
#include <qtopiasystemtestmodem.h>

#include "qtestverifydlg_p.h"
#include "qtestremote_p.h"
#include "recordevent_p.h"

#include <QProcess>
#include <QTextEdit>
#include <QMessageBox>

#include <QMetaEnum>

#include "qtopiasystemtest_p.h"

#undef qLog
#define qLog(A) if (!p->verbose()); else (QDebug(QtDebugMsg) << #A ":")

#define OBJECT_EXIST_TIMEOUT 1000

#define BT(message) (\
    message["location"] = QString("%1:%2%3").arg(__FILE__).arg(__LINE__).arg(!message["location"].toString().isEmpty() ? "\n" + message["location"].toString() : ""),\
    message)

QtopiaSystemTestPrivate::QtopiaSystemTestPrivate(QtopiaSystemTest *parent)
    : QObject(parent)
#ifdef QTUITEST_USE_PHONESIM
    , test_modem(0)
#endif
    , display_id(0)
    , device()
    , mousePreferred(false)
    , screenGeometry()
    , theme()
    , config_id()
    , got_startup_info(false)
    , sample_memory_interval(-1)
    , direct_log_read(false)
    , log_buffer(5000)
    , p(parent)
{}

QtopiaSystemTestPrivate::~QtopiaSystemTestPrivate()
{
#ifdef QTUITEST_USE_PHONESIM
    delete test_modem;
#endif
}

void QtopiaSystemTestPrivate::startApp( const QString &applicationName, const QString &subMenu, QSystemTest::StartApplicationFlags flags )
{
    if (flags & QSystemTest::BackgroundCurrentApplication) {
        p->backgroundAndGotoHome();
    } else {
        p->gotoHome();
    }
    if (p->queryFailed()) return;

    QString real_app(realAppName(applicationName));
    QTestMessage message("startApp");
    message["app"] = real_app;
    message["sub"] = subMenu;

    p->doQuery( BT(message), "qpe:" );
    if (p->queryFailed()) return;

    if (flags & QSystemTest::WaitForFocus) {
        p->waitForCurrentApplication(real_app);
        if (p->queryFailed()) return;
    }
}

/*!
    \internal
    Processes incoming messages from the system under test (such as Qtopia).
*/
bool QtopiaSystemTestPrivate::processMessage( QTestMessage *msg )
{
    if (msg->event() == "valueSpaceResponse") {
        reply_msg = *msg;
        return true;
    } else if (msg->event() == "appBecameIdle") {
        emit appBecameIdle((*msg)["appName"].toString());
        return true;
    } else if (msg->event() == "startupInfo") {
        display_id = (*msg)["display_id"].toInt();
        device = (*msg)["device"].toString();
        mousePreferred = (*msg)["mousePreferred"].toBool();
        screenGeometry = (*msg)["screenGeometry"].value<QRect>();
        theme = (*msg)["theme"].toString();

        config_id = QString("%1_%2_%3x%4")
            .arg( mousePreferred ? "touchscreen" : "keypad" )
            .arg( theme )
            .arg( screenGeometry.width() )
            .arg( screenGeometry.height() );

        p->setConfigurationIdentifier(config_id);
        got_startup_info = true;
        return true;
    } else if (msg->event() == "sampledMemory") {
        MemorySample sample;
        sample.file = p->currentFile();
        sample.line = p->currentLine();
        sample.values = (*msg)["values"].toMap();
        memorySamples << sample;
        return true;
    }

    return false;
}

/* Translates a /proc/meminfo label to a MemoryType enum */
QtopiaSystemTest::MemoryType QtopiaSystemTestPrivate::stringToMemoryType(QString const& typeString)
{
    if (typeString == "MemTotal") return QtopiaSystemTest::TotalMemory;
    if (typeString == "MemFree")  return QtopiaSystemTest::FreeMemory;
    if (typeString == "Buffers")  return QtopiaSystemTest::BuffersMemory;
    if (typeString == "Cached")   return QtopiaSystemTest::CacheMemory;
    if (typeString == "Active")   return QtopiaSystemTest::ActiveMemory;
    if (typeString == "Inactive") return QtopiaSystemTest::InactiveMemory;
    return QtopiaSystemTest::NoMemoryType;
}

/*!
    \internal
*/
QString QtopiaSystemTestPrivate::realAppName(QString const &appName)
{
    if (appName == "qpe")
        return "qpe";

    if (appNameToBinary.isEmpty()) {
        QTestMessage msg("getAppNames");
        QTestMessage reply;
        if (p->doQuery(msg, "qpe:", &reply)) {
            foreach(QString s, reply["getAppNames"].toMap().keys()) {
                appNameToBinary[s] = reply["getAppNames"].toMap()[s].toString();
            }
        }
    }

    return appNameToBinary.contains(appName) ? appNameToBinary[appName] : appName.toLower();
}

/*
    On the desktop ONLY, ensures that the environment variable \a key is set to \a value.
    Returns true if a change was made, indicating that Qt Extended must be restarted before
    the change will take effect.
*/
bool QtopiaSystemTestPrivate::ensureEnvironment(QString const& key, QString const& value)
{
    Q_ASSERT(!p->runsOnDevice());

    QFile f(QString("/tmp/qtopia-%1/qtuitest_cmds").arg(display_id));
    if (!f.open(QIODevice::ReadWrite)) {
        p->setQueryError("Couldn't open " + f.fileName());
        return false;
    }

    QStringList in_lines = QString(f.readAll()).split('\n');
    f.close();
    QByteArray out_data;

    bool changed = false;
    bool handled = false;

    foreach (QString line, in_lines) {
        if (line.startsWith("export " + key + "=")) {
            handled = true;
            if (!value.isNull()) {
                QString out = QString("export %1=\"%2\"").arg(key).arg(value);
                out_data.append(out + "\n");
                changed = (out != line);
            } else {
                changed = true;
            }
        } else if (!line.isEmpty()) {
            out_data.append(line + "\n");
        }
    }
    if (!handled && !value.isNull()) {
        QString out = QString("export %1=\"%2\"").arg(key).arg(value);
        out_data.append(out + "\n");
        changed = true;
    }

    if (!f.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        p->setQueryError("Couldn't open " + f.fileName());
        return false;
    }

    f.write(out_data);

    if (changed && (p->getenv("QTOPIA_SYSTEMTESTHELPER") != "1")) {
        p->setQueryError("When not running on a device, Qtopia needs to be run with '-runmode systemtesthelper' to use this feature!");
        return false;
    }
    return changed;
}

