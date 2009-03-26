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

#ifndef HELIXPLAYER_H
#define HELIXPLAYER_H

#include <interfaces.h>
#include <helixutil.h>
#include <helixsite.h>

#include <QtCore>
#include <QDirectPainter>
#include <QWidget>

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxausvc.h>
#include <hxerror.h>

#include "helixvideowidget.h"


class PlayerProgressAdvise;
class PlayerStateAdvise;
class PlayerVolumeAdvise;
class PlayerErrorSink;

class HelixPlayer : public QObject,
    public BasicControl,
    public PlayerState,
    public SeekControl,
    public VolumeControl,
    public PlaybackStatus,
    public VideoRender,
    public ErrorReport,
    public PlayerSettings,
    public Observer
{
    Q_OBJECT
    Q_INTERFACES(BasicControl)
    Q_INTERFACES(PlayerState)
    Q_INTERFACES(SeekControl)
    Q_INTERFACES(VolumeControl)
    Q_INTERFACES(PlaybackStatus)
    Q_INTERFACES(VideoRender)
    Q_INTERFACES(ErrorReport)
    Q_INTERFACES(PlayerSettings)

public:
    HelixPlayer(IHXClientEngine* engine);
    ~HelixPlayer();

    // BasicControl
    void open( const QString& url );

    void play();
    void pause();
    void stop();

    // PlayerState
    QtopiaMedia::State playerState() const;

    // SeekControl
    void seek( quint32 ms );

    // VolumeControl
    void setMuted( bool mute );
    bool isMuted() const;

    void setVolume( int volume );
    int volume() const;

    // PlaybackStatus
    quint32 length() const;
    quint32 position() const;

    // VideoRender
    bool hasVideo() const;
    VideoWidget* createVideoWidget();

    // ErrorReport
    QString errorString() const;

    // PlayerSettings
    QVariant value(QString const&) const;
    void setValue(QString const&, QVariant const& value);

    // Observer
    void update( Subject* subject );

    void hasStopped();

private slots:
    void updateVideoSurfaceFormats();

private:
    void startProgressTimer();
    void stopProgressTimer();
    void timerEvent(QTimerEvent* timerEvent);

    int                 m_stopTimerId;
    int                 m_progressTimerId;
    int                 m_progressWatch;
    IHXClientEngine*    m_engine;
    IHXPlayer *m_player;
    IHXVolume *m_volume;
    IHXErrorSinkControl *m_errorcontrol;

    PlayerProgressAdvise *m_progressadvise;
    PlayerStateAdvise *m_stateadvise;
    PlayerVolumeAdvise *m_volumeadvise;

    HelixSiteSupplier *m_sitesupplier;
    PlayerErrorSink *m_errorsink;
    GenericContext *m_context;
};

#endif
