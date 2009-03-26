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

#ifndef BASICHOMESCREEN_H
#define BASICHOMESCREEN_H

#include "qabstracthomescreen.h"
#include <QPixmap>
#include <QPointer>

#include "phonelock.h"
#include "pressholdgate.h"

class QMenu;

class BasicHomeScreen : public QAbstractHomeScreen
{
    Q_OBJECT

public:
    BasicHomeScreen(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~BasicHomeScreen();

    bool locked() const;
#ifdef QTOPIA_CELL
    bool simLocked() const;
#endif
    void setLocked(bool);

private slots:
#ifdef QTOPIA_TELEPHONY
    void setMissedCalls(int);
#endif

signals:
    void showLockDisplay(bool enable, const QString &pix, const QString &text);

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void inputMethodEvent(QInputMethodEvent *);
    bool eventFilter(QObject *, QEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

protected Q_SLOTS:
    void showLockInformation();
    void setContextBarLocked(bool locked, bool waiting = false);
    void activateSpeedDial();
    void specialButton(int keycode, bool held);
    void showProfileSelector();
    void lockScreen();
    void screenUnlocked();

#ifdef QTOPIA_TELEPHONY
    void phoneStateChanged();
    void viewNewMessages();
    void viewMissedCalls();
    void newMessagesChanged();
    void smsMemoryFullChanged();
#endif

protected:
#ifdef QTOPIA_CELL
    BasicEmergencyLock  *emLock;
    BasicSimPinLock     *simLock;
#endif

#ifdef QTOPIA_TELEPHONY
    QAction *actionMessages;
    QAction *actionCalls;
#endif

    BasicKeyLock    *keyLock;
    QAction         *actionLock;
    QTimer          *speeddialTimer;
    QString         speeddial;
    QString         speeddial_activated_preedit;
    PressHoldGate   *ph;
    QMenu           *m_contextMenu;
    int             lockMsgId;
    int             missedCalls;
    int             speeddial_preedit;
    bool            speeddialdown;
    int             newMessages;
    bool            smsMemoryFull;
    bool            screenLocked;
    QValueSpaceItem* newMsgVsi;
    QValueSpaceItem* smsMemFull;
    QPointer<QDialog> touchLockScreen;
};

#endif
