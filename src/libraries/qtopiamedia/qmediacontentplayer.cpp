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

#include "media.h"
#include "qmediacontent.h"
#include "qmediacontrol.h"
#include "qmediacontentplayer_p.h"


// {{{ QMediaContentPlayerPrivate
class QMediaContentPlayerPrivate : public QObject
{
    Q_OBJECT

public:
    QMediaContentPlayerPrivate(QMediaContent* mediaContent):
        m_mediaContent(mediaContent),
        m_mediaControl(0)
    {
        connect(m_mediaContent, SIGNAL(controlAvailable(QString)),
                this, SLOT(controlAvailable(QString)));

        connect(m_mediaContent, SIGNAL(mediaError(QString)),
                this, SLOT(mediaError()));
    }

    ~QMediaContentPlayerPrivate()
    {
        delete m_mediaControl;
        delete m_mediaContent;
    }

public slots:
    void controlAvailable(QString const& controlName)
    {
        if (controlName == QMediaControl::name())
        {
            m_mediaControl = new QMediaControl(m_mediaContent);

            connect(m_mediaControl, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                    this, SLOT(playerStateChanged(QtopiaMedia::State)));

            m_mediaControl->start();
        }
    }

    void mediaError()
    {
        deleteLater();
    }

    void playerStateChanged(QtopiaMedia::State state)
    {
        switch (state)
        {
        case QtopiaMedia::Stopped:
        case QtopiaMedia::Error:
            deleteLater();
            break;

        default:
            ;
        }
    }

private:
    QMediaContent*  m_mediaContent;
    QMediaControl*  m_mediaControl;
};
// }}}


/*!
    \class QMediaContentPlayer
    \inpublicgroup QtMediaModule
    \internal
    \brief The QMediaContentPlayer class is used to immediately, without
            further intervention, play media.

    QMediaContentPlayer is a simple interface to playing media content.
    Construction will trigger the playing of the media, the media will play
    until the end without any user intervention. If the media content is not
    available to be played there will be no error notification.

    QMediaContentPlayer will currently not work for media that requires
    interaction with the host application, such as video.

    \code
    Example;
    To Play a notification sound on an event.
    {
        QContent    media = grabNotificationMediaFromPreferences();

        QMediaContentPlayer(media);
    }
    \endcode

    \ingroup multimedia
*/

/*!
    Create a QMediaContentPlayer from a URL.

    The URL may point to a local or remote media resource.

    The \a url paramter is a URL representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
*/

QMediaContentPlayer::QMediaContentPlayer(QUrl const& url, QString const& domain)
{
    new QMediaContentPlayerPrivate(new QMediaContent(url, domain));
}


/*!
    Create a QMediaContentPlayer from a QContent.

    This creates a QMediaContentPlayer from a local resource known by the
    Document System.

    The \a content is the QContent representing the location of the media content.
    The \a domain is the audio domain in which this media content should exist.
*/

QMediaContentPlayer::QMediaContentPlayer(QContent const& content, QString const& domain)
{
    new QMediaContentPlayerPrivate(new QMediaContent(content, domain));
}

/*!
    Create a QMediaContentPlayer from an existing QMediaContent.

    An existing QMediaContent maybe used with this constructor.
    The QMediaContentPlayer takes ownership of this \a mediaContent 
    parameter.

*/

QMediaContentPlayer::QMediaContentPlayer(QMediaContent* mediaContent)
{
    new QMediaContentPlayerPrivate(mediaContent);
}

/*!
    Destroy the QMediaContentPlayer.
*/

QMediaContentPlayer::~QMediaContentPlayer()
{
}


#include "qmediacontentplayer.moc"

