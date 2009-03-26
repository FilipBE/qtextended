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
#include "thumbcache.h"
#include "photogallery.h"

#include <QEvent>
#include <QPluginManager>
#include <QMutexLocker>
#include <QtopiaApplication>

class ThumbLoadedEvent : public QEvent
{
public:
    ThumbLoadedEvent(const QContent &content, const QImage &image)
        : QEvent(User)
        , content(content)
        , image(image)
    {
    }

    QContent content;
    QImage image;
};

ThumbCache::ThumbCache(QObject *parent)
    : QThread(parent)
    , m_cache(50)
    , m_dropTimerId(-1)
{
    QPluginManager::init();

    connect(QtopiaApplication::instance(), SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
            this, SLOT(contentChanged(QContentIdList,QContent::ChangeType)));

    start();
}

ThumbCache::~ThumbCache()
{
    QMutexLocker lock(&m_mutex);

    m_queue.clear();

    m_waitCondition.wakeAll();

    wait();
}

void ThumbCache::queueImage(const QContent &content)
{
    m_cache.insert(content.id(), new QImage());

    QMutexLocker lock(&m_mutex);

    if (!m_queue.contains(content)) {
        m_queue.append(content);

        if (m_queue.count() > 12 && m_dropTimerId == -1)
            m_dropTimerId = startTimer(100);
        else if (m_queue.count() == 1)
            m_waitCondition.wakeAll();
    }
}

bool ThumbCache::contains(const QContent &content)
{
    return m_cache.contains(content.id());
}

void ThumbCache::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_dropTimerId) {
        QList<QContent> droppedContent;

        {
            QMutexLocker lock(&m_mutex);
            while (m_queue.count() > 12)
                droppedContent.append(m_queue.takeFirst());
        }

        foreach (QContent content, droppedContent) {
            m_cache.remove(content.id());

            emit changed(content);
        }

        if (m_queue.count() <= 12) {
            killTimer(m_dropTimerId);
            m_dropTimerId = -1;
        }
    }
}

void ThumbCache::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User) {
        ThumbLoadedEvent *e = static_cast<ThumbLoadedEvent *>(event);

        m_cache.insert(e->content.id(), new QImage(e->image));

        emit changed(e->content);

        event->accept();
    }
}

void ThumbCache::run()
{
    setPriority(QThread::IdlePriority);

    for (;;) {
        QContent content;
        {
            QMutexLocker lock(&m_mutex);

            if (m_queue.isEmpty()) {
                m_waitCondition.wait(&m_mutex);

                if (m_queue.isEmpty())
                    return;
            }
            content = m_queue.takeFirst();
        }

        QImage image = content.thumbnail();

        int rotation = PhotoGallery::rotation(content);

        if (rotation != 0)
            image = image.transformed(QTransform().rotate(rotation));

        QCoreApplication::postEvent(this, new ThumbLoadedEvent(content, image));
        msleep(10);
    }
}

void ThumbCache::contentChanged(const QContentIdList &contentIds, QContent::ChangeType change)
{
    switch (change) {
    case QContent::Removed:
    case QContent::Updated:
        foreach (QContentId contentId, contentIds)
            m_cache.remove(contentId);
    default:
        break;
    }
}

