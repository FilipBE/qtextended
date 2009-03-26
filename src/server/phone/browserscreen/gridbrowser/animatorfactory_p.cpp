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

#include "animatorfactory_p.h"
#include "animator_p.h"

#include <QString>
#include "bouncer_p.h"
#include "zoomer_p.h"
#include "shearer_p.h"
#include "rotator_p.h"
#include "radialbackground_p.h"


/*!
  \internal
  \class AnimatorFactory
    \inpublicgroup QtBaseModule

  \brief Creates Animator objects, based on a textual description.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/


/*!
  \internal
  \fn Animator *AnimatorFactory::animator(const QString &description)
   Creates and returns an Animator which corresponds to the given description,
   or 0 if no Animator class corresponds to that description. Note that it
   is the calling code's responsibility to delete the returned object.
*/
Animator *AnimatorFactory::animator(const QString &description)
{
    if ( description == Bouncer::description() ) {
        return new Bouncer;
    }
    if ( description == Zoomer::description() ) {
        return new Zoomer;
    }
    if ( description == Shearer::description() ) {
        return new Shearer;
    }
    if ( description == Rotator::description() ) {
        return new Rotator;
    }
    if ( description == RadialBackground::description() ) {
        return new RadialBackground;
    }

    // Couldn't find an Animator class for the given description.
    return 0;
}
