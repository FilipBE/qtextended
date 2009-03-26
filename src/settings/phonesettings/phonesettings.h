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
#ifndef PHONESETTINGS_H
#define PHONESETTINGS_H

#include "ui_channeledit.h"

#include <qcallsettings.h>
#include <qcallbarring.h>
#include <qcellbroadcast.h>
#include <qtelephonynamespace.h>
#include <qphonebook.h>
#include <qservicenumbers.h>

#include <QDialog>
#include <QIcon>
#include <QListWidget>
#include <QtopiaAbstractService>

class QListWidgetItem;
class QCheckBox;
class QLineEdit;
class QTimer;
class QRadioButton;
class QWaitWidget;

class QCallVolume;

class FloatingTextList : public QListWidget
{
    Q_OBJECT

public:
    FloatingTextList( QWidget *parent, int w );
    void showItemText();

protected:
    void keyPressEvent( QKeyEvent *e );

protected slots:
    void newCurrentRow( int row );
    void floatText();

private:
    QTimer *timer;
    int availableWidth;
    static int lastCharIndex; // index of the last character shown
};

class PhoneSettings : public QDialog
{
    Q_OBJECT

public:
    PhoneSettings( QWidget *parent = 0, Qt::WFlags fl = 0 );
    enum { Barring, Waiting, CallerId, Broadcast, Fixed, Flip, Service, Volume };

    void activate( int itemId );

    bool selectVoiceMail;

private slots:
    void itemActivated( QListWidgetItem *item );

private:
    void init();

private:
    QListWidget *optionList;
};

class CallBarring : public QDialog
{
    Q_OBJECT

public:
    CallBarring( QWidget *parent = 0, Qt::WFlags fl = 0 );

protected:
    void showEvent( QShowEvent *e );

private:
    void init();
    void updateMenu();

private slots:
    void itemActivated( QListWidgetItem * item );
    void barringStatus( QCallBarring::BarringType, QTelephony::CallClass );
    void setBarringStatusResult( QTelephony::Result );
    void changeBarringPasswordResult( QTelephony::Result );
    void unlockResult( QTelephony::Result );
    void unlockAll();
    void changePin();
    void checkStatus();

private:
    FloatingTextList *barOptions;
    QCallBarring *client;
    QIcon incoming, outgoing, barred;
    QAction *unlock, *pin;
    bool isLoading;
    QWaitWidget *waitWidget;
};

class CallWaiting : public QDialog
{
    Q_OBJECT

public:
    CallWaiting( QWidget *parent = 0, Qt::WFlags fl = 0 );

protected:
    void showEvent( QShowEvent *e );

private:
    void init();

private slots:
    void itemActivated( QListWidgetItem * item );
    void callWaiting( QTelephony::CallClass );
    void setCallWaitingResult( QTelephony::Result );
    void checkStatus();

private:
    QListWidget *waitOptions;
    QCallSettings *client;
    bool isLoading;
    QWaitWidget *waitWidget;
};

class CallerID : public QDialog
{
    Q_OBJECT
public:
    CallerID( QWidget *parent = 0, Qt::WFlags fl = 0 );

private:
    void init();
    void accept();
    QString formatString( QString str );

private slots:
    void activate();
    void selectContactCategory();
    void callerIdRestriction( QCallSettings::CallerIdRestriction,
            QCallSettings::CallerIdRestrictionStatus );

private:
    QRadioButton *toNone, *toAll, *opDefault, *toContact;
    QCallSettings *client;
    int choice;
};

class CellBroadcasting : public QDialog
{
    Q_OBJECT

public:
    CellBroadcasting( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CellBroadcasting();

    enum Mode { Background, Foreground };
    struct Channel {
        int num;
        QString label;
        Mode mode;
        bool active;
        QStringList languages;
    };

private:
    void init();
    void activate();
    void readConfig();
    void writeConfig();
    void accept();

private slots:
    bool edit();
    void add();
    void remove();
    void itemActivated( QListWidgetItem * item );

private:
    QList<Channel *> channels;
    QListWidget *channelList;
    QAction *actionEdit, *actionAdd, *actionRemove;
    QCellBroadcast *client;
};

class CellBroadcastEditDialog : public QDialog
{
    Q_OBJECT

public:
    CellBroadcastEditDialog( QWidget *parent, Qt::WFlags f = 0 );
    void setChannel( CellBroadcasting::Channel c );
    CellBroadcasting::Channel channel() const;

protected:
    void accept();
    void setLanguages();

private slots:
    void selectLanguages();
    void itemActivated( QListWidgetItem *item );

private:
    CellBroadcasting::Channel ch;
    QListWidget *lstLang;
    Ui::ChannelEdit *editor;
};

class FixedDialing : public QDialog
{
    Q_OBJECT

public:
    FixedDialing( QWidget *parent = 0, Qt::WFlags fl = 0 );

private:
    void init();

private slots:
    void setFixedDialing( bool enabled );
    void fixedDialingState( bool );
    void setFixedDialingStateResult( QTelephony::Result );
    void phoneBookEntries( const QString& pbook, const QList<QPhoneBookEntry>& entries );
    void phonebookLimits(const QString&,const QPhoneBookLimits&);
    void add();
    void remove();

private:
    QCheckBox *active;
    FloatingTextList *allowedNumbers;
    QPhoneBook *phonebook;
    QAction *actionAdd, *actionRemove;
    int limit;
    QString pin2;
};

class FlipFunction : public QDialog
{
    Q_OBJECT

public:
    FlipFunction( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~FlipFunction();

private:
    void init();
    void readConfig();
    void writeConfig();

private:
    QCheckBox *answer, *hangup;
};

class ServiceNumbers : public QDialog
{
    Q_OBJECT

public:
    ServiceNumbers( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~ServiceNumbers();

    void selectVoiceMail() { focusVoiceMail = true; }

private:
    void init();

protected:
    void accept();

private slots:
    void serviceNumber( QServiceNumbers::NumberId id, const QString& number );

private:
    QLineEdit *voiceMail, *serviceCenter;
    QString init_voicenum, init_smsnum;
    QServiceNumbers *serviceNumbers;
    bool focusVoiceMail;
};

class CallVolume : public QDialog
{
    Q_OBJECT

public:
    CallVolume( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallVolume();

private:
    void init();

protected:
    void accept();
    void reject();

private slots:
    void speakerSliderChanged(int volume);
    void speakerVolumeChanged(int volume);
    void microphoneSliderChanged(int volume);
    void microphoneVolumeChanged(int volume);

private:
    QSlider *speakerVolume;
    QSlider *microphoneVolume;

    int m_oldSpeakerVolume;
    int m_oldMicrophoneVolume;

    bool m_changeSpeakerVolume;
    bool m_changeMicrophoneVolume;

    QCallVolume *callVolume;
};

class VoiceMailService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PhoneSettings;
private:
    VoiceMailService( PhoneSettings *parent )
        : QtopiaAbstractService( "VoiceMail", parent )
    { this->parent = parent; publishAll(); }

public:
    ~VoiceMailService();

public slots:
    void setVoiceMail();

private:
    PhoneSettings *parent;
};

#endif
