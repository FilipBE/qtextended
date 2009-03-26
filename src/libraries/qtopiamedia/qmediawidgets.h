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

#ifndef QMEDIAWIDGETS_H
#define QMEDIAWIDGETS_H

#include <media.h>
#include <qmediacontent.h>
#include <qmediacontrol.h>
#include <qmediatools.h>

#include <QtGui>

class QMediaStateLabelPrivate;

class QTOPIAMEDIA_EXPORT QMediaStateLabel : public QWidget
{
    Q_OBJECT
public:
    explicit QMediaStateLabel( QtopiaMedia::State state = QtopiaMedia::Stopped, QWidget* parent = 0 );
    ~QMediaStateLabel();

    QtopiaMedia::State state() const;

    // QWidget
    QSize sizeHint() const;

public slots:
    void setState( QtopiaMedia::State state );

protected:
    // QWidget
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

private:
    QtopiaMedia::State m_state;
    QMediaStateLabelPrivate *m_d;
};

class QMediaProgressLabelPrivate;

class QTOPIAMEDIA_EXPORT QMediaProgressLabel : public QWidget
{
    Q_OBJECT
public:
    enum Type { ElapsedTime, ElapsedTotalTime, RemainingTime };

    explicit QMediaProgressLabel( Type type = ElapsedTime, QWidget* parent = 0 );
    ~QMediaProgressLabel();

    Type progressType() const;
    void setProgressType( Type type );

    QMediaContent* content() const;

public slots:
    void setMediaContent( QMediaContent* content );
    void setElapsed( quint32 ms );
    void setTotal( quint32 ms );

private slots:
    void activate();
    void deactivate();

private:
    void updateProgress();

    Type m_type;
    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    quint32 m_elapsed, m_total;
    QMediaProgressLabelPrivate *m_d;
};

class QMediaProgressWidgetPrivate;

class QTOPIAMEDIA_EXPORT QMediaProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMediaProgressWidget( QWidget* parent = 0 );
    ~QMediaProgressWidget();

    QMediaContent* content() const;

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void setPosition( quint32 ms );
    void setLength( quint32 ms );

    void activate();
    void deactivate();

private:
    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    QMediaProgressWidgetPrivate *m_d;
};

class QMediaVolumeLabelPrivate;

class QTOPIAMEDIA_EXPORT QMediaVolumeLabel : public QWidget
{
    Q_OBJECT
public:
    enum Type { MinimumVolume, MaximumVolume, MuteVolume };

    explicit QMediaVolumeLabel( Type type = MinimumVolume, QWidget* parent = 0 );
    ~QMediaVolumeLabel();

    Type volumeType() const;
    void setVolumeType( Type type );

    // QWidget
    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

private:
    Type m_type;
    QMediaVolumeLabelPrivate *m_d;
};

class QMediaVolumeWidgetPrivate;

class QTOPIAMEDIA_EXPORT QMediaVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMediaVolumeWidget( QWidget* parent = 0 );
    ~QMediaVolumeWidget();

    QMediaContent* content() const;

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void setVolume( int volume );

    void activate();
    void deactivate();

    void suspend();
    void resume();

protected:
    // QWidget
    void keyPressEvent( QKeyEvent* e );
    void keyReleaseEvent( QKeyEvent* e );

private:
    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    QMediaVolumeWidgetPrivate *m_d;
};

class QMediaSeekWidgetPrivate;

class QTOPIAMEDIA_EXPORT QMediaSeekWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMediaSeekWidget( QWidget* parent = 0 );
    ~QMediaSeekWidget();

    QMediaContent* content() const;

signals:
    void lengthChanged( quint32 ms );
    void positionChanged( quint32 ms );

public slots:
    void setMediaContent( QMediaContent* content );

protected:
    // QWidget
    void keyPressEvent( QKeyEvent* e );
    void keyReleaseEvent( QKeyEvent* e );

private:
    QMediaContentContext *m_context;
    QMediaSeekWidgetPrivate *m_d;
};

#endif
