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

#include "gsmfiltertask.h"
#include "gsmkeyactions.h"
#include "qtopiaserverapplication.h"

class GSMDialFilterPrivate
{
public:
    GSMDialFilterPrivate() 
        : gsmkeyActions(0)
    {
    }

    GsmKeyActions *gsmkeyActions;
};


/*!
    \class GSMDialFilter
    \inpublicgroup QtCellModule
    \brief The GSMDialFilter class provides a GSM dial filter required for GCF compliance.
    \ingroup QtopiaServer::Telephony

    This class filters key sequences looking for GSM control
    actions and handles special GSM actions for requesting and controlling supplementary services.
  
    The following standard control actions are recognized by
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


    The following GSM actions for requesting and controlling supplementary services are
    handled/supported:

    \table
    \row \o \c{*#06#} \o Display the IMEI serial number of the phone.
    \row \o \c{[*]*03*ZZ*OLD_PASSWORD*NEW_PASSWORD*NEW_PASSWORD#}
         \o Change the password for supplementary service \c{ZZ}.
    \row \o \c{**04*OLD_PIN*NEW_PIN*NEW_PIN#}
         \o Change the \c{SIM PIN} to a new value.
    \row \o \c{**042*OLD_PIN*NEW_PIN*NEW_PIN#}
         \o Change the \c{SIM PIN2} to a new value.
    \row \o \c{**05*PIN_UNBLOCKING_KEY*NEW_PIN*NEW_PIN#}
         \o Unblock the \c{SIM PIN} and change it to a new value.
    \row \o \c{**052*PIN_UNBLOCKING_KEY*NEW_PIN*NEW_PIN#}
         \o Unblock the \c{SIM PIN2} and change it to a new value.
    \row \o \c{*#30#}
         \o Query the state of the caller id presentation (CLIP) service.
    \row \o \c{*#31#}
         \o Query the state of the caller id restriction (CLIR) service.
    \row \o \c{*31#number}
         \o Dial \c{number} with caller id explicitly enabled.
    \row \o \c{#31#number}
         \o Dial \c{number} with caller id explicitly disabled.
    \row \o \c{*#76#}
         \o Query the state of the connected id presentation (COLP) service.
    \endtable

    Supplementary service actions that are not recognized are sent
    to the network for processing.

    This class is a Qt Extended server class and cannot be used by other Qt Extended Applications.

   \sa AbstractDialFilter
  */

/*!
  Creates a new GSMDialFilter instance with the given \a parent.
  */
GSMDialFilter::GSMDialFilter( QObject *parent )
    : AbstractDialFilter( parent ), d(new GSMDialFilterPrivate())
{
    d->gsmkeyActions = new GsmKeyActions( this );
}

/*!
  Destroys the GSMDialFilter instance.
  */
GSMDialFilter::~GSMDialFilter()
{
    if (d)
        delete d;
}

/*!
  \reimp
  */
AbstractDialFilter::Action GSMDialFilter::filterInput( const QString &input, 
        bool sendPressed, bool takeNoAction )
{
    bool filterable = false;
    if (takeNoAction) {
        d->gsmkeyActions->testKeys(input, filterable);
    } else if (sendPressed) {
        d->gsmkeyActions->filterSelect(input, filterable);
    } else {
        d->gsmkeyActions->filterKeys(input, filterable);
    }

    if ( filterable)
        return AbstractDialFilter::ActionTaken;
    else
        return AbstractDialFilter::Continue;
}

/*!
  \reimp
  This function always returns "GSM".
  */
QByteArray GSMDialFilter::type() const
{
    return QByteArray("GSM");
}

QTOPIA_TASK(GSMDialFilter,GSMDialFilter);
QTOPIA_TASK_PROVIDES(GSMDialFilter,AbstractDialFilter);
