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

#include "cruxusoutputthread.h"

#include "cruxusoutputdevices.h"


namespace cruxus
{


int OutputDevices::created = 0;
OutputThread* OutputDevices::output = 0;

/*!
    \class cruxus::OutputDevices
    \internal
*/

QMediaDevice* OutputDevices::createOutputDevice()
{
    if (output == 0)
        output = new OutputThread;

    created++;

    return output;
}

void OutputDevices::destroyOutputDevice(QMediaDevice* )
{
    if (--created == 0) {
        delete output;
        output = 0;
    }
}

}   // ns cruxus
