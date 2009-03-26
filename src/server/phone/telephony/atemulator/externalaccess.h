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

#ifndef EXTERNALACCESS_H
#define EXTERNALACCESS_H

#include <QObject>
#include "qtopiaserverapplication.h"

class QCommServiceManager;

class ExternalAccess : public QObject
{
    Q_OBJECT
public:
    ExternalAccess( QObject *parent = 0 );
    ~ExternalAccess();

private slots:
    void servicesChanged();
    void start();

private:
    QCommServiceManager *manager;
    bool started;
};

QTOPIA_TASK_INTERFACE(ExternalAccess);

#endif
