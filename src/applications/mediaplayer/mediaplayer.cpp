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

#include <QtopiaApplication>
#include <QSoftMenuBar>

#include <qmediatools.h>

#include "playercontrol.h"
#include "playerwidget.h"
#include "mediabrowser.h"
#include "keyhold.h"
#include "requesthandler.h"
#include "playmediaservice.h"

#include "mediaplayer.h"

#ifndef NO_NICE
#include <unistd.h>
#endif

class MediaServiceRequestHandler : public RequestHandler
{
public:
    struct Context
    {
        MediaPlayer *mediaplayer;
        PlayerControl *control;
#ifdef USE_PICTUREFLOW
        MediaBrowser *mediabrowser;
#endif
    };

    MediaServiceRequestHandler( const Context& context, RequestHandler* handler = 0 )
        : RequestHandler( handler ), m_context( context )
    { }

    // RequestHandler
    void execute( ServiceRequest* request );

private:
    Context m_context;
};

void MediaServiceRequestHandler::execute( ServiceRequest* request )
{
    switch( request->type() )
    {
    case ServiceRequest::OpenUrl:
        {
        OpenUrlRequest *req = (OpenUrlRequest*)request;
        if (req->closeOnFinish())
            m_context.mediaplayer->setDocument(req->url());
        else
            m_context.mediaplayer->openUrl( req->url() );

        delete request;
        }
        break;
    case ServiceRequest::CuePlaylist:
        {
        CuePlaylistRequest *req = (CuePlaylistRequest*)request;
        if(!m_context.mediaplayer->isPlayerVisible()) {
            m_context.mediaplayer->playerWidget();
            m_context.mediaplayer->setPlayerVisible(false);
        }
        if(m_context.mediaplayer->playlist().rowCount() != 0) {
            QMediaPlaylist playlist(m_context.mediaplayer->playlist());
            playlist.enqueue( req->playlist() );
            m_context.mediaplayer->setPlaylist(playlist);
        }
        else
            m_context.mediaplayer->setPlaylist(req->playlist());
        m_context.control->setState( PlayerControl::Playing );

        delete request;
        }
        break;
    case ServiceRequest::PlayNow:
        {
        PlayNowRequest *req = (PlayNowRequest*)request;
        // ensure the player widget is already existing
        if(!m_context.mediaplayer->isPlayerVisible()) {
            m_context.mediaplayer->playerWidget();
            m_context.mediaplayer->setPlayerVisible(false);
        }
        m_context.mediaplayer->setPlaylist(req->playlist());
        m_context.control->setState( PlayerControl::Playing );

        delete request;
        }
        break;
    case ServiceRequest::ShowPlayer:
        {
        m_context.mediaplayer->setPlayerVisible( true );

        delete request;
        }
        break;
#ifdef USE_PICTUREFLOW
    case ServiceRequest::ShowCoverArt:
        {
        m_context.mediaplayer->mediabrowser()->setCoverArtVisible(true);

        delete request;
        }
        break;
#endif
    default:
        RequestHandler::execute( request );
        break;
    }
}

static const int KEY_BACK_HOLD = Qt::Key_unknown + Qt::Key_Back;

MediaPlayer::MediaPlayer( QWidget* parent, Qt::WFlags f ):
    QWidget( parent, f ),
    m_playerwidget( 0 ),
    m_closeonback( false ),
    m_acceptclose( true ),
    m_playlist(QStringList())
{
    setWindowTitle( tr( "Media Player" ) );

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_playercontrol = new PlayerControl( this );

    MediaServiceRequestHandler::Context requestcontext =
#ifndef USE_PICTUREFLOW
    { this, m_playercontrol };
#else
    { this, m_playercontrol, 0 };
#endif

    m_requesthandler = new MediaServiceRequestHandler( requestcontext );

    m_mediabrowser = new MediaBrowser( m_playercontrol, m_requesthandler );
    m_mediabrowser->setCurrentPlaylist( m_playlist );

    m_layout->addWidget( m_mediabrowser );
    setLayout( m_layout );

    m_mediabrowser->show();
    m_mediabrowser->setFocus();

    context = new QMediaContentContext( this );
    connect( m_playercontrol, SIGNAL(contentChanged(QMediaContent*)),
        context, SLOT(setMediaContent(QMediaContent*)) );
    context->addObject( m_mediabrowser );

    new KeyHold( Qt::Key_Back, KEY_BACK_HOLD, 500, this, this );

    // Initialize volume
    QSettings config( "Trolltech", "MediaPlayer" );
    int volume = config.value( "Volume", 50 ).toInt();

    m_playercontrol->setVolume( volume );

#ifndef NO_NICE
    static const int NICE_DELTA = -15;

    // Increase process priority to improve gui responsiveness
    nice( NICE_DELTA );
#endif

    new PlayMediaService(m_requesthandler, this);
}

MediaPlayer::~MediaPlayer()
{
}

void MediaPlayer::setPlaylist( const QMediaPlaylist &playlist )
{
    if( playlist != m_playlist ) {
        // Open playlist in player
        m_mediabrowser->setCurrentPlaylist( playlist );
        playerWidget()->setPlaylist( playlist );

        m_playlist = playlist;

        // Connect to new playlist
        connect( &m_playlist, SIGNAL(playingChanged(QModelIndex)),
            this, SLOT(playingChanged(QModelIndex)) );
    }

    // Launch playlist if playing index is valid
    if( m_playlist.playing().isValid() ) {
        m_acceptclose = false;
        setPlayerVisible( true );
        m_playercontrol->setState( PlayerControl::Playing );
    }
}

bool MediaPlayer::isPlayerVisible() const
{
    return m_playerwidget ? m_playerwidget->isVisible() : false;
}

void MediaPlayer::setPlayerVisible( bool visible )
{
    if( visible ) {
        playerWidget()->show();
        playerWidget()->setFocus();

        m_mediabrowser->hide();
    } else {
        playerWidget()->hide();

        m_mediabrowser->show();
        m_mediabrowser->setFocus();
    }
}

void MediaPlayer::openUrl( const QString& url )
{
    QMediaPlaylist playlist( QStringList() << url );
    playlist.setPlaying( playlist.index( 0 ) );

    setPlaylist( playlist );
}

void MediaPlayer::setDocument( const QString& doc )
{
    m_closeonback = true;
    openUrl( doc );
}

void MediaPlayer::playingChanged( const QModelIndex& index )
{
    if( !index.isValid() ) {
        setPlayerVisible( false );
    }
}

void MediaPlayer::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Back:
        if( !m_closeonback ) {
            if( isPlayerVisible() ) {
                setPlayerVisible( false );
                e->accept();
            } else if( m_mediabrowser->hasBack() ) {
                m_mediabrowser->goBack();
                e->accept();
            } else if( m_playercontrol->state() != PlayerControl::Stopped ) {
                // Hide if player active
                setWindowState( windowState() | Qt::WindowMinimized );
                e->accept();
            } else {
                m_closeonback = true;
                e->accept();
            }
        }

        if( m_closeonback ) {
            m_acceptclose = true;
            m_playercontrol->close();
            hide();
        }
        break;
    case KEY_BACK_HOLD:
        if( isPlayerVisible() ) {
            setPlayerVisible( false );
            e->accept();
        }

        // Return to main menu
        while( m_mediabrowser->hasBack() ) {
            m_mediabrowser->goBack();
            e->accept();
        }
        break;

    case Qt::Key_Hangup:
        m_playercontrol->setState(PlayerControl::Stopped);
        m_acceptclose = true;
        // can't/shouldn't accept on key_hangup, or it'll stop processing it correctly for
        // the rest of the applications, so just fall through to default
    default:
        // Ignore
        QWidget::keyPressEvent(e);
        break;
    }
}

void MediaPlayer::closeEvent( QCloseEvent* e )
{
    if( m_acceptclose ) {
        e->accept();
    } else {
        // ### FIXME Ensure focused widget always has edit focus
        //focusWidget()->setEditFocus( true );
        e->ignore();
    }
}

MediaPlayer *MediaPlayer::instance()
{
    if(QtopiaApplication::instance()->mainWidget() && QtopiaApplication::instance()->mainWidget()->inherits("MediaPlayer"))
        return qobject_cast<MediaPlayer *>(QtopiaApplication::instance()->mainWidget());
    else
        return NULL;
}

PlayerWidget *MediaPlayer::playerWidget()
{
    if(!m_playerwidget)
    {
        m_playerwidget = new PlayerWidget( m_playercontrol );
        m_layout->addWidget(m_playerwidget);
        context->addObject( m_playerwidget );
        m_playerwidget->setPlaylist(m_playlist);
    }
    return m_playerwidget;
}

MediaBrowser *MediaPlayer::mediabrowser()
{
    // todo do some dynamic adding of this, as a safety mechanism
    return m_mediabrowser;
}
