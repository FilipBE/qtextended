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

#ifndef DOUBLEDATA_H
#define DOUBLEDATA_H

#include <qtopiaglobal.h>

#include "data.h"
#include "engine.h"

// Data type
class DoubleData:public Data {
public:
    DoubleData();
    ~DoubleData(){};
    QString getType(){return QString("Double");}; // No tr
    void setEdited(bool edit) {
        edited = edit;
    };

    void set(double);
    double get();
    bool push(char,bool);
    bool del();
    void clear();
private:
    double dbl;
    bool edited;
};

#endif
