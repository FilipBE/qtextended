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

//QTEST_SKIP_TEST_DOC

#include "qtopiabluetoothunittest.h"

#ifdef QT_QWS_GREENPHONE

DBusLauncher::DBusLauncher()
 : m_pid(-1)
{
    QByteArray envAddress("DBUS_SESSION_BUS_ADDRESS");
    QByteArray envPid("DBUS_SESSION_BUS_PID");
    QRegExp reAddress("^" + envAddress + "=[\"']?([^\"']+)[\"']?;$");
    QRegExp rePid("^" + envPid + "=([0-9]+);$");

    if (!qgetenv(envAddress).isEmpty()) return;

    QProcess p;
    p.start("/usr/bin/dbus-launch", QStringList("--sh-syntax"), QIODevice::ReadOnly);
    if (!p.waitForStarted(5000) || !p.waitForFinished(5000))
        qFatal("Couldn't start dbus session bus for test.");

    bool success = false;
    QStringList lines;
    while (p.canReadLine()) {
        QByteArray line(p.readLine().trimmed());
        lines << line;
        if (-1 != reAddress.indexIn(line)) {
            setenv(envAddress, qPrintable(reAddress.cap(1)), 1);
            success = true;
        } else if (-1 != rePid.indexIn(line)) {
            m_pid = rePid.cap(1).toInt();
        }
    }

    if (!success)
        qFatal(qPrintable(QString("dbus-launch didn't give expected output:\n%1").arg(lines.join("\n"))));
}

DBusLauncher::~DBusLauncher()
{
    if (-1 == m_pid) return;
    kill(m_pid, SIGTERM);
}

DBusLauncher QtopiaBluetoothUnitTest::launcher;

#endif

QtopiaBluetoothUnitTest::QtopiaBluetoothUnitTest() {
    QDBusConnection dbc = QDBusConnection::sessionBus();
    if (!dbc.isConnected()) {
        qFatal(qPrintable(QString("Unable to connect to DBUS; this test requires DBUS to be running.\n"
                                    "Last DBUS error: %1 %2")
                            .arg(dbc.lastError().name())
                            .arg(dbc.lastError().message()))
        );
    }

    QDBusInterface iface("org.bluez", "/org/bluez",
                            "org.bluez.Manager", dbc);

    if (iface.isValid()) {
        qFatal("org.bluez is already connected to this bus - it shouldn't be!");
    }

    if (!dbc.registerService("org.bluez")) {
        qFatal("Couldn't register org.bluez DBUS service!");
    }

    bluez = new BluezStub(this);
    bluezAdapter = new BluezAdapterStub(this);
    if ( !dbc.registerObject("/org/bluez", this) ) {
        qFatal("Couldn't register /org/bluez object.");
    }
    bluez->log = false;
    bluezAdapter->log = false;
}

