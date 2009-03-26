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

#ifndef MEDIAAGENTSESSION_H
#define MEDIAAGENTSESSION_H

#include <QMediaServerSession>

namespace mediaserver
{

class MediaAgent;

class MediaAgentSessionPrivate;

class MediaAgentSession : public QMediaServerSession
{
    Q_OBJECT

public:
    MediaAgentSession(MediaAgent* agent, QMediaServerSession* wrapped, QString engineName);
    ~MediaAgentSession();

    QMediaServerSession* wrappedSession() const;
    QString engineName() const;

    void start();
    void pause();
    void stop();

    void suspend();
    void resume();
    bool isSuspended() const;
    bool isActive() const;

    void seek(quint32 ms);
    quint32 length();

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool mute);
    bool isMuted() const;

    QtopiaMedia::State playerState() const;

    QString errorString();

    void setDomain(QString const& domain);
    QString domain() const;

    QStringList interfaces();

    QString id() const;
    QString reportData() const;

private slots:
    void processPlayerStateChange(QtopiaMedia::State);

private:
    MediaAgentSessionPrivate*  d;
};

}   // ns mediaserver

#endif
