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

#ifndef HELIXENGINE_H
#define HELIXENGINE_H

#include <qmediaengine.h>
#include <qmediasessionbuilder.h>
#include "helixsession.h"

class QTimerEvent;
class QMediaEngineInformation;
class QMediaSessionRequest;
class QMediaServerSession;
class IHXClientEngine;

namespace qtopia_helix
{

class HelixEnginePrivate;

class HelixEngine :
    public QMediaEngine,
    public QMediaSessionBuilder
{
    Q_OBJECT

public:
    HelixEngine();
    ~HelixEngine();

    // QMediaEngine
    void initialize();

    void start();
    void stop();

    void suspend();
    void resume();

    QMediaEngineInformation const* engineInformation();

    // QMediaSessionBuilder
    QString type() const;
    QMediaSessionBuilder::Attributes const& attributes() const;

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* serverSession);

private:
    void timerEvent(QTimerEvent* timerEvent);

    HelixEnginePrivate* d;
    QList<HelixSession*> sessions;
    QList<HelixSession*> suspendedSessions;
};

}   // ns qtopia_helix

#endif
