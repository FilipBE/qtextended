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

#include "exampletask.h"
#include <qtopiaglobal.h>

#include <QDebug>

class ExampleTask: public QObject
{
public:
    ExampleTask( QObject* parent = 0 ) : QObject(parent)
    {
        //server task code
    }

    ~ExampleTask()
    {
    }
};


ExampleTaskPlugin::ExampleTaskPlugin(QObject* parent)
    : ServerTaskPlugin( parent )
{
}

ExampleTaskPlugin::~ExampleTaskPlugin()
{
}

QByteArray ExampleTaskPlugin::name() const
{
    return QByteArray("ExampleTask");
}

QObject* ExampleTaskPlugin::initTask(void* createArg ) const
{
    Q_UNUSED( createArg );
    return new ExampleTask();
}

bool ExampleTaskPlugin::demand() const
{
    return true;
}

QTOPIA_EXPORT_PLUGIN( ExampleTaskPlugin )


