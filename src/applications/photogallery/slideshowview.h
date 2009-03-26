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
#ifndef SLIDESHOWVIEW_H
#define SLIDESHOWVIEW_H

#include <QTime>
#include <QWidget>

class DurationSlider;
class QContent;
class QToolButton;
class QLabel;
class ImageLoader;
class QThread;
class TitleWindow;
class ImageLoader;
class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;

class SlideShowView : public QWidget
{
    Q_OBJECT
public:
    SlideShowView(QWidget *parent = 0);
    ~SlideShowView();

    void setModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel);

    int timeout() const;

public slots:
    void start();
    void stop();

    void setTimeout(int timeout);

    void enableModelUpdates();
    void disableModelUpdates();

signals:
    void back();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void pause();
    void setImage(const QContent &image);
    void setImage(const QImage&);
    void setCurrentIndex(const QModelIndex&, const QModelIndex& );
    void setCurrentIndex(const QModelIndex&);

    void nextImage();

private:
    QAbstractItemModel *m_model;
    QItemSelectionModel *m_selectionModel;
    bool m_enableModelUpdates;

    QToolButton *m_pauseButton;
    QLabel *m_title;
    TitleWindow *m_controls;
    DurationSlider *m_timeoutSlider;
    QImage m_image;
    int m_rotation;
    int m_advanceTimerId;
    QPoint m_pressedPos;

    QTime m_advanceTime;

    ImageLoader *m_imageLoader;
};

#endif
