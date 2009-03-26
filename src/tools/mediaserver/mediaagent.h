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

#ifndef MEDIAAGENT_H
#define MEDIAAGENT_H

#include <QObject>
#include <QAudioInterface>

class QMediaSessionRequest;
class QMediaServerSession;
class QMediaEngineInformation;
class QMediaEngine;

namespace mediaserver
{

class MediaAgentSession;

class MediaAgentPrivate;

class MediaAgent : public QObject
{
    Q_OBJECT
    friend class MediaAgentSession;

public:
    ~MediaAgent();

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

    static MediaAgent* instance();

private slots:
    void suspend();
    void resume();
    void updateSessions();

private:
    MediaAgent();

    void initialize();

    void sessionStarting(MediaAgentSession* session);

    void activateMediaEngine(QMediaEngine* mediaEngine);

    MediaAgentPrivate*  d;
    QAudioInterface*    audioMgr;
};


}   // ns mediaserver

#endif
