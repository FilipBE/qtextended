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

#ifndef DATA_H
#define DATA_H

#include <qtopiaglobal.h>
#include <QString>

class QPixmap;

class Data {
public:
    Data();
    virtual ~Data();

    virtual bool push(char,bool commit=true);
    virtual bool del(); // Returns true if the item is to be deleted completely
    virtual void clear();

    virtual QString getType();
    virtual QString getFormattedOutput();
    virtual QPixmap *draw();
    QPixmap *cache;
protected:
    QString formattedOutput, cachedOutput;
};

#endif
