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

#ifndef HEADERTESTDATA_H
#define HEADERTESTDATA_H

#include <inttypes.h>
#include <QString>

namespace HeaderTestData
{
    QString unicodeString();

    void setQuint32_fillData();
    void rawUint32_fillData();

    void setQuint8_fillData();
    void rawUint8_fillData();

    void setString_fillData(bool canBeUnicode);

    void rawUnicode_fillData(bool canBeUnicode);

    void rawTime_fillData();

    //void setTime4Byte_fillData();
    //void rawTime4Byte_fillData();

    void setBytes_fillData();

    void setUuid_fillData();
    void rawUuid_fillData();

    void setValue_fillData();
};

#endif
