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
#ifndef CALLFORWARDING_H
#define CALLFORWARDING_H

#include <qcallforwarding.h>
#include <qtelephonynamespace.h>
#include <qservicenumbers.h>
#include <QDialog>
#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <QValidator>

class QTabWidget;
class QContact;
class QMenu;
class QAction;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QWaitWidget;

class ContactSelectDialog : public QDialog
{
    Q_OBJECT

public:
    ContactSelectDialog( QWidget *parent );
    QString number() const;

private slots:
    void itemSelectionChanged();
    void selectedContact( QListWidgetItem *item );
    void deleteAll();
    void deleteNumber();

private:
    void init();
    void numberFromContacts();
    void numberFromInputLine();
    void addNumber( const QString& newNumber );
    void saveSettings();

private:
    QListWidget *contactList;

    QListWidgetItem *voiceMail;
    QListWidgetItem *findContact;
    QListWidgetItem *typeNumber;

    QMenu *contextMenu;
    QAction *actionDeleteNumber;
    QAction *actionDeleteAll;

    QStringList contacts;
    QString selectedNumber;
};

class CallForwardItem : public QCheckBox
{
    Q_OBJECT

public:
    CallForwardItem( QTelephony::CallClass c, QCallForwarding::Reason reason, QWidget *parent = 0 );
    QString selectedContact();
    void setStatus( const bool enabled, const QString number );
    QString status() const;
    void readSettings();
    void deactivate();
    void setForwardingResult( QTelephony::Result );

private:
    void init();
    QString selectContact();
    bool usePreviousNumber( bool & );
    QString conditionName();
    void keyPressEvent(QKeyEvent *ke);
    int selectOperationType();
    void setText( bool enabled, const QString &number = QString() );

signals:
    void sendRequest( QCallForwarding::Reason reason, QString number );
    void keyPressed( Qt::Key key );

private slots:
    void checked( bool on );

private:
    QTelephony::CallClass classX;
    QCallForwarding::Reason reason;
    QString forwardNumber, newNumber;
    bool statusUpdate, abort, currentStatus, newStatus;
};

class CallClassTab : public QWidget
{
    Q_OBJECT

public:
    CallClassTab( QTelephony::CallClass c, QWidget *mainWidget, QWidget *parent = 0, Qt::WFlags f = 0 );
    QString typeName();
    void deactivateAll();
    QTelephony::CallClass callClassX();
    void setStatus( QCallForwarding::Reason, const QCallForwarding::Status );
    QString status() const;
    void readSettings();

private:
    void init();
    void showEvent( QShowEvent *e );

signals:
    void sendRequest( QCallForwarding::Reason reason, QString number, QTelephony::CallClass c );

public slots:
    void alwaysChecked( const bool on );
    void receiveRequest( const QCallForwarding::Reason reason, const QString& number );
    void setForwardingResult( QCallForwarding::Reason, QTelephony::Result );

friend class CallForwarding;

private:
    QTelephony::CallClass classX;

    CallForwardItem *alwaysItem;
    CallForwardItem *busyItem;
    CallForwardItem *unansweredItem;
    CallForwardItem *unavailableItem;
};

class CallForwarding : public QDialog
{
    Q_OBJECT

public:
    CallForwarding( QWidget *parent = 0, Qt::WFlags f = 0 );

    struct ForwardItem { QCallForwarding::Reason reason; bool enabled; QTelephony::CallClass classx; QString number; };
    enum RequestType { NoRequest, Activation, Deactivation, Status };

protected:
    void accept();

private:
    void init();
    QString status();
    void setStatus( const QString details );
    void activate( const QString details );
    void pullSettingStatus();

private slots:
    void deactivateAll();
    void receive( const QString& msg, const QByteArray& data );
    void receiveRequest( const QCallForwarding::Reason reason, const QString number, const QTelephony::CallClass c );
    void forwardingStatus( QCallForwarding::Reason, const QList<QCallForwarding::Status>& );
    void setForwardingResult( QCallForwarding::Reason, QTelephony::Result );
    void checkStatus();
    void pushSettingStatus();
    void serviceNumber( QServiceNumbers::NumberId, const QString& );

private:
    QCallForwarding *client;

    QTabWidget *tabWidget;

    CallClassTab *voiceTab, *dataTab, *faxTab, *smsTab;
    ForwardItem currentItem;

    QMenu *contextMenu;
    QAction *actionDeactivateAll;
    QAction *actionCheckStatus;
    QAction *actionCapture;

    QWaitWidget *splash;
    RequestType reqType;
    bool isAutoActivation, isStatusView, isFromActiveProfile, isLoaded;

    QList<ForwardItem> forwardItemList;
};

#endif
