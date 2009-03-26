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

#include "radioservice.h"
#include "radioplayer.h"

/*!
    \service RadioService Radio
    \brief Provides the Qtopia Radio service.

    The \i Radio service enables applications to control the radio
    tuner program.
*/

/*!
    \internal
*/
RadioService::RadioService( RadioPlayer *parent )
    : QtopiaAbstractService( "Radio", parent )
{
    publishAll();
    this->parent = parent;
}

/*!
    \internal
*/
RadioService::~RadioService()
{
}

/*!
    Turn on the radio mute.
*/
void RadioService::mute()
{
    parent->setMute( true );
}

/*!
    Turn off the radio mute.
*/
void RadioService::unmute()
{
    parent->setMute( false );
}

/*!
    Set the radio to tune in a specific station on \a band and \a frequency.
*/
void RadioService::setStation( const QString& band, qlonglong frequency )
{
    parent->setStation( band, frequency );
}
