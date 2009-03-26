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

#include "qwhereabouts.h"

#include <QSettings>
#include <QStringList>

class QWhereaboutsPrivate
{
public:
    QWhereabouts::UpdateMethods methods;
    QWhereabouts::State state;
    int interval;
    QWhereaboutsUpdate lastUpdate;
};


/*!
    \class QWhereabouts
    \inpublicgroup QtLocationModule
    \ingroup whereabouts
    \brief The QWhereabouts class is a base class for providing regular updates about one's global position, and other related information, at a particular point in time.

    Call QWhereaboutsFactory::create() to get a QWhereabouts object, then
    call startUpdates() or requestUpdate() to receive positional data through the
    updated() signal.

    An example:

    \quotefromfile whereabouts/simpledemo/main.cpp
    \skipto class SimpleLocationDemo
    \printuntil };


    \section1 Subclassing QWhereabouts

    When startUpdates() or requestUpdate() is first called, a QWhereabouts subclass
    instance should change state to QWhereabouts::Initializing. If
    initialization succeeds, the state should change to
    QWhereabouts::Available, and then to QWhereabouts::PositionFixAcquired
    once a position fix is acquired and updates are possible. Otherwise, if
    initialization fails, the state should return to
    QWhereabouts::NotAvailable.

    Note that updated() should only be emitted while the state is
    QWhereabouts::PositionFixAcquired, and the provided update should always
    have valid date & time and coordinate values. Receivers of the signal should
    be able to assume that the provided update is valid without additional
    verification.

    Subclasses can call emitUpdated() to set the value of lastUpdate() and
    emit updated().

    \sa QWhereaboutsFactory, QWhereaboutsPlugin, {Location Services}
*/

/*!
    \enum QWhereabouts::UpdateMethod
    This enum defines the methods that may be used to calculate and retrieve
    the information to provide a QWhereaboutsUpdate.

    \value AssistedUpdate Position data is calculated using assistance data from an AGPS module. (This is generally used to obtain faster and more accurate fixes.)
    \value NetworkBasedUpdate The position data is calculated by a remote server.
    \value TerminalBasedUpdate The position data is calculated by the terminal (i.e. the mobile device).
*/

/*!
    \enum QWhereabouts::State
    This enum defines the possible states for a whereabouts object.

    \value NotAvailable Positional data cannot be retrieved. This is the
    default state.
    \value Initializing The object is in the initialization stage. It will change to the Available state when it is successfully initialized.
    \value Available The object is able to receive position data, but it is not currently able to provide position updates as there is no position fix.
    \value PositionFixAcquired A position fix has been acquired and position updates can now be provided.
*/

/*!
    Constructs a whereabouts object with the given \a updateMethods and \a parent.
*/
QWhereabouts::QWhereabouts(UpdateMethods updateMethods, QObject *parent)
    : QObject(parent),
      d(new QWhereaboutsPrivate)
{
    d->methods = updateMethods;
    d->state = QWhereabouts::NotAvailable;
    d->interval = 0;
}

/*!
    Destroys the whereabouts object.
*/
QWhereabouts::~QWhereabouts()
{
    delete d;
}

/*!
    Returns the methods that this whereabouts object uses to retrieve
    positional data.
*/
QWhereabouts::UpdateMethods QWhereabouts::updateMethods() const
{
    return d->methods;
}

/*!
    \overload

    Starts providing updates with an update interval of \a msec milliseconds.
*/
void QWhereabouts::startUpdates(int msec)
{
    setUpdateInterval(msec);
    startUpdates();
}

/*!
    Sets the current state to \a state and emits stateChanged() if
    necessary.
*/
void QWhereabouts::setState(State state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
    }
}

/*!
    Returns the current state.
*/
QWhereabouts::State QWhereabouts::state() const
{
    return d->state;
}

/*!
    \property QWhereabouts::updateInterval
    \brief This property holds the interval between each update, in milliseconds.

    If setUpdateInterval() is called after startUpdates(), the interval change
    will not take effect until startUpdates() is called again.

    The default update interval is 0. An update interval of 0 means that
    updated() is emitted as soon as a valid update becomes available.
*/
void QWhereabouts::setUpdateInterval(int interval)
{
    d->interval = interval;
}

int QWhereabouts::updateInterval() const
{
    return d->interval;
}

/*!
    Returns the last update that was emitted through updated(), or an invalid
    update if no updates have been emitted yet.

    The returned update may contain out-of-date information if updated() has
    not been emitted for some time. Use requestUpdate() if you require fresh
    positional data.

    \sa startUpdates(), emitUpdated()
*/
QWhereaboutsUpdate QWhereabouts::lastUpdate() const
{
    return d->lastUpdate;
}

/*!
    Sets the value of lastUpdate() to \a update and emits updated().

    Note that updated() should only be emitted while the state is
    QWhereabouts::PositionFixAcquired, and the provided update should always
    have valid date/time and coordinate values. Receivers of the signal should
    be able to assume that the provided update is valid without additional
    verification.
*/
void QWhereabouts::emitUpdated(const QWhereaboutsUpdate &update)
{
    d->lastUpdate = update;
    emit updated(update);
}

/*!
    \fn void QWhereabouts::startUpdates()

    Starts emitting updated() with the interval specified by updateInterval(),
    or less frequently if updates are not available at a particular time.
    If updateInterval() is 0, updated() is emitted as soon as a valid update
    becomes available.

    If startUpdates() has already been called, this restarts with updateInterval()
    as the new update interval.

    If the state is currently QWhereabouts::NotAvailable, the object will
    first be initialized and will begin to provide updates once it is in the
    QWhereabouts::PositionFixAcquired state.

    \sa stopUpdates()
*/

/*!
    \fn void QWhereabouts::stopUpdates()

    Stops emitting updated() at regular intervals.

    \sa startUpdates()
*/

/*!
    \fn void QWhereabouts::requestUpdate()

    Requests that updated() be emitted with the current whereabouts if
    possible. This can be called regardless of whether startUpdates() has already
    been called.

    This is useful if you need to retrieve the current whereabouts but you
    do not need the periodic updates offered by startUpdates().

    If the state is currently QWhereabouts::NotAvailable, the object will
    first be initialized and will begin to provide updates once it is in the
    QWhereabouts::PositionFixAcquired state.

    \sa startUpdates(), lastUpdate()
*/


/*!
    \fn void QWhereabouts::updated(const QWhereaboutsUpdate &update)

    This signal is emitted to notify recipients of a newly available
    \a update.

    \sa startUpdates(), stopUpdates(), requestUpdate()
*/

/*!
    \fn void QWhereabouts::stateChanged(QWhereabouts::State state)

    This signal is emitted when the whereabouts object's state changes to
    \a state.

    \sa setState()
*/

