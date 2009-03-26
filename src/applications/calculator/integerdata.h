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

#ifndef INTEGERDATA_H
#define INTEGERDATA_H

#include "data.h"

/*
This class supports bases 2,8,10 and 16
and may be subclassed to support other
bases without having to create new
conversion functions
*/
class IntegerData : public Data {
public:
    IntegerData():Data(){};
    ~IntegerData(){};

    virtual bool appendChar(char);
    virtual void delChar();
    virtual void clear(){set(0);};

    QString getType(){return QString("INTEGER");};

    void set(int = 0,int = 10);
    int get(){return i;};
private:
    int i,base;
};

#endif
