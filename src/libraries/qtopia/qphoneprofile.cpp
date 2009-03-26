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

#include "qphoneprofile.h"

#include <QSettings>
#include <QHash>
#include <QTranslatableSettings>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include <QtopiaApplication>
#include <QSharedData>
#include <QContentSet>


/*!
  \class QPhoneProfile::Setting
    \inpublicgroup QtBaseModule

  \brief The Setting class provides applications with a mechanism to add their own settings to a profile.

  Applications may integrate with the Qt Extended profiles by adding their own
  settings.  Optionally, when the profile changes the application will be
  notified and instructed to apply the new settings.

  Most profile editing applications (such as Ring Profiles in Qt Extended Phone) will
  use the Settings service to interactively acquire settings from applications
  to add in this way, but it is not necessary to do so.

  Application settings are added to a profile through the
  QPhoneProfile::setApplicationSettings() or
  QPhoneProfile::setApplicationSetting() methods and read via the
  QPhoneProfile::applicationSetting() call.

  \sa QPhoneProfile

  \ingroup io
 */

// define QPhoneProfile::Setting
/*!
  Constructs a null application setting object.
 */
QPhoneProfile::Setting::Setting()
: notify(false)
{
}

/*!
  Constructs an empty application setting object for \a application.
  */
QPhoneProfile::Setting::Setting(const QString &application)
: appName(application), notify(false)
{
}

/*!
  Constructs a copy of \a other.
  */
QPhoneProfile::Setting::Setting(const Setting &other)
: appName(other.appName), appTitle(other.appTitle), details(other.details),
  notify(other.notify)
{
}

/*!
  Assigns \a other to this setting and returns a reference to this setting.
  */
QPhoneProfile::Setting &QPhoneProfile::Setting::operator=(const Setting &other)
{
    appName = other.appName;
    appTitle = other.appTitle;
    details = other.details;
    notify = other.notify;
    return *this;
}

/*!
  Returns true if this setting is null; otherwise returns false.
  A null object is one that has an empty applicationName().
  */
bool QPhoneProfile::Setting::isNull() const
{
    return appName.isEmpty();
}

/*!
  Returns the application to which this setting object applies, or an empty
  string if this is a null settings object.
 */
QString QPhoneProfile::Setting::applicationName() const
{
    return appName;
}

/*!
  Returns an optional visual description for this setting.
  Generally this is the visual name of the application.
  */
QString QPhoneProfile::Setting::description() const
{
    return appTitle;
}

/*!
  Returns the data portion of this setting.
 */
QString QPhoneProfile::Setting::data() const
{
    return details;
}

/*!
  Returns true if the application will be notified when the profile changes;
  otherwise returns false.

  Applications will be notified with a \c {Settings::activateSettings(QString)}
  message via \c {QPE/Application/appname} when they are to apply settings.
  \c {appname} is applicationName() and the parameter is data() portion of the setting.

  If the profile changes to one where the
  application has not added a custom setting from one where the application has
  added a custom setting, a \c {Settings::activateDefault()} message will
  be sent.

  Therefore applications should be listening to the Qt Extended application channels to handle these requests.
  For example, in the application:

  \code
        SomeApplication::SomeApplication()
        {
            connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
                this, SLOT(receive(QString,QByteArray)) );
            ...
        }

        void SomeApplication::receive( const QString& message, const QByteArray& data )
        {
            QDataStream stream( data );
            if ( message == "Settings::activateSettings(QString)" ) {
                ... // parse and apply application settings data
            } else if ( message == "Settings::activateDefault()" ) {
                applyDefault(); // apply application default settings
            }
        }
  \endcode

  \sa SettingsService, QtopiaApplication::appMessage()
 */
bool QPhoneProfile::Setting::notifyOnChange() const
{
    return notify;
}

/*!
  Sets the application \a name for this setting.
  */
void QPhoneProfile::Setting::setApplicationName(const QString &name)
{
    appName = name;
}

/*!
  Sets the setting \a description for this setting.
  */
void QPhoneProfile::Setting::setDescription(const QString &description)
{
    appTitle = description;
}

/*!
  Set the setting \a data for this setting.
 */
void QPhoneProfile::Setting::setData(const QString &data)
{
    details = data;
}

/*!
  Sets whether the application is notified on changes to the profile or not.
  If \a notifyOnChange is true applications are notified, otherwise they are
  not.
  */
void QPhoneProfile::Setting::setNotifyOnChange(bool notifyOnChange)
{
    notify = notifyOnChange;
}

/*!
  Returns true if setting \a other is equal to this setting; otherwise returns false.
  */
bool QPhoneProfile::Setting::operator==(const Setting &other) const
{
    return appName == other.appName &&
           appTitle == other.appTitle &&
           details == other.details &&
           notify == other.notify;
}

/*!
  Returns true if setting \a other is not equal to this setting; otherwise returns false.
 */
bool QPhoneProfile::Setting::operator!=(const Setting &other) const
{
    return !(other == *this);
}

// declare QPhoneProfilePrivate
class QPhoneProfilePrivate : public QSharedData
{
public:
    QPhoneProfilePrivate()
    : mSaveId(-1), mIsSystemProfile(false), mVolume(3), mVibrate(true),
      mCallAlert(QPhoneProfile::Continuous), mMsgAlert(QPhoneProfile::Once),
      mVideoOption(QPhoneProfile::AlwaysOff),
      mMsgAlertDuration(5000), mAutoAnswer(false), mPlaneMode(false) {}
    QPhoneProfilePrivate &operator=(const QPhoneProfilePrivate &o);

    QString mName;
    int mSaveId;
    bool mIsSystemProfile;
    int mVolume;
    bool mVibrate;
    QPhoneProfile::AlertType mCallAlert;
    QPhoneProfile::AlertType mMsgAlert;
    QPhoneProfile::VideoOption mVideoOption;
    int mMsgAlertDuration;
    bool mAutoAnswer;
    QContent mCallTone;
    QContent mVideoTone;
    QContent mMessageTone;
    bool mPlaneMode;
    QString mIcon;
    QPhoneProfile::Settings mSettings;
    QPhoneProfile::Schedule mSchedule;
    QString mAudioProfile;
#ifdef QTOPIA_TELEPHONY
    QString mSpeedDialInput;
#endif
};

// define QPhoneProfilePrivate
QPhoneProfilePrivate &QPhoneProfilePrivate::operator=(const QPhoneProfilePrivate &o)
{
    mName = o.mName;
    mIsSystemProfile = o.mIsSystemProfile;
    mVolume = o.mVolume;
    mVibrate = o.mVibrate;
    mCallAlert = o.mCallAlert;
    mMsgAlert = o.mMsgAlert;
    mMsgAlertDuration = o.mMsgAlertDuration;
    mVideoOption = o.mVideoOption;
    mAutoAnswer = o.mAutoAnswer;
    mCallTone = o.mCallTone;
    mVideoTone = o.mVideoTone;
    mMessageTone = o.mMessageTone;
    mPlaneMode = o.mPlaneMode;
    mIcon = o.mIcon;
    mSaveId = o.mSaveId;
    mSettings = o.mSettings;
    mSchedule = o.mSchedule;
    mAudioProfile = o.mAudioProfile;
#ifdef QTOPIA_TELEPHONY
    mSpeedDialInput = o.mSpeedDialInput;
#endif

    return *this;
}

/*!
  \class QPhoneProfile
    \inpublicgroup QtBaseModule

  \brief The QPhoneProfile class encapsulates a single phone profile
         configuration.

  The QPhoneProfile class holds data on how the phone
  behaves for incoming calls or messages when it is activated.
  When a new profile is created it plays system default tones,
  systemCallTone() and systemMessageTone(),
  which can be personalized by setCallTone() and setMessageTone().

  \section1 Activation

  A profile can be manually activated through the
  QPhoneProfileManager::activateProfile() method.
  Alternatively a profile may be set to auto-activate at certain times,
  controlled through by QPhoneProfile::schedule(),
  or when a phone audio profile
  is attached by QPhoneProfile::setAudioProfile().
  For example, a profile can be automatically activated
  when Bluetooth hands-free is enabled. 

  \sa QPhoneProfile::Schedule, QHardwareInterface

  \section1 Associated Settings

  A profile can have a number of associated settings
  that can be retrieved by applicationSetting() and applicationSettings().
  These settings are passed to applications
  by SettingsService::activateSettings() to be applied when the profile is activated.

  Applications that wish to add their settings to a profile can use SettingsManagerService.

  \sa QPhoneProfileManager, QPhoneProfileProvider, SettingsManagerService, SettingsService

  \ingroup io
 */

// define QPhoneProfile
static const QString cName("Name"); // no tr
static const QString cSystem("System"); // no tr
static const QString cVolume("Volume"); // no tr
static const QString cVibrate("Vibrate"); // no tr
static const QString cCallA("CallAlert"); // no tr
static const QString cMsgA("MsgAlert"); // no tr
static const QString cMsgADuration("MsgAlertDuration");
static const QString cVideoOption("VideoOption");
static const QString cAutoAnswer("AutoAnswer"); // no tr
static const QString cCallTone("RingTone"); // no tr
static const QString cVideoTone("VideoTone"); // no tr
static const QString cMessageTone("MessageTone"); // no tr
static const QString cPlaneMode("PlaneMode"); // no tr
static const QString cIcon("Icon"); // no tr
static const QString cAudioProfile("AudioProfile"); // no tr
#ifdef QTOPIA_TELEPHONY
static const QString cSpeedDialInput("SpeedDialInput");
#endif
static const QString cDescription("Description"); // no tr
static const QString cData("Data"); // no tr
static const QString cNotify("Notify"); // no tr

static const QContent *systemRingTone = 0;
static const QContent *systemAlertTone = 0;

static QContent findSystemRingTone(const QString &name)
{
    static QContentSet *systemRingTones = 0;
    if (!systemRingTones)
        systemRingTones = new QContentSet( QContentFilter::Category, QLatin1String( "SystemRingtones" ));

    return systemRingTones->findFileName(name);
}

/*!
  \enum QPhoneProfile::AlertType

  Controls how a ring or message tone is played.

  \value Off No tone is played.
  \value Once The tone is played once, from beginning to end.
  \value Continuous The tone is played repeatedly until the user acknowledges
         the alert.
  \value Ascending The tone is played repeatedly with increasing volume each
         time.
  */

/*!
  \enum QPhoneProfile::VideoOption

  Determines whether the video is turned or not for incoming or outgoing calls.

  \value AlwaysOff The video is always off.
  \value OnForIncoming The video will be turned on for incoming calls.
  \value OnForOutgoing The video will be turned on for outgoing calls.
  \value AlwaysOn The video will be turned on for both incoming and outgoing calls.
*/

/*!
  \typedef QPhoneProfile::Settings

  This is a convenience typedef to encapsulate a mapping between application
  names and their corresponding Setting object.  The exact type is:
  \code
  QMap<QString, QPhoneProfile::Setting>
  \endcode
  */

/*!
  Constructs a null QPhoneProfile.

  It is recommended to use QPhoneProfileManager::newProfile() to create a new profile.
  */
QPhoneProfile::QPhoneProfile()
: d(new QPhoneProfilePrivate)
{
}

/*!
  Constructs an empty QPhoneProfile with the given \a id.

  It is recommended to use QPhoneProfileManager::newProfile() to create a new profile.
 */
QPhoneProfile::QPhoneProfile(int id)
: d(new QPhoneProfilePrivate)
{
    d->mSaveId = id;
}

/*!
  Constructs a copy of \a other.
 */
QPhoneProfile::QPhoneProfile(const QPhoneProfile &other)
: d(other.d)
{
}

/*!
  Destroys the QPhoneProfile object.
 */
QPhoneProfile::~QPhoneProfile()
{
}

/*!
  \fn bool QPhoneProfile::operator==(const QPhoneProfile &other) const

  Returns true if this profile is equal to \a other; otherwise returns false.
  Equality means all the profile properties are equivalent.
 */
bool QPhoneProfile::operator==(const QPhoneProfile &o) const
{
    return d->mName == o.d->mName &&
           d->mIsSystemProfile == o.d->mIsSystemProfile &&
           d->mVolume == o.d->mVolume &&
           d->mVibrate == o.d->mVibrate &&
           d->mCallAlert == o.d->mCallAlert &&
           d->mMsgAlert == o.d->mMsgAlert &&
           d->mMsgAlertDuration == o.d->mMsgAlertDuration &&
           d->mVideoOption == o.d->mVideoOption &&
           d->mAutoAnswer == o.d->mAutoAnswer &&
           d->mCallTone == o.d->mCallTone &&
           d->mVideoTone == o.d->mVideoTone &&
           d->mMessageTone == o.d->mMessageTone &&
           d->mPlaneMode == o.d->mPlaneMode &&
           d->mIcon == o.d->mIcon &&
           d->mSaveId == o.d->mSaveId &&
           d->mSettings == o.d->mSettings &&
           d->mSchedule == o.d->mSchedule;
}

/*!
  \fn bool QPhoneProfile::operator!=(const QPhoneProfile &other) const

  Returns true if this profile is not equal to \a other; otherwise returns false.
  */
bool QPhoneProfile::operator!=(const QPhoneProfile &o) const
{
    return !(*this == o);
}

/*!
  Assigns \a other to this profile and returns a reference to this profile.
  */
QPhoneProfile &QPhoneProfile::operator=(const QPhoneProfile &other)
{
    d = other.d;
    return *this;
}

/*!
  Returns true if this profile is null; otherwise returns false.
  A null profile is one with an id of -1.
  */
bool QPhoneProfile::isNull() const
{
    return id() == -1;
}

/*!
  Returns the user visible name of this profile.
 */
QString QPhoneProfile::name() const
{
    return d->mName;
}

/*!
  Returns true if this is a system profile; otherwise returns false.
  System profiles can typically not be deleted.
 */
bool QPhoneProfile::isSystemProfile() const
{
    return d->mIsSystemProfile;
}

/*!
  Returns the volume for this profile.
 */
int QPhoneProfile::volume() const
{
    return d->mVolume;
}

/*!
  Returns true if vibration is enabled; otherwise returns false.
 */
bool QPhoneProfile::vibrate() const
{
    return d->mVibrate;
}

/*!
  Returns the AlertType to use for incoming calls.
  */
QPhoneProfile::AlertType QPhoneProfile::callAlert() const
{
    return d->mCallAlert;
}

/*!
  Returns the AlertType to use for incoming messages.
 */
QPhoneProfile::AlertType QPhoneProfile::msgAlert() const
{
    return d->mMsgAlert;
}

/*!
  Returns the VideoOption to use for calls.
*/
QPhoneProfile::VideoOption QPhoneProfile::videoOption() const
{
    return d->mVideoOption;
}

/*!
  Returns the duration in milliseconds to play the message tone.
  */
int QPhoneProfile::msgAlertDuration() const
{
    return d->mMsgAlertDuration;
}

/*!
  Returns true if incoming calls should be automatically answered in this
  profile; otherwise returns false.
 */
bool QPhoneProfile::autoAnswer() const
{
    return d->mAutoAnswer;
}

/*!
  Returns the default ring tone to use for incoming calls.
  */
QContent QPhoneProfile::callTone() const
{
    return d->mCallTone;
}

/*!
  Returns the system ring tone to use for incoming calls.
  The system ring tone will be used when the user defined tone cannot be found.
*/
QContent QPhoneProfile::systemCallTone() const
{
    if (!systemRingTone)
        systemRingTone = new QContent(findSystemRingTone(QLatin1String("phonering.wav")));
    return *systemRingTone;
}

/*!
  Returns the video ring tone to use for incoming calls.
*/
QContent QPhoneProfile::videoTone() const
{
    return d->mVideoTone;
}

/*!
  Returns the default ring tone to use for incoming messages.
 */
QContent QPhoneProfile::messageTone() const
{
    return d->mMessageTone;
}

/*!
  Returns the system message tone to use for incoming messages.
  The system message tone will be used when the user defined tone cannot be found.
*/
QContent QPhoneProfile::systemMessageTone() const
{
    if (!systemAlertTone)
        systemAlertTone = new QContent(findSystemRingTone(QLatin1String("alarm.wav")));
    return *systemAlertTone;
}

/*!
  Returns true if plane mode is on for this profile; otherwise returns false.
  Phone calls cannot be made in plane mode.
  */
bool QPhoneProfile::planeMode() const
{
    return d->mPlaneMode;
}

/*!
  Returns all application settings for this profile.
  */
QPhoneProfile::Settings QPhoneProfile::applicationSettings() const
{
    return d->mSettings;
}

/*!
  Returns the setting for \a application for this profile if applicable;
  otherwise returns a null setting.
 */
QPhoneProfile::Setting QPhoneProfile::applicationSetting(const QString &application) const
{
    return d->mSettings[application];
}

/*!
  Returns the file name for the icon to use to identify this profile.
  */
QString QPhoneProfile::icon() const
{
    return d->mIcon;
}

/*!
  Returns the auto activation schedule for this profile.
 */
QPhoneProfile::Schedule QPhoneProfile::schedule() const
{
    return d->mSchedule;
}

/*!
  Returns the identifier for this profile.
  */
int QPhoneProfile::id() const
{
    return d->mSaveId;
}

/*!
  Returns the audio profile on which this profile should auto activate if applicable;
  otherwise returns an empty string.

  \sa QAudioStateInfo
 */
QString QPhoneProfile::audioProfile() const
{
    return d->mAudioProfile;
}

#ifdef QTOPIA_TELEPHONY
/*!
  Returns the speed dial input on which this profile should activate if applicable;
  otherwise returns an empty string.
  */
QString QPhoneProfile::speedDialInput() const
{
    return d->mSpeedDialInput;
}
#endif

/*!
  Sets the profile \a id.
  */
void QPhoneProfile::setId(int id)
{
    d->mSaveId = id;
}

/*!
  Sets the profile \a name.
  */
void QPhoneProfile::setName(const QString &name)
{
    d->mName = name;
}

/*!
  Sets whether this profile is a system profile to \a isSystemProfile.
  */
void QPhoneProfile::setIsSystemProfile(bool isSystemProfile)
{
    d->mIsSystemProfile = isSystemProfile;
}

/*!
  Sets the profile \a volume. Valid values are from 0 to 5.
  */
void QPhoneProfile::setVolume(int volume)
{
    d->mVolume = qMin(5, qMax(0, volume));
}

/*!
  Sets the profile vibration to \a vibrate.
  */
void QPhoneProfile::setVibrate(bool vibrate)
{
    d->mVibrate = vibrate;
}

/*!
  Sets the incoming call alert \a type.
  */
void QPhoneProfile::setCallAlert(AlertType type)
{
    d->mCallAlert = type;
}

/*!
  Sets the incoming message alert \a type.
  */
void QPhoneProfile::setMsgAlert(AlertType type)
{
    d->mMsgAlert = type;
}

/*!
  Sets the video option to \a option.
*/
void QPhoneProfile::setVideoOption(VideoOption option)
{
    d->mVideoOption = option;
}

/*!
  Sets the duration in milliseconds to play the message tone to \a ms.
  */
void QPhoneProfile::setMsgAlertDuration(int ms)
{
    d->mMsgAlertDuration = ms;
}

/*!
  Sets whether auto answering is enabled for this profile to \a autoAnswer.
  */
void QPhoneProfile::setAutoAnswer(bool autoAnswer)
{
    d->mAutoAnswer = autoAnswer;
}

/*!
  Sets whether plane mode is enabled for this profile to \a planeMode.
  Phone calls cannot be made in plane mode.
 */
void QPhoneProfile::setPlaneMode(bool planeMode)
{
    d->mPlaneMode = planeMode;
}

/*!
  Sets the application \a settings.
 */
void QPhoneProfile::setApplicationSettings(const Settings &settings)
{
    d->mSettings = settings;
}

/*!
  Adds a single application \a setting if it does not exist;
  otherwise updates the existing setting.
 */
void QPhoneProfile::setApplicationSetting(const Setting &setting)
{
    if(!setting.applicationName().isEmpty())
        d->mSettings.insert(setting.applicationName(), setting);
}

/*!
  Sets the profile's auto activation \a schedule.
 */
void QPhoneProfile::setSchedule(const Schedule &schedule)
{
    d->mSchedule = schedule;
}

/*!
  \fn void QPhoneProfile::setCallTone(const QContent &tone)

  Sets the incoming call \a tone.

  \sa setVideoTone()
  */
void QPhoneProfile::setCallTone(const QContent &l)
{
    d->mCallTone = l;
}

/*!
  Sets the incoming video ring \a tone.

  \sa setCallTone()
*/
void QPhoneProfile::setVideoTone(const QContent &tone)
{
    d->mVideoTone = tone;
}

/*!
  \fn void QPhoneProfile::setMessageTone(const QContent &tone)

  Sets the incoming message \a tone.
 */
void QPhoneProfile::setMessageTone(const QContent &l)
{
    d->mMessageTone = l;
}

/*!
  Sets the auto activation \a audioProfile. A phone profile
  is activated if
  \code
    QPhoneProfile profile;
    QAudioStateConfiguration config;
    QAudioStateInfo info = config.currentState();
    info.profile() == profile.audioProfile() //must be true
  \endcode

  \sa QAudioStateConfiguration, QAudioStateInfo
 */
void QPhoneProfile::setAudioProfile(const QString &audioProfile)
{
    d->mAudioProfile = audioProfile;
}

#ifdef QTOPIA_TELEPHONY
/*!
  Sets the speed dial \a input.
  */
void QPhoneProfile::setSpeedDialInput(const QString &input)
{
    d->mSpeedDialInput = input;
}
#endif

/*!
  \internal
  Reads the configuration file and populates the properties for this profile.
*/
void QPhoneProfile::read(QTranslatableSettings &c)
{
    d->mName=c.value(cName).toString();
    setIsSystemProfile(c.value(cSystem).toBool());
    setVolume(c.value(cVolume).toInt());
    setVibrate(c.value(cVibrate).toBool());
    setCallAlert((QPhoneProfile::AlertType)c.value(cCallA).toInt());
    setMsgAlert((QPhoneProfile::AlertType)c.value(cMsgA).toInt());
    setVideoOption((QPhoneProfile::VideoOption)c.value(cVideoOption).toInt());
    setMsgAlertDuration(c.value(cMsgADuration).toInt());
    setAutoAnswer(c.value(cAutoAnswer).toBool());
    setPlaneMode(c.value(cPlaneMode).toBool());
    setIcon(c.value(cIcon).toString());
    setAudioProfile(c.value(cAudioProfile).toString());
#ifdef QTOPIA_TELEPHONY
    setSpeedDialInput(c.value(cSpeedDialInput).toString());
#endif

    QContent link = QContent(c.value(cCallTone).toString());
    if (link.fileKnown())
        d->mCallTone = link;

    link = QContent(c.value(cVideoTone).toString());
    if (link.fileKnown())
        d->mVideoTone = link;

    link = QContent(c.value(cMessageTone).toString());
    if (link.fileKnown())
        d->mMessageTone = link;
    else
       d->mMessageTone = systemMessageTone();

    QString str = c.value("SettingList").toString();
    QStringList settingList;
    if (!str.isEmpty())
        settingList = str.split( ',' );

    // load associated settings
    Settings settings;
    Setting setting;
    QString settingName, tmp;

    for(int i = 0; i < settingList.size(); i++) {
        settingName = settingList.at(i);
        setting.setApplicationName(settingName);
        c.beginGroup(settingName);
        setting.setDescription(c.value(cDescription).toString());
        setting.setData(c.value(cData).toString());
        setting.setNotifyOnChange(c.value(cNotify, false).toBool());
        c.endGroup();
        settings[setting.applicationName()] = setting;
    }
    setApplicationSettings(settings);

    d->mSchedule = Schedule(c.value("Schedule").toString());
}

/*!
  \fn void QPhoneProfile::setIcon(const QString &fileName)

  Sets the profile's icon to \a fileName.
  */
void QPhoneProfile::setIcon(const QString &icon)
{
    d->mIcon = icon;
    if(!d->mIcon.startsWith('/') && !d->mIcon.startsWith(":image/"))
        d->mIcon = ":image/" + d->mIcon;
}

/*!
  \internal
  Writes the properties of this profile to the configuration file.
*/
void QPhoneProfile::write(QSettings &c) const
{
    // do not write system profile's name.
    // it should be translated into different languages
    if (!isSystemProfile())
        c.setValue(cName, d->mName);
    c.setValue(cSystem, isSystemProfile());
    c.setValue(cVolume, volume());
    c.setValue(cVibrate, vibrate());
    c.setValue(cCallA, (int)callAlert());
    c.setValue(cMsgA, (int)msgAlert());
    c.setValue(cVideoOption, (int)videoOption());
    c.setValue(cMsgADuration, msgAlertDuration());
    c.setValue(cAutoAnswer, autoAnswer());
    c.setValue(cPlaneMode, planeMode());
    c.setValue(cIcon, icon());
    c.setValue(cAudioProfile, audioProfile());
#ifdef QTOPIA_TELEPHONY
    c.setValue(cSpeedDialInput, speedDialInput());
#endif

    if ( d->mCallTone.fileKnown() )
        c.setValue(cCallTone, d->mCallTone.file());
    if ( d->mVideoTone.fileKnown() )
        c.setValue(cVideoTone, d->mVideoTone.file());
    if ( d->mMessageTone.fileKnown() )
        c.setValue(cMessageTone, d->mMessageTone.file());

    Settings s = applicationSettings();
    int sCount = s.count();
    if ( sCount > 0 ) {
        QStringList sList;
        QStringList keys = s.keys();
        QString currentKey;
        for ( int j = 0; j < s.count(); j++ ) {
            currentKey = keys.value( j );
            Setting setting = s[currentKey];
            c.beginGroup(setting.applicationName());
            c.setValue(cDescription, setting.description());
            c.setValue(cData, setting.data());
            c.setValue(cNotify, setting.notifyOnChange());
            sList += s[currentKey].applicationName();
            c.endGroup();
        }
        c.setValue( "SettingList", sList.join(QString(',')) );
    } else {
        c.setValue( "SettingList", QString() );
    }

    c.setValue("Schedule", d->mSchedule.toString());
}


/*!
  \class QPhoneProfile::Schedule
    \inpublicgroup QtBaseModule

  \brief The Schedule class provided information on timed auto-activation of a
         profile.

  Qt Extended profiles may be automatically activated on certain times and dates.
  The QPhoneProfile::Schedule class represents the times at which the profile
  should be activated.  A profile's schedule can be read through the
  QPhoneProfile::schedule() method, and set through a
  QPhoneProfile::setSchedule() call.

  \sa QPhoneProfile

  \ingroup io
 */
// define QPhoneProfile::Schedule
/*!
  Constructs a new, empty schedule.
  */
QPhoneProfile::Schedule::Schedule()
: _active(false), _days(0)
{
}

/*! \internal */
QPhoneProfile::Schedule::Schedule(const QString &str)
: _active(false), _days(0)
{
    fromString(str);
}

/*!
  Constructs a copy of \a other.
  */
QPhoneProfile::Schedule::Schedule(const Schedule &other)
: _active(other._active), _time(other._time), _days(other._days)
{
}

/*!
  Assigns \a other to this schedule and returns a reference to this schedule.
  */
QPhoneProfile::Schedule::Schedule &QPhoneProfile::Schedule::operator=(const Schedule &other)
{
    _active = other._active;
    _time = other._time;
    _days = other._days;
    return *this;
}

/*!
  \fn bool QPhoneProfile::Schedule::operator==(const Schedule &other) const

  Returns true if this schedule is equal to \a other; otherwise returns false.
  */
bool QPhoneProfile::Schedule::operator==(const QPhoneProfile::Schedule &other) const
{
    return _active == other._active &&
           _time == other._time &&
           _days == other._days;
}

/*!
  \fn bool QPhoneProfile::Schedule::operator!=(const Schedule &other) const

  Returns true if this schedule is not equal to \a other; otherwise returns false.
  */
bool QPhoneProfile::Schedule::operator!=(const QPhoneProfile::Schedule &other) const
{
    return !(other == *this);
}

/*!
  Returns true if the schedule is active.
  Inactive schedules will be ignored by the auto activation system.
 */
bool QPhoneProfile::Schedule::isActive() const
{
    return _active;
}

/*!
  Sets whether this schedule is enabled to \a active.
 */
void QPhoneProfile::Schedule::setActive(bool active)
{
    _active = active;
}

/*!
  Returns the activation time of day for this schedule.
 */
QTime QPhoneProfile::Schedule::time() const
{
    return _time;
}

/*!
  Sets the activation \a time of day for this schedule.
  */
void QPhoneProfile::Schedule::setTime(const QTime &time)
{
    _time = time;
}

inline static unsigned char dayToMask(Qt::DayOfWeek day)
{
    unsigned char rv = 1;
    if ((int)day > 0 && day <= Qt::Sunday)
        rv <<= ((int)day - 1);
    return rv;
}

/*!
  Returns true the activation is scheduled on \a day for this schedule.
 */
bool QPhoneProfile::Schedule::scheduledOnDay(Qt::DayOfWeek day) const
{
    return _days & dayToMask(day);
}

/*!
  Returns the list of days on which the auto activation is scheduled.
 */
QList<Qt::DayOfWeek> QPhoneProfile::Schedule::scheduledOnDays() const
{
    QList<Qt::DayOfWeek> rv;

    unsigned char day = 1;
    for(int ii = 1; ii <= 7; ++ii) {
        if(_days & day)
            rv.append((Qt::DayOfWeek)ii);
        day <<= 1;
    }

    return rv;
}

/*!
  Schedules the auto activation on \a day. If other days are set they will not
  be affected.
  */
void QPhoneProfile::Schedule::setScheduledDay(Qt::DayOfWeek day)
{
    _days |= dayToMask(day);
}

/*!
  Removes the auto activation scheduled on \a day.
 */
void QPhoneProfile::Schedule::unsetScheduledDay(Qt::DayOfWeek day)
{
    _days &= ~dayToMask(day);
}

/*!
  Clears all scheduled days for auto activation for this schedule.
  */
void QPhoneProfile::Schedule::clearScheduledDays()
{
    _days = 0;
}

/*!
  \internal

  Format is:

  \c {[on|off],<integer 0-6 for each scheduled day>,<time in minutes>,}

  For example, a schedule of 8:06 on Tuesday, Wednesday and Sunday would be

  \c {on,2,3,7,486,}
 */
QString QPhoneProfile::Schedule::toString() const
{
    QString rv;
    if(isActive())
        rv = QString("on,");
    else
        rv = QString("off,");

    unsigned char days = _days;
    for(int ii = (int)Qt::Monday; ii <= (int)Qt::Sunday; ++ii) {
        if(days & 0x01) {
            rv.append(QString::number(ii));
            rv.append(",");
        }
        days >>= 1;
    }

    int minutes = time().hour() * 60 + time().minute();
    rv.append(QString::number(minutes));

    rv.append(","); // Backwards compatibility

    return rv;
}

/*!
  \internal
  */
void QPhoneProfile::Schedule::fromString(const QString &str)
{
    *this = QPhoneProfile::Schedule();

    QStringList lst = str.split(',', QString::SkipEmptyParts);
    if(lst.isEmpty())
        return;

    if(lst.first() == "on") {
        setActive(true);
    } else if(lst.first() == "off") {
        setActive(false);
    }

    lst.pop_front();

    if(lst.isEmpty())
        return;

    int time = lst.last().toInt();
    setTime(QTime(time / 60, time % 60));

    lst.pop_back();

    for(int ii = 0; ii < lst.count(); ++ii) {
        int day = lst.at(ii).toInt();
        if(day >= Qt::Monday && day <= Qt::Sunday)
            setScheduledDay((Qt::DayOfWeek)day);
    }

    return;
}

// declare QPhoneProfileManagerPrivate
class QPhoneProfileManagerPrivate
{
public:
    QPhoneProfileManagerPrivate()
    : m_selected(-1), m_planeMode(false),
      m_planeModeAvailable(false), m_maxId(-1),
      m_constructing(true) {}

    QPhoneProfileManagerPrivate(const QPhoneProfileManagerPrivate &o)
    : m_selected(-1), m_planeMode(o.m_planeMode),
      m_planeModeAvailable(o.m_planeModeAvailable), m_maxId(o.m_maxId),
      m_constructing(o.m_constructing) {}

    int m_selected;
    bool m_planeMode;
    bool m_planeModeAvailable;
    int m_maxId;
    bool m_constructing;

    typedef QHash<int, QPhoneProfile> Profiles;
    Profiles m_profiles;
};

/*!
  \class QPhoneProfileManager
    \inpublicgroup QtBaseModule

  \brief The QPhoneProfileManager class allows applications to control phone profiles.

  The Qt Extended phone profiles are stored in the \c {Trolltech/PhoneProfile}
  configuration file.  A device may have any number of integrator or user
  defined profiles.  Each profile has a unique integer identifier which is used
  to refer to it by the system.

  To activate a profile manually use the QPhoneProfileManager::activateProfile() method.

  \sa QPhoneProfile, QPhoneProfileProvider

  \ingroup io
 */

// define QPhoneProfileManager
/*!
  Constructs a new QPhoneProfileManager with the given \a parent.
 */
QPhoneProfileManager::QPhoneProfileManager(QObject *parent)
: QObject(parent), d(new QPhoneProfileManagerPrivate)
{
    sync();

    QtopiaChannel *channel = new QtopiaChannel("QPE/PhoneProfiles", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(profMessage(QString,QByteArray)));

    d->m_constructing = false;
}

/*!
  Destroys the QPhoneProfileManager object.
 */
QPhoneProfileManager::~QPhoneProfileManager()
{
    delete d;
}

/*!
  \internal
  Reads the configuration file and create QPhoneProfile objects.
*/
void QPhoneProfileManager::loadConfig()
{
    d->m_profiles.clear();

    QTranslatableSettings c("Trolltech","PhoneProfile"); // no tr
    c.beginGroup("Profiles"); // no tr
    d->m_selected = c.value("Selected").toInt();
    d->m_planeMode = c.value("PlaneMode", false).toBool();
    d->m_planeModeAvailable = c.value("PlaneModeAvailable", false).toBool();

    QStringList profiles = c.value("Profiles").toString().split(','); // no tr
    for(QStringList::Iterator it = profiles.begin();
        it != profiles.end(); ++it) {

        c.endGroup();
        c.beginGroup("Profile " + (*it));

        int id = (*it).toInt();
        if (id > d->m_maxId)
            d->m_maxId = id;

        QPhoneProfile rpp(id);
        rpp.read(c);

        d->m_profiles.insert(id, rpp);
    }
}

/*!
  Returns a QPhoneProfile for the given \a id if exists;
  otherwise returns a null QPhoneProfile.
 */
QPhoneProfile QPhoneProfileManager::profile(int id) const
{
    QPhoneProfileManagerPrivate::Profiles::Iterator iter =
        d->m_profiles.find(id);
    if(iter == d->m_profiles.end())
        return QPhoneProfile();
    else
        return *iter;
}

/*!
  Returns all configured phone profiles.
 */
QList<QPhoneProfile> QPhoneProfileManager::profiles() const
{
    QList<QPhoneProfile> rv;

    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter =
            d->m_profiles.begin();
            iter != d->m_profiles.end();
            ++iter)
        rv.append(*iter);

    return rv;
}

/*!
  Returns a list of ids for all configured profiles.
 */
QList<int> QPhoneProfileManager::profileIds() const
{
    return d->m_profiles.keys();
}

/*!
  Returns true if plane mode is active; otherwise returns false.
  Plane mode is active if either it is overridden or
  the plane mode for the active profile is true.

  \sa planeModeOverride(), QPhoneProfile::planeMode()
 */
bool QPhoneProfileManager::planeMode() const
{
    return d->m_planeMode || activeProfile().planeMode();
}

/*!
  Returns true if plane mode override is on; otherwise returns false.
  The plane mode override allows plane mode to be enabled,
  even if the active profile does not specify it as such.

  \sa QPhoneProfile::planeMode()
 */
bool QPhoneProfileManager::planeModeOverride() const
{
    return d->m_planeMode;
}

/*!
  Sets the current plane mode override state to \a mode.
 */
void QPhoneProfileManager::setPlaneModeOverride(bool mode)
{
    d->m_planeMode = mode;
    QSettings c("Trolltech", "PhoneProfile");
    c.beginGroup("Profiles");
    c.setValue("PlaneMode", d->m_planeMode);

    QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Returns true if plane mode is available; otherwise returns false.
 */
bool QPhoneProfileManager::planeModeAvailable() const
{
    return d->m_planeModeAvailable;
}

/*!
  Returns the currently active profile, or a null profile if no profile is
  active.
 */
QPhoneProfile QPhoneProfileManager::activeProfile() const
{
    return profile(d->m_selected);
}

/*!
  \overload

  Returns true if able to activates the \a profile; otherwise returns false.
  If \a profile has not been saved, the saved version
  with the same QPhoneProfile::id() will be activated.

  This call is equivalent to \c {activateProfile(profile.id())}.
 */
bool QPhoneProfileManager::activateProfile(const QPhoneProfile &profile)
{
    return activateProfile(profile.id());
}

/*!
  \fn bool QPhoneProfileManager::activateProfile(int profile)

  Returns true if able to activate the \a profile id; otherwise returns false.
  If \a profile is -1 or the specified profile does not exist,
  a null (default values) profile will be activated.
 */
bool QPhoneProfileManager::activateProfile(int newProf)
{
    // Find the new profile
    QPhoneProfile newProfile = profile(newProf);

    // Find the existing profile
    QPhoneProfile oldProfile = activeProfile();

    activateProfile(newProfile, oldProfile);

    return (newProfile.id() != -1) || (newProf == -1);
}

/*!
  \internal
*/
void QPhoneProfileManager::activateProfile(const QPhoneProfile &newProfile,
                                           const QPhoneProfile &oldProfile)
{
    // Update settings file
    {
        QSettings cfg("Trolltech", "PhoneProfile");
        cfg.beginGroup("Profiles");
        cfg.setValue("Selected", newProfile.id());
    }

    // Send changed message
    {
        QtopiaIpcEnvelope e( "QPE/PhoneProfiles", "profileChanged()" );
    }

    // Update external application settings
    activateSettings(newProfile.applicationSettings(),
                     oldProfile.applicationSettings());
}

/*!
  \internal
  Activates associated application settings if any.
  If the previous profile had associated settings
  that the current profile does not have
  default settings for those application is applied.
*/
void QPhoneProfileManager::activateSettings(const QPhoneProfile::Settings &current, const QPhoneProfile::Settings &previous)
{
    // list of all available settings
    QStringList allapps = QtopiaService::apps("Settings");

    // count of current profile's associated sttings
    foreach(QPhoneProfile::Setting setting, current) {
        if(setting.notifyOnChange()) {
            // activate the setting with the saved details
            QtopiaIpcEnvelope e("QPE/Application/" + setting.applicationName(),
                                "Settings::activateSettings(QString)");
            e << setting.data();
        }

        // remove from the list of all settings
        allapps.removeAll(setting.applicationName());
    }

    // if the previous profile has associated settings
    // that are not included in the current profile,
    // activate the default state for each of them
    foreach(QPhoneProfile::Setting setting, previous) {
        if(setting.notifyOnChange() &&
           allapps.contains(setting.applicationName())) {
            QtopiaIpcEnvelope e("QPE/Application/" + setting.applicationName(),
                                "Settings::activateDefault()");
        }
    }

}

/*!
  Saves the specified \a profile.

  Saving a profile will overwrite any previously saved information.
 */
void QPhoneProfileManager::saveProfile(const QPhoneProfile &profile)
{
    if(profile.id() == -1 /* XXX || !profile.mTainted */)
        return;

    d->m_profiles.insert(profile.id(), profile);

    // Actually write to disk
    QSettings c("Trolltech", "PhoneProfile");
    c.remove("Profile " + QString::number(profile.id()));
    c.beginGroup("Profile " + QString::number(profile.id()));
    profile.write(c);
    c.endGroup();
    c.beginGroup("Profiles");
    QStringList profs =
        c.value("Profiles").toString().split(',', QString::SkipEmptyParts);
    if(!profs.contains(QString::number(profile.id()))) {
        profs.append(QString::number(profile.id()));
        c.setValue("Profiles", profs.join(QString(',')));
    }

    if(profile.id() > d->m_maxId)
        d->m_maxId = profile.id();

    QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Removes \a profile. If \a profile is a null profile, no action will be
  taken.

  If \a profile is active, the default profile (QPhoneProfile::id() == 1)
  will be activated.
 */
void QPhoneProfileManager::removeProfile(const QPhoneProfile &profile)
{
    if(-1 == profile.id())
        return;

    bool wasActive = false;
    if(activeProfile().id() == profile.id()) {
        wasActive = true;
        if(activeProfile().id() == 1) {
            activateProfile(QPhoneProfile());
        } else {
            activateProfile(this->profile(1));
        }
    }

    QPhoneProfileManagerPrivate::Profiles::Iterator iter =
        d->m_profiles.find(profile.id());
    if(iter != d->m_profiles.end()) {
        d->m_profiles.erase(iter);
        QSettings c("Trolltech", "PhoneProfile");
        c.remove("Profile " + QString::number(profile.id()));
        // remove id from id list
        c.beginGroup("Profiles"); // no tr
        QStringList profiles = c.value("Profiles").toString().split(','); // no tr
        profiles.removeAll(QString::number(profile.id()));
        c.setValue("Profiles", profiles.join(QString(',')));
    }

    if(!wasActive)
        QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Returns a new QPhoneProfile instance with its id set to the next available
  integer.
  */
QPhoneProfile QPhoneProfileManager::newProfile()
{
    return QPhoneProfile(++d->m_maxId);
}

/*!
  \internal
*/
void QPhoneProfileManager::profMessage(const QString &msg,
                                       const QByteArray &)
{
    if("profileChanged()" == msg)
        sync();
}

/*!
  Forces the re-reading of the profile information.
  */
void QPhoneProfileManager::sync()
{
    if(d->m_constructing) {
        loadConfig();
        return;
    }

    bool oldPlaneMode = planeMode();
    QPhoneProfileManagerPrivate::Profiles oldProfiles = d->m_profiles;
    int oldSelected = d->m_selected;

    loadConfig();

    // Work out what changed
    bool activeChanged = oldSelected != d->m_selected;
    bool hasPlaneModeChanged = oldPlaneMode != planeMode();

    QList<QPhoneProfile> added;
    QList<QPhoneProfile> changed;

    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter = d->m_profiles.begin(); iter != d->m_profiles.end(); ++iter) {

        QPhoneProfileManagerPrivate::Profiles::Iterator olditer = oldProfiles.find(iter.key());

        if(olditer == oldProfiles.end()) {
            added.append(*iter);
        } else if(*olditer != *iter) {
            if(iter->id() == d->m_selected)
                activeChanged = true;
            changed.append(*iter);
        }

        oldProfiles.erase(olditer);
    }


    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter = oldProfiles.begin(); iter != oldProfiles.end(); ++iter) {
        emit profileRemoved(*iter);
    }

    for(int ii = 0; ii < changed.count(); ++ii)
        emit profileUpdated(changed.at(ii));

    for(int ii = 0; ii < added.count(); ++ii)
        emit profileAdded(added.at(ii));

    if(activeChanged)
        emit activeProfileChanged(activeProfile());

    if(hasPlaneModeChanged)
        emit planeModeChanged(planeMode());
}

/*!
  \fn void QPhoneProfileManager::planeModeChanged(bool enabled)

  This signal is emitted whenever the plane mode state changes.
  When plane mode is on \a enabled is true; otherwise false.
 */

/*!
  \fn void QPhoneProfileManager::activeProfileChanged(const QPhoneProfile &profile)

  This signal is emitted whenever the active \a profile changes or is updated.
  */

/*!
  \fn void QPhoneProfileManager::profileUpdated(const QPhoneProfile &profile)

  This signal is emitted whenever \a profile is updated.
 */

/*!
  \fn void QPhoneProfileManager::profileAdded(const QPhoneProfile &profile)

  This signal is emitted when a new \a profile is added.
 */

/*!
  \fn void QPhoneProfileManager::profileRemoved(const QPhoneProfile &profile)

  This signal is emitted when \a profile is removed.
 */

