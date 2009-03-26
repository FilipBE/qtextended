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

#include <custom.h>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    QFile maxBrightness;
    if (QFileInfo("/sys/devices/platform/omapfb/panel/backlight_max").exists() ) {
        maxBrightness.setFileName("/sys/devices/platform/omapfb/panel/backlight_max");
    }

    QString strvalue;
    int value;
    if( !maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"max brightness File not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
        maxBrightness.close();
    }

    value = strvalue.toInt();

    return value;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    int steps =  qpe_sysBrightnessSteps();
    qWarning() <<"setBrightness"<<b << steps;

    if(b == 1) {
        // dim
        b = steps - (steps/4);
    }
    else if (b == -1) {
        //bright
        b = steps;
    }

    else if(b == 0) {

    } else if(b == steps) {
    }
    else {

    }

    QFile brightness;
    if (QFileInfo("/sys/devices/platform/omapfb/panel/backlight_level").exists() ) {
        brightness.setFileName("/sys/devices/platform/omapfb/panel/backlight_level");
    }

    if( !brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"brightness File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}
