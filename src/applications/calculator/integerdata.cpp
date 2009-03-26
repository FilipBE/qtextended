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
#include "integerdata.h"
#include <QDebug>

// Data type functions
void IntegerData::set(int j,int b) {
    i = j;
    if (base != 2 || base != 8 ||
            base != 10 || base != 16) {
        qWarning("Base %d is not supported",b);
        base = 10;
    } else
        base = b;
}
bool IntegerData::appendChar(char c) {
    QString tmp = formattedOutput;
    tmp.append(c);
    bool ok;
    i = tmp.toInt(&ok,base);
    if (!ok)
        return false;
    formattedOutput = tmp;
    return true;
}
void IntegerData::delChar() {
    formattedOutput.truncate(formattedOutput.length()-1);
    if (!formattedOutput.length())
        formattedOutput.setNum(0);
    bool ok;
    i = formattedOutput.toInt(&ok,base);
}

