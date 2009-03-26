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

#ifndef RINGPROFILE_H
#define RINGPROFILE_H

#include <QListWidget>
#include <QDialog>
#include <QMap>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QtopiaAbstractService>

class QAction;
class QGridLayout;
class QTabWidget;
class QLabel;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
#if defined(QTOPIA_TELEPHONY)
class RingToneButton;
#endif
class QTimeEdit;
class QPushButton;
class QSlider;
class PhoneProfileItem;
class QMouseEvent;

class ProfileEditDialog : public QDialog
{
    Q_OBJECT
public:
    ProfileEditDialog(QWidget *parent, bool isnew);
    ~ProfileEditDialog();
    void setProfile(PhoneProfileItem *);
    void setActive( bool b ) { isActive = b; };
    void addSetting( const QPhoneProfile::Setting s );

protected:
    void accept();
    bool event(QEvent *);

private:
    void setPhoneProfile();
    void setSettings();
    void setSchedule();
    void initDialog( QDialog *dlg );
    void processSchedule();
    bool eventFilter( QObject *o, QEvent *e );
    bool showSettingList( QStringList &settingList );

private slots:
#if defined(QTOPIA_TELEPHONY)
    void updateState();
    void toneSelected(const QContent& tone);
#endif
    void viewSetting();
    void deleteSetting();
    void setSoftMenu(int);
    void tabChanged(int);
    void showEditScheduleDialog();
    void enableEditSchedule(bool);
    void pullSettingStatus();

private:
    QTabWidget *tabWidget;
    QWidget *infoTab, *settingTab;
#if defined(QTOPIA_TELEPHONY)
    QWidget *toneTab;
#endif

    QLineEdit *profileName;
    QSlider *masterVolume;
#if defined(QTOPIA_TELEPHONY)
    QCheckBox *autoAnswer, *vibrateAlert;
    QComboBox *videoOption;
#endif
    QCheckBox *autoActivation;
    QPushButton *editSchedule;
    QTimeEdit *time;
    QCheckBox **days;

#if defined(QTOPIA_TELEPHONY)
    QComboBox *ringType;
    RingToneButton *ringTone;
    RingToneButton *videoTone;

    QComboBox *messageType;
    QSpinBox *messageAlertDuration;
    RingToneButton *messageTone;
#endif

    QListWidget *settingListWidget;

    PhoneProfileItem *profile;
    QPhoneProfile::Settings settings;
    int id;
    bool isActive;
    bool isLoading;
    bool deleteDays;
    bool editVolume;
    QPhoneProfile::Schedule schedule;

    QAction *actionView;
    QAction *actionDelete;
    QAction *actionCapture;
};

class PhoneProfileItem : public QListWidgetItem {
public:
    PhoneProfileItem(const QPhoneProfile &pr, QListWidget *l = 0);

    const QPhoneProfile &profile() const { return rpp; }
    QPhoneProfile &profile() { return rpp; }

    int width( const QListWidget* lb ) const;
    int height( const QListWidget* lb ) const;

private:
    QIcon cache;
    QPhoneProfile rpp;
};

class ActiveProfileDisplay;
class ProfileSelect : public QDialog
{
    Q_OBJECT
    friend class ProfilesService;

public:
    ProfileSelect( QWidget *parent = 0, Qt::WFlags f = 0, const char *name = 0);
    ~ProfileSelect();

public slots:
    void appMessage(const QString &msg, const QByteArray &data);
    void activatePlaneMode();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void activateProfile();
    void activateProfile(int);
    void activateProfile(QListWidgetItem *);
    void itemActivated(QListWidgetItem *);
    void itemPressed(QListWidgetItem *);
    void editProfile();
    void editProfile(QListWidgetItem *);
    void editProfile(PhoneProfileItem *);
    void createNewProfile();
    void removeCurrentProfile();
    void setPlaneMode(bool);
    void addToFavorites();
    void updateIcons();

private:
    void setActiveProfile( PhoneProfileItem *pItem, bool = false );
    void loadConfig();
    void capture();
    QString findUniqueName( const QString &name, int curIndex );
    void makeProfileNameUnique( PhoneProfileItem * );

    QAction *actionNew, *actionEdit, *actionRemove, *actionActivate, *actionFavorites;
    QAction *actionPlane;
    bool isLoading;
    QMenu *rightClickMenu;

    QListWidget *itemlist;
    ActiveProfileDisplay *activeDisplay;
    ProfileEditDialog *editDialog;

    PhoneProfileItem *activeProfile;

    QPhoneProfileManager profMan;

    bool origPlaneMode;
    QString appName, appTitle, details;
};

class ProfilesService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ProfileSelect;
private:
    ProfilesService( ProfileSelect *parent )
        : QtopiaAbstractService( "Profiles", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ProfilesService();

public slots:
    void activatePlaneMode();
    void showProfiles();
    void setProfile( int id );

private:
    ProfileSelect *parent;
};

#endif
