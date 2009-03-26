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
#include <QValueSpaceItem>
#include <QStringList>

#include "systeminfo.h"
#include <QSet>
#include <QDebug>
#include <QtopiaIpcEnvelope>
#include <QTimer>
#include <QTimeString>
#include <QtopiaServiceDescription>

#include <QWebPage>
#include <QWebFrame>
#include <QApplication>
#include <QDebug>

#include <QLocale>
#include <QStorageMetaInfo>
#include <QFileSystem>
#include <QPhoneProfileManager>
#include <QPhoneProfile>

S60SystemInfo::S60SystemInfo(QWebPage *p)
    : m_page(p), m_frame(p->mainFrame())
{
    m_profileManager = new QPhoneProfileManager(this);

    m_vsi = new QValueSpaceItem(this);
    m_chargevsi = new QValueSpaceItem("/Hardware/Accessories/QPowerSource/DefaultBattery/Charge", this);
    m_chargingvsi = new QValueSpaceItem("/Hardware/Accessories/QPowerSource/DefaultBattery/Charging", this);
    connect(m_chargevsi, SIGNAL(contentsChanged()), this, SLOT(chargeChanged()));
    connect(m_chargingvsi, SIGNAL(contentsChanged()), this, SLOT(chargingChanged()));
}

int S60SystemInfo::chargeLevel()
{
    return m_chargevsi->value().toInt();
}

bool S60SystemInfo::chargerConnected()
{
    return m_chargingvsi->value().toBool();
}

QString S60SystemInfo::onChargeLevel() const
{
    return m_oncharge;
}

QString S60SystemInfo::onChargerConnected() const
{
    return m_oncharging;
}

void S60SystemInfo::setOnChargeLevel(const QString& script)
{
    m_oncharge = script;
}

void S60SystemInfo::setOnChargerConnected(const QString& script)
{
    m_oncharging = script;
}

void S60SystemInfo::chargeChanged()
{
    if (!m_oncharge.isEmpty()) {
        // Weird callback scheme
        m_frame->evaluateJavaScript(m_oncharge);
    }
}

void S60SystemInfo::chargingChanged()
{
    if (!m_oncharging.isEmpty()) {
        m_frame->evaluateJavaScript(m_oncharging);
    }
}

int S60SystemInfo::signalBars() const
{
    return m_vsi->value("/Hardware/Accessories/QSignalSource/DefaultSignal").toInt();
}

int S60SystemInfo::networkRegistrationStatus() const
{
    int ret = 0;
    // 0 = unknown
    // 1 = unregistered, no networks
    // 2 = unregistered, not searching, emergency calls only
    // 3 = unregistered, searching
    // 4 = registered, network busy [unsupported?]
    // 5 = registered on home network
    // 6 = registration denied
    // 7 = registered, roaming

    if (m_vsi->value("/Telephony/Status/ModemStatus").toString() == "AerialOff")
        ret = 1;
    else {
        QString state = m_vsi->value("/Telephony/Status/RegistrationState").toString();

        if (state == "Home")
            ret = 5;
        else if (state == "Roaming")
            ret = 7;
        else if (state == "Denied")
            ret = 6;
        else if (state == "None")
            ret = 2;
        else if (state == "Searching")
            ret = 3;
    }

    return ret;
}

QString S60SystemInfo::networkName() const
{
    return m_vsi->value("/Telephony/Status/OperatorName").toString();
}

// Vibration information and control services
void S60SystemInfo::startvibra(int duration, int intensity) const
{
    // XXX This really needs a server service to manage
    Q_UNUSED(duration);
    Q_UNUSED(intensity);
}

void S60SystemInfo::stopvibra() const
{
    // XXX This really needs a server service to manage
}

int S60SystemInfo::vibraSettings() const
{
    QPhoneProfile p = m_profileManager->activeProfile();

    if (p.vibrate())
        return 1;
    else
        return 2;
}

int S60SystemInfo::totalRam() const
{
    // Returns RAM size in bytes
    return 128*1024*1024; // XXX return the real ram size
}

int S60SystemInfo::freeRam() const
{
    // Returns free RAM size in bytes
    return 64*1023*1027; // XXX return the real ram size
}

// Memory and file system information
quint64 S60SystemInfo::drivesize(const QString& drive) const
{
    QFileSystem fs = QFileSystem::fromFileName(drive);

    return fs.totalBlocks() * fs.blockSize();
}

quint64 S60SystemInfo::drivefree(const QString& drive) const
{
    QFileSystem fs = QFileSystem::fromFileName(drive);

    return fs.availBlocks() * fs.blockSize();
}


QString S60SystemInfo::driveList() const
{
    QList<QFileSystem*> list = QStorageMetaInfo::instance()->fileSystems(0);

    QStringList names;

    foreach (QFileSystem* fs, list) {
        // Skip the 'system' drive
        if (fs->applicationsPath() != QFileSystem::applicationsFileSystem().applicationsPath()) {
            if (!names.contains(fs->path()))
                names.append(fs->path());
        }
    }

    return names.join(" ");
}

QString S60SystemInfo::language() const
{
    QLocale l;
    QString s = l.name();
    int idx = s.lastIndexOf('_');
    if (idx > 0)
        s.truncate(idx);
    return s;
}

void S60SystemInfo::lighton(int target, int duration, int intensity, bool fade) const
{
    // XXX This really needs a server service to manage
    Q_UNUSED(target);
    Q_UNUSED(duration);
    Q_UNUSED(intensity);
    Q_UNUSED(fade);
}

void S60SystemInfo::lightoff(int target, int duration, bool fade) const
{
    // XXX This really needs a server service to manage
    Q_UNUSED(target);
    Q_UNUSED(duration);
    Q_UNUSED(fade);
}

void S60SystemInfo::lightblink(int target, int duration, int onDuration, int offDuration, int intensity) const
{
    // XXX This really needs a server service to manage
    Q_UNUSED(target);
    Q_UNUSED(duration);
    Q_UNUSED(onDuration);
    Q_UNUSED(offDuration);
    Q_UNUSED(intensity);
}

// Beep tone control services
void S60SystemInfo::beep(int frequency, int duration) const
{
    // XXX This really needs a server service to manage
    Q_UNUSED(frequency);
    Q_UNUSED(duration);
}


#include "systeminfo.moc"
