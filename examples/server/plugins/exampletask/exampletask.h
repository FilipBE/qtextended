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

#ifndef EXAMPLETASK_H
#define EXAMPLETASK_H

#include <ServerTaskPlugin>

class ExampleTaskPlugin : public ServerTaskPlugin
{
    Q_OBJECT
public:
    ExampleTaskPlugin( QObject* parent = 0 );
    virtual ~ExampleTaskPlugin();

    //name of task
    QByteArray name() const;

    //initialises task
    QObject* initTask( void* createArg = 0 ) const;

    //returns true if started in demand
    bool demand() const;
};

#endif
