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

#include "contentdevice.h"
#include "cruxusurihandlers.h"

namespace cruxus
{

/*!
    \class cruxus::UriHandlers
    \internal
*/

QStringList UriHandlers::supportedUris()
{
    QStringList supported;

    supported << "file";

    return supported;
}

QMediaDevice* UriHandlers::createInputDevice(QString const& uriScheme, QString const& path)
{
    if (uriScheme == "file")
    {
        return new ContentDevice(path);
    }

    return 0;
}

void UriHandlers::destroyInputDevice(QMediaDevice* inputDevice)
{
    delete inputDevice;
}


}   // ns cruxus


