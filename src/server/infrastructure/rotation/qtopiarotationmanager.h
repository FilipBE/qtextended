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

#ifndef QTOPIAROTATIONMANAGER_H
#define QTOPIAROTATIONMANAGER_H

#include <QWidget>
#include <QtopiaAbstractService>
#ifndef QTOPIA_TEST
#include "qtopiaserverapplication.h"
#endif
class QValueSpaceObject;

class RotationManager : public QObject
{
    Q_OBJECT
public:

    RotationManager( QObject *parent = 0);
    ~RotationManager();

public slots:

    void setRotation(int rotationDegree);
    void defaultRotation();
    int defaultOrientation();

private:
    int getCurrentOrientation();
    QValueSpaceObject* vso;
private slots:
    void restartPainting();
};


//////////////////////////

class RotationService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    RotationService(RotationManager *manager, QObject *parent);
    ~RotationService();

public slots:
    void setCurrentRotation(int degree);
    void defaultRotation();
private:
    RotationManager *rotationManager;
};

#endif
