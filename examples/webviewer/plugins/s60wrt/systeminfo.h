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

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QWidget>
#include <QValueSpaceItem>
#include <QHash>

#include <QVariant>
#include <QDebug>

class QWebPage;
class QWebFrame;
class QPhoneProfileManager;

// XXX Has to be a qwidget to work
class S60SystemInfo : public QWidget
{
    Q_OBJECT;
public:
    S60SystemInfo(QWebPage* host);

    // XXX Hide deleteLater and destroyed


    // Power information services
    Q_PROPERTY(int chargelevel READ chargeLevel);
    Q_PROPERTY(bool chargerconnected READ chargerConnected);
    Q_PROPERTY(QString onchargelevel READ onChargeLevel WRITE setOnChargeLevel);
    Q_PROPERTY(QString onchargerconnected READ onChargerConnected WRITE setOnChargerConnected);

    int chargeLevel(); // 0-100
    bool chargerConnected();
    QString onChargeLevel() const;
    QString onChargerConnected() const;
    void setOnChargeLevel(const QString& script);
    void setOnChargerConnected(const QString& script);

    // Network information services
    Q_PROPERTY(int signalbars READ signalBars);
    Q_PROPERTY(QString networkname READ networkName);
    Q_PROPERTY(int networkregistrationstatus READ networkRegistrationStatus);

    int signalBars() const; // 0-7
    int networkRegistrationStatus() const;  // an enum, actually
    QString networkName() const;

    // Display and keypad illumination
    Q_PROPERTY(int lightminintensity READ lightMinIntensity);
    Q_PROPERTY(int lightmaxintensity READ lightMaxIntensity);
    Q_PROPERTY(int lightdefaultintensity READ lightDefaultIntensity);
    Q_PROPERTY(int lightinfiniteduration READ lightInfiniteDuration);
    Q_PROPERTY(int lightmaxduration READ lightMaxDuration);
    Q_PROPERTY(int lightdefaultcycletime READ lightDefaultCycleTime);
    Q_PROPERTY(int lighttargetsystem READ lightTargetSystem);
    Q_PROPERTY(int lighttargetprimarydisplayandkeyboard READ lightTargetPrimaryDisplayAndKeyboard);

    int lightMinIntensity() const {return 0;}
    int lightMaxIntensity() const {return 100;}
    int lightDefaultIntensity() const {return 80;}
    int lightInfiniteDuration() const {return lightMaxDuration() + 1;}
    int lightMaxDuration() const {return 300000;} // 5 minutes
    int lightDefaultCycleTime() const {return -1;}
    int lightTargetSystem() const {return 3;}
    int lightTargetPrimaryDisplayAndKeyboard() const {return 1;}
    Q_INVOKABLE void lighton(int target, int duration, int intensity, bool fade) const;
    Q_INVOKABLE void lightoff(int target, int duration, bool fade) const;
    Q_INVOKABLE void lightblink(int target, int duration, int onDuration, int offDuration, int intensity) const;

    // Vibration information and control services
    Q_PROPERTY(int vibraminintensity READ vibraMinIntensity);
    Q_PROPERTY(int vibramaxintensity READ vibraMaxIntensity);
    Q_PROPERTY(int vibramaxduration READ vibraMaxDuration);
    Q_PROPERTY(int vibrasettings READ vibraSettings);

    int vibraMinIntensity() const {return -100;}
    int vibraMaxIntensity() const {return 100;}
    int vibraMaxDuration() const {return 5000;} // 5 seconds
    int vibraSettings() const;  // returns an enum
    Q_INVOKABLE void startvibra(int duration, int intensity) const;
    Q_INVOKABLE void stopvibra() const;

    // Memory and file system information
    Q_PROPERTY(int totalram READ totalRam);
    Q_PROPERTY(int freeram READ freeRam);
    Q_PROPERTY(QString drivelist READ driveList);

    int totalRam() const;
    int freeRam() const;
    QString driveList() const;
    Q_INVOKABLE quint64 drivesize(const QString& drive) const;
    Q_INVOKABLE quint64 drivefree(const QString& drive) const;

    // Beep tone control services
    Q_INVOKABLE void beep(int frequency, int duration) const;

    // System language information
    Q_PROPERTY(QString language READ language);

    QString language() const;

protected:
    QString m_oncharge;
    QString m_oncharging;
    QValueSpaceItem *m_vsi;
    QValueSpaceItem *m_chargevsi;
    QValueSpaceItem *m_chargingvsi;
    QWebPage *m_page;
    QWebFrame *m_frame;
    QPhoneProfileManager *m_profileManager;

private slots:
    void chargeChanged();
    void chargingChanged();
};

#endif
