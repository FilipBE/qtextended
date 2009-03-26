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

#ifndef QPHONEPROFILE_H
#define QPHONEPROFILE_H

#include <QSharedDataPointer>
#include <QString>
#include <QContent>

class QSettings;
class QTranslatableSettings;
class QPhoneProfilePrivate;
class QTOPIA_EXPORT QPhoneProfile {
public:
    class QTOPIA_EXPORT Setting {
    public:
        Setting();
        explicit Setting(const QString &);
        Setting(const Setting &);
        Setting &operator=(const Setting &);

        bool isNull() const;

        bool operator==(const Setting &) const;
        bool operator!=(const Setting &) const;

        QString applicationName() const;
        QString description() const;
        QString data() const;
        bool notifyOnChange() const;

        void setApplicationName(const QString &);
        void setDescription(const QString &);
        void setData(const QString &);
        void setNotifyOnChange(bool);

    private:
        QString appName;
        QString appTitle;
        QString details;
        bool notify;
    };
    typedef QMap<QString,Setting> Settings;

    class QTOPIA_EXPORT Schedule {
    public:
        Schedule();
        explicit Schedule(const QString &);
        Schedule(const Schedule &);
        Schedule &operator=(const Schedule &);
        bool operator==(const Schedule &) const;
        bool operator!=(const Schedule &) const;

        bool isActive() const;
        void setActive(bool);

        QTime time() const;
        void setTime(const QTime &);

        QList<Qt::DayOfWeek> scheduledOnDays() const;
        bool scheduledOnDay(Qt::DayOfWeek) const;
        void setScheduledDay(Qt::DayOfWeek);
        void unsetScheduledDay(Qt::DayOfWeek);
        void clearScheduledDays();

        QString toString() const;
        void fromString(const QString &);

    private:
        bool _active;
        QTime _time;
        unsigned char _days;
    };

    enum AlertType {
        Off,
        Once,
        Continuous,
        Ascending
    };

    enum VideoOption {
        AlwaysOff,
        OnForIncoming,
        OnForOutgoing,
        AlwaysOn
    };

    QPhoneProfile();
    explicit QPhoneProfile(int id);
    QPhoneProfile(const QPhoneProfile &);
    QPhoneProfile &operator=(const QPhoneProfile &);
    virtual ~QPhoneProfile();

    bool operator==(const QPhoneProfile &) const;
    bool operator!=(const QPhoneProfile &) const;

    bool isNull() const;

    QString name() const;
    bool isSystemProfile() const;
    int volume() const;
    bool vibrate() const;
    AlertType callAlert() const;
    AlertType msgAlert() const;
    VideoOption videoOption() const;
    int msgAlertDuration() const;
    bool autoAnswer() const;
    QContent callTone() const;
    QContent videoTone() const;
    QContent systemCallTone() const;
    QContent messageTone() const;
    QContent systemMessageTone() const;
    bool planeMode() const;
    Settings applicationSettings() const;
    Setting applicationSetting(const QString &) const;
    QString icon() const;
    Schedule schedule() const;
    int id() const;
    QString audioProfile() const;
#ifdef QTOPIA_TELEPHONY
    QString speedDialInput() const;
#endif

    void setId(int id);
    void setIcon(const QString &);
    void setName(const QString &);
    void setIsSystemProfile(bool);
    void setVolume(int);
    void setVibrate(bool);
    void setCallAlert(AlertType);
    void setMsgAlert(AlertType );
    void setVideoOption(VideoOption);
    void setMsgAlertDuration(int);
    void setAutoAnswer(bool);
    void setPlaneMode(bool);
    void setApplicationSettings(const Settings &);
    void setApplicationSetting(const Setting &);
    void setSchedule(const Schedule &);
    void setCallTone(const QContent &);
    void setVideoTone(const QContent &);
    void setMessageTone(const QContent &);
    void setAudioProfile(const QString &);
#ifdef QTOPIA_TELEPHONY
    void setSpeedDialInput(const QString &);
#endif

private:
    QSharedDataPointer<QPhoneProfilePrivate> d;
    void read(QTranslatableSettings &);
    void write(QSettings &) const;
    friend class QPhoneProfileManager;
};

class QPhoneProfileManagerPrivate;
class QTOPIA_EXPORT QPhoneProfileManager : public QObject
{
Q_OBJECT
public:
    explicit QPhoneProfileManager(QObject *parent = 0);
    virtual ~QPhoneProfileManager();

    QPhoneProfile activeProfile() const;
    bool activateProfile(const QPhoneProfile &);
    bool activateProfile(int);

    bool planeMode() const;
    bool planeModeOverride() const;
    void setPlaneModeOverride(bool);

    bool planeModeAvailable() const;

    QList<QPhoneProfile> profiles() const;
    QList<int> profileIds() const;

    QPhoneProfile profile(int) const;
    void saveProfile(const QPhoneProfile &);
    void removeProfile(const QPhoneProfile &);

    QPhoneProfile newProfile();

    void sync();

signals:
    void planeModeChanged(bool);
    void activeProfileChanged(const QPhoneProfile &);
    void profileUpdated(const QPhoneProfile &);
    void profileAdded(const QPhoneProfile &);
    void profileRemoved(const QPhoneProfile &);

private slots:
    void profMessage(const QString &msg, const QByteArray &data);

private:
    void loadConfig();
    Q_DISABLE_COPY(QPhoneProfileManager);

    QPhoneProfileManagerPrivate *d;

    void activateProfile(const QPhoneProfile &newProfile,
                         const QPhoneProfile &oldProfile);
    void activateSettings(const QPhoneProfile::Settings &current,
                          const QPhoneProfile::Settings &previous);
};

#endif
