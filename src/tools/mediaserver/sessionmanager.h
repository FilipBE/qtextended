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

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QList>

#include <qtopiamedia/media.h>

class QMediaSessionRequest;
class QMediaServerSession;

namespace mediaserver
{

class SessionManagerSession;
class SessionManagerPrivate;

class SessionManager : public QObject
{
    Q_OBJECT

    friend class SessionManagerSession;
    friend class SessionManagerPrivate;

public:
    ~SessionManager();

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

    static SessionManager* instance();

    QList<QMediaServerSession*> const& sessions() const;

signals:
    void activeSessionCountChanged(int);

private:
    SessionManager();

    bool sessionCanStart(SessionManagerSession* session);
    void sessionStopped(SessionManagerSession* session);

    SessionManagerPrivate*  d;
};


}   // ns mediaserver

#endif
