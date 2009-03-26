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

#include "gstreamerurisessionbuilder.h"

#include "gstreamerengineinformation.h"


namespace gstreamer
{

class EngineInformationPrivate
{
public:
    Engine*                     engine;
    QMediaSessionBuilderList    sessionBuilders;
};

/*!
    \class gstreamer::EngineInformation
    \internal
*/

EngineInformation::EngineInformation(Engine* engine):
    d(new EngineInformationPrivate)
{
    d->engine = engine;
    d->sessionBuilders.append(new UriSessionBuilder(engine));
}

EngineInformation::~EngineInformation()
{
    delete d;
}


QString EngineInformation::name() const
{
    return "GStreamer";
}

QString EngineInformation::version() const
{
    return "0.1";
}


int EngineInformation::idleTime() const
{
    return -1;
}

bool EngineInformation::hasExclusiveDeviceAccess() const
{
#ifdef HAVE_OSS
    return true;
#else
    return false;
#endif
}

QMediaSessionBuilderList EngineInformation::sessionBuilders() const
{
    return d->sessionBuilders;
}

}   // ns gstreamer

