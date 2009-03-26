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

#ifndef PHONELOCK_H
#define PHONELOCK_H

#include <QObject>
#include <QString>
#include <qvaluespace.h>
#include <QKeyEvent>
#ifdef QTOPIA_CELL
#include <qpinmanager.h>
#include "cellmodemmanager.h"
#endif

class BasicKeyLockPrivate;
class BasicKeyLock : public QObject
{
Q_OBJECT
public:
    BasicKeyLock(QObject *parent = 0);
    virtual ~BasicKeyLock();

    enum State { Open,
                 KeyLocked,
                 KeyLockIncorrect,
                 KeyLockToComplete };
    State state() const;

    bool locked() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);

    void setStateTimeout(int);
    int stateTimeout();

public slots:
    void reset();
    void lock();
    void unlock();

signals:
    void stateChanged(BasicKeyLock::State);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void stopTimer();
    void startTimer();
    void setState(State);
    BasicKeyLockPrivate *d;
};

#ifdef QTOPIA_CELL

class BasicEmergencyLockPrivate;
class BasicEmergencyLock : public QObject
{
    Q_OBJECT
public:
    BasicEmergencyLock(QObject *parent = 0);
    virtual ~BasicEmergencyLock();

    enum State { NoEmergencyNumber, 
                 PartialEmergencyNumber,
                 EmergencyNumber };
    State state() const;
    QString emergencyNumber() const;
    bool emergency() const;

    static Qt::Key lockKey();

    bool processKeyEvent(QKeyEvent *);

public slots:
    void reset();

signals:
    void stateChanged(BasicEmergencyLock::State, const QString &);
    void dialEmergency(const QString &);

private:
    BasicEmergencyLockPrivate *d;
};

class BasicSimPinLockPrivate;
class BasicSimPinLock : public QObject
{
Q_OBJECT
public:
    BasicSimPinLock(QObject *parent = 0);
    virtual ~BasicSimPinLock();

    enum State { Pending, Open, Waiting, SimPinRequired, VerifyingSimPin, SimPukRequired,
                 NewSimPinRequired, VerifyingSimPuk };

    State state() const;

    QString number() const;
    bool locked() const;

    static Qt::Key lockKey();
    void processKeyEvent(QKeyEvent *);

public slots:
    void reset();

signals:
    void stateChanged(BasicSimPinLock::State, 
                      const QString &);

private slots:
    void cellStateChanged(CellModemManager::State newState);

private:
    State stateFromCellState(CellModemManager::State);
    BasicSimPinLockPrivate *d;
};

#endif // QTOPIA_CELL

#endif
