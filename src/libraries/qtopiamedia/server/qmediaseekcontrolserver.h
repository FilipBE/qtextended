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

#ifndef QMEDIASEEKCONTROLSERVER_H
#define QMEDIASEEKCONTROLSERVER_H

#include "qmediaabstractcontrolserver.h"

#include <qtopiaglobal.h>

#include "media.h"



class QMediaHandle;



class QTOPIAMEDIA_EXPORT QMediaSeekDelegate : public QObject
{
    Q_OBJECT

public:
    virtual ~QMediaSeekDelegate();

    virtual void seek(quint32 position, QtopiaMedia::Offset offset) = 0;
    virtual void jumpForward(quint32 ms) = 0;
    virtual void jumpBackwards(quint32 ms) = 0;
    virtual void seekToStart() = 0;
    virtual void seekToEnd() = 0;
    virtual void seekForward() = 0;
    virtual void seekBackward() = 0;

signals:
    void positionChanged(quint32 ms);
};


class QTOPIAMEDIA_EXPORT QMediaSeekControlServer :
    public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    QMediaSeekControlServer(QMediaHandle const& handle,
                            QMediaSeekDelegate* seekDelegate,
                            QObject* parent = 0);
    ~QMediaSeekControlServer();

public slots:
    void seek(quint32 position, QtopiaMedia::Offset offset);
    void jumpForward(quint32 ms);
    void jumpBackwards(quint32 ms);
    void seekToStart();
    void seekToEnd();
    void seekForward();
    void seekBackward();

signals:
    void positionChanged(quint32 ms);

private:
    QMediaSeekDelegate*     m_seekDelegate;
};

#endif

