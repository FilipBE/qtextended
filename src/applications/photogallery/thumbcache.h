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
#ifndef THUMBCACHE_H
#define THUMBCACHE_H

#include <QThread>
#include <QCache>
#include <QContent>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

class ThumbCache : public QThread
{
    Q_OBJECT
public:
    ThumbCache(QObject *parent = 0);
    ~ThumbCache();

    QImage *image(const QContent &content) const
        { return m_cache.object(content.id()); }

    void queueImage(const QContent &content);

    bool contains(const QContent &content);

signals:
    void changed(const QContent &content);

protected:
    void timerEvent(QTimerEvent *event);
    void customEvent(QEvent *event);

    void run();

private slots:
    void contentChanged(const QContentIdList &contentIds, QContent::ChangeType change);

private:
    QCache<QContentId, QImage> m_cache;
    QList<QContent> m_queue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    int m_dropTimerId;
};

#endif
