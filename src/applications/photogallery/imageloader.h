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
#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QContent>
#include <QImage>

class ImageLoader : public QThread
{
Q_OBJECT
public:
    ImageLoader( QObject *parent=0 );
    ~ImageLoader();

public slots:
    void loadImage( const QContent&, const QSize& displaySize = QSize() );

signals:
    void loaded( const QImage&, bool downscaled );

protected:
    void customEvent(QEvent *event);
    void run();

private:
    QContent m_contentToLoad;
    QSize m_displaySize;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    bool m_exiting;
};

#endif

