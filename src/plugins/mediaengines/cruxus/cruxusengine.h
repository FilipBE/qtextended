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

#ifndef CRUXUSENGINE_H
#define CRUXUSENGINE_H

#include <QMediaEngine>
#include <QMediaSessionBuilder>

#include "cruxussimplesession.h"


class QMediaSessionRequest;
class QMediaServerSession;

namespace cruxus
{

class CruxusEnginePrivate;

class Engine : public QMediaEngine
{
    Q_OBJECT

public:
    Engine();
    ~Engine();

    void initialize();

    void start();
    void stop();

    void suspend();
    void resume();

    QMediaEngineInformation const* engineInformation();

    void registerSession(QMediaServerSession* session);
    void unregisterSession(QMediaServerSession* session);

private:
    CruxusEnginePrivate*    d;
};

}   // ns cruxus

#endif

