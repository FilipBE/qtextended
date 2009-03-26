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

#include "gsmkeyfilter.h"
#include <qslotinvoker.h>
#include <QStringList>

/*!
    \class GsmKeyFilter
    \inpublicgroup QtCellModule
    \internal
    \brief The GsmKeyFilter class filters key sequences looking for GSM control actions.

    The GsmKeyFilter class filters key sequences looking for GSM control
    actions.  The following standard control actions are recognized by
    this class:

    \table
    \row \o \c{0 SEND}
         \o Set the busy state for a waiting call, or release the held calls.
    \row \o \c{1 SEND}
         \o Release the active calls.
    \row \o \c{1n SEND}
         \o Release only call \c{n}.
    \row \o \c{2 SEND}
         \o Swap the active and held calls.
    \row \o \c{2n SEND}
         \o Activate only call \c{n}, putting all others on hold.
    \row \o \c{3 SEND}
         \o Join the active and held calls into a multi-party conversation.
    \row \o \c{4 SEND}
         \o Join the active and held calls into a multi-party conversation
            and then detach the local user (i.e. transfer).
    \row \o \c{4*n SEND}
         \o Deflect the waiting call to the number \c{n}.
    \endtable

    Additional actions can be added with addAction().
  
    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    \enum GsmKeyFilter::Flag
    Flags that modify the filtering of keys for GSM control actions.

    \value Send The GSM \c SEND function has been selected.  Usually this
           is the call button on the keypad.
    \value Immediate The action should be taken immediately when the last
           character is matched.  This is for sequences such as \c{*#06#}
           which must activate when the final \c{#} is pressed.
    \value OnCall The digits were entered when there was an
           active or held call.
    \value Incoming The digits were entered when there was an incoming
           call that has not yet been accepted.
    \value BeforeDial The digits were encountered just before the dial
           attempt was made.  The digits may have originated from a dialer
           or from a contact.  This provides a last chance to intercept a
           string of digits before regular dialing occurs.
    \value TestOnly Test to see if the filter would suceed, but do not apply.
*/

/*!
    \enum GsmKeyFilter::ServiceAction
    Action types for supplementary services.

    \value Activate Request to activate a supplementary service.
    \value Deactivate Request to deactivate a supplementary service.
    \value Interrogate Interrogate the current state of a supplementary service.
    \value Registration Register for a supplementary service.
    \value Erasure Erase a supplementary service.
*/

class GsmKeyFilterPrivate
{
public:
    struct Filter
    {
        QRegExp             regex;
        QSlotInvoker       *invoker;
        GsmKeyFilter::Flags flags;
        bool                service;
        GsmKeyFilter::ServiceAction action;
    };

    QList<Filter> filters;
};

/*!
    Construct a GSM key filter and attach it to \a parent.
*/
GsmKeyFilter::GsmKeyFilter( QObject *parent )
    : QObject( parent )
{
    d = new GsmKeyFilterPrivate();

    // Add the standard call actions from GSM 02.30, section 4.5.5.
    addAction( "0", this, SIGNAL(setBusy()), Send | Incoming );
    addAction( QRegExp( "4\\*([0-9]+)" ), this, SIGNAL(deflect(QString)),
               Send | Incoming );
    addAction( "0", this, SIGNAL(releaseHeld()), Send | OnCall );
    addAction( "0", this, SIGNAL(releaseActive()), Send | Incoming );
    addAction( "1", this, SIGNAL(releaseAllAcceptIncoming()), Send | Incoming );
    addAction( "1", this, SIGNAL(releaseActive()), Send | OnCall );
    addAction( QRegExp( "1([1-9])" ), this, SLOT(releaseId(QString)),
               Send | OnCall );
    addAction( "2", this, SIGNAL(swap()), Send | OnCall );
    addAction( QRegExp( "2([1-9])" ), this, SLOT(activateId(QString)),
               Send | OnCall );
    addAction( "3", this, SIGNAL(join()), Send | OnCall );
    addAction( "4", this, SIGNAL(transfer()), Send | OnCall );
}

/*!
    Destroy this GSM key filter.
*/
GsmKeyFilter::~GsmKeyFilter()
{
    delete d;
}

/*!
    Filter the supplied string of \a digits to determine if it corresponds
    to a GSM control action.   The \a flags indicate the context within
    which \a digits should be interpreted.

    Returns false if the digit sequence is not recognized by the filter.
    The filter will be called again when there are more digits (or less
    if a backspace was encountered).

    Returns true if the digit sequence was recognized and acted upon.
    The caller should clear the digit buffer.
*/
bool GsmKeyFilter::filter( const QString& digits, Flags flags )
{
    // Scan the list of filters for a match.
    QList<GsmKeyFilterPrivate::Filter>::Iterator it;
    for ( it = d->filters.begin(); it != d->filters.end(); ++it ) {
        if ( (*it).regex.exactMatch( digits ) ) {
            if ( ( flags & TestOnly ) != 0 )
                return true;
            if ( ( (*it).flags & ( flags & ~TestOnly ) ) == (*it).flags ) {
                QList<QVariant> args;
                QStringList captures = (*it).regex.capturedTexts();
                if ( !(*it).service ) {
                    if ( captures.size() > 1 )
                        args += QVariant( captures[1] );
                    else
                        args += QVariant( digits );
                } else {
                    args += qVariantFromValue( (*it).action );
                    args += captures[1].split( QChar('*') );
                }
                (*it).invoker->invoke( args );
                return true;
            }
        }
    }
    return false;
}

/*!
    Add \a digits as an action which will cause \a slot to be invoked
    on \a target when the specified string of \a digits is entered.

    The \a flags modifies when the action will be accepted or ignored.
    For example, setting \c Send will require that the \c SEND key be
    pressed to terminate the sequence; setting \c Incoming will require
    that there be an incoming call.

    The prototype for \a slot should be \c{action()} or \c{action(QString)}
    where \c{action} is the name of the slot.  The digit sequence entered
    will be passed to the slot if it has a QString argument.

    Actions are matched in the order in which they were added.
*/
void GsmKeyFilter::addAction
    ( const QString& digits, QObject *target, const char *slot,
      GsmKeyFilter::Flags flags )
{
    addAction( QRegExp( QRegExp::escape(digits) ), target, slot, flags );
}

/*!
    Add \a regex as an action which will cause \a slot to be invoked
    on \a target when a string of digits is entered which matches \a regex.

    The \a flags modifies when the action will be accepted or ignored.
    For example, setting \c Send will require that the \c SEND key be
    pressed to terminate the sequence; setting \c Incoming will require
    that there be an incoming call.  All specified flags must be present.

    The prototype for \a slot should be \c{action()} or \c{action(QString)}
    where \c{action} is the name of the slot.  The digit sequence entered
    will be passed to the slot if it has a QString argument.

    If \a regex includes a capture specification between parentheses,
    then only that part of the digit string will be passed to the slot.
    For example, \c{4*([0-9]+)} indicates that only the digits after the
    \c{4*} prefix should be passed to the slot.

    Actions are matched in the order in which they were added.
*/
void GsmKeyFilter::addAction
    ( const QRegExp& regex, QObject *target, const char *slot,
      GsmKeyFilter::Flags flags )
{
    GsmKeyFilterPrivate::Filter filter;
    filter.regex = regex;
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = false;
    filter.action = Activate;
    d->filters.append( filter );
}

/*!
    Add action filters for the supplementary service associated
    with \a code and call \a slot on \a target when the action
    is encountered.

    The prototype for \a slot should be
    \c{action(GsmKeyFilter::ServiceAction, QStringList)} where
    \c{action} is the name of the slot.  The first parameter will
    be the type of action to be taken, and the second parameter will
    be the list of \c{*}-separated parameters that were supplied
    to the action, starting with the service code.

    The \a flags modifies when the action will be accepted or ignored.
    For example, setting \c Send will require that the \c SEND key be
    pressed to terminate the sequence; setting \c Incoming will require
    that there be an incoming call.  All specified flags must be present.
*/
void GsmKeyFilter::addService
    ( const QString& code, QObject *target, const char *slot,
      GsmKeyFilter::Flags flags )
{
    GsmKeyFilterPrivate::Filter filter;
    QString regex = QChar('(') + QRegExp::escape(code) + "(\\*[0-9]*)*)#";

    filter.regex = QRegExp( "\\*" + regex);
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = true;
    filter.action = Activate;
    d->filters.append( filter );

    filter.regex = QRegExp( QChar('#') + regex );
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = true;
    filter.action = Deactivate;
    d->filters.append( filter );

    filter.regex = QRegExp( "\\*#" + regex );
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = true;
    filter.action = Interrogate;
    d->filters.append( filter );

    filter.regex = QRegExp( "\\*\\*" + regex );
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = true;
    filter.action = Registration;
    d->filters.append( filter );

    filter.regex = QRegExp( "##" + regex );
    filter.invoker = new QSlotInvoker( target, slot, this );
    filter.flags = flags;
    filter.service = true;
    filter.action = Erasure;
    d->filters.append( filter );
}

/*!
    \fn void GsmKeyFilter::setBusy()

    Signal that is emitted when the \c{0 SEND} key sequence is encountered
    when there is an incoming waiting call.
*/

/*!
    \fn void GsmKeyFilter::releaseHeld()

    Signal that is emitted when the \c{0 SEND} key sequence is encountered
    while on a call and there is no incoming waiting call.  This will release
    the held calls, if any.
*/

/*!
    \fn void GsmKeyFilter::releaseActive()

    Signal that is emitted when the \c{1 SEND} key sequence is encountered
    while on a call.  This will release the active calls, and accept the
    waiting call if there is one.
*/

/*!
    \fn void GsmKeyFilter::releaseAllAcceptIncoming()

    Signal that is emitted when the \c{1 SEND} key sequence is encountered
    when there is an incoming call but no active calls.  This will accept
    the incoming call.
*/

/*!
    \fn void GsmKeyFilter::release( int call )

    Signal that is emitted when the \c{1X SEND} key sequence is encountered
    while on a call, where \c X is the \a call identifier.  This will release
    the indicated call.
*/

/*!
    \fn void GsmKeyFilter::activate( int call )

    Signal that is emitted when the \c{2X SEND} key sequence is encountered
    while on a call, where \c X is the \a call identifier.  This will activate
    the indicated call, putting other calls on hold.
*/

/*!
    \fn void GsmKeyFilter::swap()

    Signal that is emitted when the \c{2 SEND} key sequence is encountered
    while on a call, which indicates that the active and held calls should be
    swapped.  If there is only an active call, it should be put on hold.
    If there is only a held call, then it should be activated.  If there
    is a waiting call, it will be accepted.
*/

/*!
    \fn void GsmKeyFilter::join()

    Signal that is emitted when the \c{3 SEND} key sequence is encountered
    while on a call, which indicates that the active and held calls should
    be joined together into a single multi-party call.
*/

/*!
    \fn void GsmKeyFilter::transfer()

    Signal that is emitted when the \c{4 SEND} key sequence is encountered
    while on a call, which indicates that the active and held calls should
    be joined together into a single multi-party call, and then the local
    user should be detached.
*/

/*!
    \fn void GsmKeyFilter::deflect( const QString& number )

    Signal that is emitted when the \c{4*NUM SEND} key sequence is encountered
    when there was an incoming call, which indicates that the incoming call
    should be deflected to \c NUM.  The \c NUM value is supplied in the
    \a number parameter to the signal.
*/

void GsmKeyFilter::releaseId( const QString& id )
{
    emit release( id.toInt() );
}

void GsmKeyFilter::activateId( const QString& id )
{
    emit activate( id.toInt() );
}

Q_IMPLEMENT_USER_METATYPE_ENUM(GsmKeyFilter::ServiceAction);
