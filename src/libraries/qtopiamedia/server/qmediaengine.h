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

#ifndef QMEDIAENGINE_H
#define QMEDIAENGINE_H

#include <QObject>
#include <QList>

#include <qtopiaglobal.h>

class QMediaEngineInformation;

class QTOPIAMEDIA_EXPORT QMediaEngine : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaEngine();

    virtual void initialize() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void suspend() = 0;
    virtual void resume() = 0;

    virtual QMediaEngineInformation const* engineInformation() = 0;
};

typedef QList<QMediaEngine*>    QMediaEngineList;

#endif
