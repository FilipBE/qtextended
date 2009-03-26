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

#include "qtopiatimer.h"
#include <QValueSpaceItem>
#include <QDate>
#include <QTime>

struct QtopiaTimerPrivate
{
    QtopiaTimerPrivate() : 
        type(QtopiaTimer::Normal), 
        interval(0), active(false), timerId(0), singleShot(false), elapsedInterval(0), item(0) {}

    QtopiaTimer::Type type;

    int interval;
    bool active;
    int timerId;

    bool singleShot;

    // Applies only to pause timers
    QDate pDate;
    QTime pTime;
    int elapsedInterval;
    int runningInterval;

    QValueSpaceItem *item;
};

/*!
  \class QtopiaTimer
    \inpublicgroup QtBaseModule

  \brief The QtopiaTimer class provides timers that can pause automatically when the screen
  is off.

  The use of this class is similar to QTimer as the API is the same, except setInterval() and start()
  now have an optional argument that is the type() of pausing behavior to use.

  When the type is PauseWhenInactive, the timer will pause while the screen is off.
  This allows applications to more easily suspend repetitive processing when the user is
  not able to interact with the application.

  \ingroup time
  \sa QTimer
*/

/*!
  Creates a new QtopiaTimer instance with the specified \a parent.
*/
QtopiaTimer::QtopiaTimer(QObject *parent)
: QObject(parent), d(0)
{
    d = new QtopiaTimerPrivate;
}

/*!
  Destroys the QtopiaTimer instance.
*/
QtopiaTimer::~QtopiaTimer()
{
    delete d;
    d = 0;
}

/*!
  Returns the type of the timer.
  \sa QtopiaTimer::Type, setInterval()
*/
QtopiaTimer::Type QtopiaTimer::type() const
{
    return d->type;
}

/*!
  Returns the number of milliseconds that the QtopiaTimer will wait. A value of 0 means that the timer
  will time out as soon as all the events in the window system event queue have been processed.
  \sa setInterval()
*/
int QtopiaTimer::interval () const
{
    return d->interval;
}

/*!
  Returns true if the timer is active; false otherwise.
  \sa start(), stop()
*/
bool QtopiaTimer::isActive () const
{
    return d->active;
}

/*!
  Returns true if the timer is a single-shot timer; false otherwise.
  Single-shot timers do not restart automatically.
  \sa setSingleShot()
*/
bool QtopiaTimer::isSingleShot() const
{
    return d->singleShot;
}

/*!
  Sets the timer to be a single-shot timer if \a singleShot is true. If \a singleShot is false
  the timer will restart automatically.
  \sa isSingleShot()
*/
void QtopiaTimer::setSingleShot(bool singleShot)
{
    d->singleShot = singleShot;
}

/*!
  Sets the timer to wait \a msec milliseconds before triggering. The type is set to \a type.
  \sa interval(), type()
*/
void QtopiaTimer::setInterval(int msec, Type type)
{
    bool active = isActive();
    if(active)
        stop();
    d->type = type;
    d->interval = msec;
    if(active)
        start();
}

/*!
  Starts the timer using the previously-set values.
  \sa setInterval()
*/
void QtopiaTimer::start()
{
    stop();
    d->active = true;
    if(d->type == PauseWhenInactive) {
        if(!d->item) {
            d->item = new QValueSpaceItem("/Hardware/Display/0/Backlight", 
                                          this);
            QObject::connect(d->item, SIGNAL(contentsChanged()), 
                             this, SLOT(activeChanged()));
        }

        disable(true);
        activeChanged();

    } else {
        d->timerId = startTimer(d->interval);
        d->pTime.start();
        d->elapsedInterval = 0;
    }
}

/*!
  Stops the timer.
  \sa isActive()
*/
void QtopiaTimer::stop()
{
    if(d->timerId) {
        killTimer(d->timerId);
        d->timerId = 0;
    }
    d->active = false;
}

/*!
  Starts the timer with a new interval \a msec and \a type.
  \sa setInterval(), start()
*/
void QtopiaTimer::start(int msec, Type type)
{
    stop();
    setInterval(msec, type);
    start();
}

/*!
  \internal
  Reacts to a timeout.
*/
void QtopiaTimer::timerEvent(QTimerEvent *)
{
    if(PauseWhenInactive == type() && d->interval != d->runningInterval) {
        if(d->timerId)
            killTimer(d->timerId);
        d->timerId = startTimer(d->interval);
        d->pTime.start();
        d->elapsedInterval = 0;
        d->runningInterval = d->interval;
    }

    if(d->singleShot)
        stop();

    emit timeout();
}

/*!
  \internal
  Disables the timer. If \a forceReset is set, forces the timer instance to be reset.
*/
void QtopiaTimer::disable(bool forceReset)
{
    Q_ASSERT(PauseWhenInactive == type() && isActive());
    if(d->timerId || forceReset) {
        if (d->timerId) {
            d->elapsedInterval += d->pTime.elapsed();
            killTimer(d->timerId);
        }
        d->timerId = 0;

        d->pDate = QDate::currentDate();
        d->pTime.start();
    }
}

/*!
  \internal
  Enables the timer.
*/
void QtopiaTimer::enable()
{
    if(d->timerId)
        return;

    Q_ASSERT(PauseWhenInactive == type() && isActive() && !d->timerId);

    // Do we need an immediate emit?
    int pausedTime = d->pTime.elapsed();
    if (d->pDate != QDate::currentDate() ||
        pausedTime + d->elapsedInterval >= d->interval) {
        // Yes!
        d->timerId = startTimer(d->interval);
        d->pTime.start();
        d->elapsedInterval = 0;
        d->runningInterval = d->interval;
        emit timeout();
    } else {
        // No
        d->elapsedInterval += pausedTime;

        int interval = d->interval - d->elapsedInterval;
        Q_ASSERT(interval > 0);

        d->timerId = startTimer(interval);
        d->pTime.start();
        d->runningInterval = interval;
    }
}

/*!
  \internal
  Reacts to a change in the system's state, pausing or resuming the timer if appropriate.
*/
void QtopiaTimer::activeChanged()
{
    if(type() == PauseWhenInactive && isActive()) {

        Q_ASSERT(d->item);
        bool backlight = (d->item->value(QByteArray(), 1).toInt() >= 1);

        if(backlight)
            enable();
        else
            disable(false);

    }
}

/*!
  \fn void QtopiaTimer::timeout()
  This signal is emitted when the timer interval has elapsed.
  Unless the timer is a single-shot timer, it will automatically restart and so trigger after
  the interval passes again.
  \sa start(), stop(), setSingleShot()
*/

/*!
  \enum QtopiaTimer::Type

  This enum type specifies the behavior that QtopiaTimer uses.

  \value Normal
         operate as a QTimer.
  \value PauseWhenInactive
         pause while the screen turns off.
*/

