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

#ifndef DIALPROXY_H
#define DIALPROXY_H

#include "dialerservice.h"
#include "qtopiaserverapplication.h"

#include <QServiceNumbers>
#include <QUniqueId>

class QAbstractMessageBox;
class AbstractAudioHandler;
class DialProxy : public DialerService
{
    Q_OBJECT
public:
    enum CallScreenTrigger
    {
        CallDialing = 0x01,
        CallIncoming = 0x02,
        CallAccepted = 0x04,
        CallDropped = 0x08
    };
    Q_DECLARE_FLAGS(CallScreenTriggers, CallScreenTrigger)

    DialProxy(QObject *parent = 0);
    ~DialProxy();

    void setCallScreenTriggers(CallScreenTriggers triggers);
    CallScreenTriggers callScreenTriggers() const;

public slots:
    void dialVoiceMail();
    void dial( const QString& name, const QString& number );
    void dial( const QString& number, const QUniqueId& contact );
    void showDialer( const QString& digits );
    void onHook();
    void offHook();
    void headset();
    void speaker();
    void setDialToneOnlyHint( const QString &app );
    void redial();

    void acceptIncoming();
    bool hangupPressed();
    void requestDial(const QString &n, const QUniqueId &c = QUniqueId());

signals:
    void doShowDialer(const QString& number);
    void showCallScreen();
    void resetScreen();
    void onHookGesture();
    void offHookGesture();

private slots:
    void serviceNumber(QServiceNumbers::NumberId id, const QString& number);
    void messageBoxDone(int);
    void stateChanged();

private:
    void dialNumber(const QString &n, const QUniqueId &c, const QString &callType);
    void showWarning(const QString &title, const QString &text);
    void processAudioKey(int key, bool isPress);

    bool dialerVisible() const;
    bool callHistoryIsActiveWindow() const;

    bool waitingVoiceMailNumber;
    QServiceNumbers *serviceNumbers;
    QAbstractMessageBox *noVoiceMailNumberMsgBox;
    QAbstractMessageBox *warningMsgBox;
    QAbstractMessageBox *voipNoPresenceMsgBox;
    QAbstractMessageBox *dialingMsgBox;

    bool queuedIncoming;
    QAbstractMessageBox *incomingMsgBox;

    QString queuedCall;
    QString queuedCallType;
    QUniqueId queuedCallContact;

    CallScreenTriggers m_callScreenTriggers;
    AbstractAudioHandler *m_dialtoneAudio;
    AbstractAudioHandler *m_callAudio;
    QStringList dialToneOnlyHintApps;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DialProxy::CallScreenTriggers);
QTOPIA_TASK_INTERFACE(DialProxy);

#endif
