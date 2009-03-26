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

#ifndef QSOUNDPROVIDER_H
#define QSOUNDPROVIDER_H

#include <qtopiaipcadaptor.h>
#include <quuid.h>
#include <media.h>

class QMediaServerSession;

namespace mediaserver
{

class SessionManager;

class QSoundPlayer : public QObject
{
    Q_OBJECT

public:
    QSoundPlayer(QUuid const& id, SessionManager* sessionManager);
    ~QSoundPlayer();

    void open(QString const& filePath);
    void setVolume(int volume);
    void setPriority(int priority);
    void play();
    void stop();

private slots:
    void playerStateChanged(QtopiaMedia::State state);

private:
    QUuid           m_id;
    QString         m_domain;
    SessionManager* m_sessionManager;
    QMediaServerSession*  m_mediaSession;

    QString              m_filename;
    QtopiaMedia::State   m_state;
};


class QSoundProvider : public QtopiaIpcAdaptor
{
    Q_OBJECT

    typedef QMap<QUuid, QSoundPlayer*>  PlayerMap;

public:
    QSoundProvider(SessionManager* sessionManager);
    ~QSoundProvider();

public slots:
    // System wide messages
    void setPriority(int priority);

    // Sound specific messages
    void subscribe(const QUuid& id);

    void open(const QUuid& id, const QString& url);

    void setVolume(const QUuid& id, int volume);

    void setPriority(const QUuid& id, int priority);

    void play(const QUuid& id);

    void stop(const QUuid& id);

    void revoke(const QUuid& id);

private:
    QSoundPlayer* player(QUuid const& id);

    PlayerMap       m_playerMap;
    SessionManager* m_sessionManager;
};

}   // ns mediaserver

#endif
