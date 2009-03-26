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
#include "timing.h"
#include "system.h"
#include "transfer.h"
#include <qsound.h>
#include <qtopianamespace.h>
#include <qsoundcontrol.h>
#include <qpainter.h>
#include <qmovie.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qfontmetrics.h>
#include <qtoolbutton.h>
#include <QHBoxLayout>
#include <QBuffer>
#include <QDebug>
#include <QTemporaryFile>
#include <qdrmcontent.h>
#include <QXmlStreamAttributes>
#include <QMimeType>
#ifdef MEDIA_SERVER
#include <QtopiaMedia>
#include <QMediaControl>
#include <QMediaVideoControl>
#include <QMediaContent>
#endif

static QString tempFileTemplate(Qtopia::tempDir() + "smilmedia-XXXXXX");

SmilMedia::SmilMedia(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    setVisible(false);
}

void SmilMedia::setState(State s)
{
    SmilElement::setState(s);
    sys->update(rect());
}

void SmilMedia::reset()
{
    SmilElement::reset();
    vis = false;
}

SmilMediaParam *SmilMedia::findParameter(const QString &name)
{
    SmilElementList::Iterator it;
    for (it = chn.begin(); it != chn.end(); ++it) {
        if ((*it)->name() == "param") {
            SmilMediaParam *p = (SmilMediaParam *)*it;
            if (p->name == name)
                return p;
        }
    }

    return 0;
}

SmilText::SmilText(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilMedia(sys, p, n, atts), waiting(false)
{
    source = atts.value("src").toString();
}

void SmilText::addCharacters(const QString &ch)
{
    text += ch;
}

void SmilText::setData(const QByteArray &data, const QString &type)
{
    text = QString::fromUtf8(data.data(), data.size());
    SmilMedia::setData(data, type);
}

void SmilText::process()
{
    SmilMedia::process();
    SmilMediaParam *p = findParameter("foregroundColor");
    if (!p)
        p = findParameter("foreground-color");
    if (p)
        textColor = parseColor(p->value);
    if (!source.isEmpty() && !waiting) {
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

Duration SmilText::implicitDuration()
{
    return Duration(0);
}

void SmilText::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        if (!textColor.isValid()) {
            // Ensure the text color has sufficient contrast with the background
            int r, g, b;
            backgroundColor().getRgb(&r, &g, &b);
            textColor = (((r + g + b) / 3) > 128 ? Qt::black : Qt::white);
        }

        QPen oldPen = p->pen();
        p->setPen(textColor);
        p->drawText(rect(), Qt::AlignLeft|Qt::AlignTop|Qt::TextWrapAnywhere, text);
        p->setPen(oldPen);
    }
}

class ImgPrivate : public QObject
{
    Q_OBJECT
public:
    ImgPrivate(SmilSystem *s, const QRect &r);

public slots:
    void status(QMovie::MovieState);
    void update(const QRect &);

public:
    SmilSystem *sys;
    QMovie *movie;
    QRect rect;
    QDrmContent *drm;
    QByteArray imageData;
};

ImgPrivate::ImgPrivate(SmilSystem *s, const QRect &r)
    : QObject(), sys(s), movie(0), rect(r), drm( 0 )
{
}

void ImgPrivate::status(QMovie::MovieState)
{
}

void ImgPrivate::update(const QRect &imgrect)
{
    QRect urect = imgrect;
    urect.translate(rect.x(), rect.y());
    sys->update(urect);
}

SmilImg::SmilImg(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilMedia(sys, p, n, atts), d(0), waiting(false)
{
    source = atts.value("src").toString();
}

SmilImg::~SmilImg()
{
    sys->transferServer()->endData(this, source);
    if (d) {
        delete d->movie;
        delete d->drm;
    }
    delete d;
}

void SmilImg::process()
{
    SmilMedia::process();
    if (!source.isEmpty() && !waiting) {
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

void SmilImg::setState(State s)
{
    SmilMedia::setState(s);
    if (d && d->movie) {
        switch (s) {
            case Startup:
            case Active:
                d->movie->start();
                break;
            case Idle:
            case End:
                d->movie->stop();
                break;
            default:
                break;
        }
    }
    if (d && d->drm) {
        switch (s) {
            case Startup:
            case Active:
                d->drm->renderStarted();
                break;
            case Idle:
                d->drm->renderPaused();
                break;
            case End:
                d->drm->renderStopped();
            default:
                break;
        }
    }
}

static bool withinBounds(const QSize& target, const QSize& bound)
{
    return target.height() < bound.height() && target.width() < bound.width();
}

void SmilImg::setData(const QByteArray &data, const QString &type)
{
    if (type == QLatin1String("application/vnd.oma.drm.content")
       || type == QLatin1String("application/vnd.oma.drm.dcf")) {
        QTemporaryFile file( QDir::tempPath() + QLatin1String("/qt_temp.XXXXXX.dcf" ) );

        if (file.open()) {
            file.write( data );
            file.flush();

            QContent content(QDir::temp().filePath(file.fileName()));
            if (content.type() != QLatin1String("application/vnd.oma.drm.content")
               && type != QLatin1String("application/vnd.oma.drm.dcf")) {
                QDrmContent *drm = new QDrmContent(QDrmRights::Display);
                QIODevice *device = 0;
                if (drm->requestLicense(content) && (device = content.open(QIODevice::ReadOnly)) != 0) {
                    setData( device->readAll(), content.type() );
                    delete device;
                    if (!d)
                        d = new ImgPrivate(sys, rect());
                    d->drm = drm;
                } else {
                    delete drm;
                }
            }
            file.close();
            content.removeFiles();
        }
    } else {
        char head[7];
        memcpy(head, data.data(), qMin(6, data.size()));
        head[6] = '\0';

        if (QString(head) == "GIF89a") {
            d = new ImgPrivate(sys, rect());
            d->imageData = data;
            QBuffer* buf = new QBuffer(&d->imageData,d);
            buf->open(QIODevice::ReadOnly);
            d->movie = new QMovie(buf);
            QObject::connect(d->movie, SIGNAL(updated(QRect)), d, SLOT(update(QRect)));
            QObject::connect(d->movie, SIGNAL(stateChanged(QMovie::MovieState)), d, SLOT(status(QMovie::MovieState)));
        } else {
            QImage img;
            img.loadFromData(data);
            if(!withinBounds(img.size(),rect().size()))
                img = img.scaled(rect().size(), Qt::KeepAspectRatio);
            pix = QPixmap::fromImage(img);
        }
        waiting = false;
        sys->transferServer()->endData(this, source);
        SmilMedia::setData(data, type);
    }
}

Duration SmilImg::implicitDuration()
{
    Duration dur(0);
    if(d && d->movie)
        dur = Duration(d->movie->frameCount() * d->movie->nextFrameDelay());
    else
        dur = Duration(5000); //provide short duration for implicit update later

    return dur;
}

void SmilImg::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        int deltax = (rect().width() - pix.size().width()) / 2;
        int deltay = (rect().height() - pix.size().height()) / 2;
        if (d && d->movie) {
            QPixmap pm = d->movie->currentPixmap();
            deltax = (rect().width() - pm.width()) / 2;
            deltay = (rect().height() - pm.height()) / 2;
            if(!withinBounds(pm.size(),rect().size())) {
                QImage img = d->movie->currentImage();
                img = img.scaled(rect().size(), Qt::KeepAspectRatio);
                pm = QPixmap::fromImage(img);
                deltax = (rect().width() - img.size().width()) / 2;
                deltay = (rect().height() - img.size().height()) / 2;
            }
            p->drawPixmap(rect().x() + deltax, rect().y() + deltay, pm);
        } else {
            p->drawPixmap(rect().x() + deltax, rect().y() + deltay, pix);
        }
    }
}

class AudioPlayer : public QObject
{
public:
    AudioPlayer(QObject *parent = 0);
    ~AudioPlayer();

    void play(const QByteArray &data, const QString &type);
    void stop();

private:
    QString audioFile;
    QSoundControl *soundControl;
};


AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent), soundControl(0)
{
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::play(const QByteArray &data, const QString &type)
{
    const QString lcType(type.toLower());

    stop();
    QString ext;
    if (lcType == "audio/amr") {
        ext = ".amr";
    } else if (lcType == "audio/x-wav" || lcType == "audio/wav") {
        ext = ".wav";
    } else if (lcType == "audio/mpeg") {
        ext = ".mp3";
    } else if (lcType == QLatin1String("application/vnd.oma.drm.content")) {
        ext = QLatin1String(".dcf");
    } else {
        // guess
        char buf[7];
        memcpy(buf, data.data(), qMin(6, data.size()));
        buf[6] = '\0';

        QByteArray head(buf);
        if (head.startsWith("#!AMR")) {
            ext = ".amr";
        } else if (head.startsWith("RIFF")) {
            ext = ".wav";
        } else if (head.startsWith("ID3\003")) {
            // We don't know this is MP3, but we may as well try
            ext = ".mp3";
        }
    }

    if (!ext.isEmpty()) {
        if (!audioFile.isEmpty())
            QFile::remove(audioFile);
        audioFile = Qtopia::tempDir() + "MMSaudio" + ext;
        QFile f(audioFile);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(data);
            f.close();
            soundControl = new QSoundControl(new QSound(audioFile));
            soundControl->sound()->play();
        }
    } else {
        qWarning() << "Unknown audio type:" << lcType;
    }
}

void AudioPlayer::stop()
{
    if ( soundControl ) {
        if (!soundControl->sound()->isFinished())
            soundControl->sound()->stop();
        delete soundControl->sound();
        delete soundControl;
        soundControl = 0;
    }
    if (!audioFile.isEmpty()) {
        QFile::remove(audioFile);
        audioFile = QString();
    }
}

AudioPlayer *SmilAudio::player = 0;

SmilAudio::SmilAudio(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilMedia(sys, p, n, atts), waiting(false)
{
    source = atts.value("src").toString();
}

SmilAudio::~SmilAudio()
{
    sys->transferServer()->endData(this, source);
}

void SmilAudio::process()
{
    SmilMedia::process();
    if (!source.isEmpty() && !waiting) {
        if (!player)
            player = new AudioPlayer();
        waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

void SmilAudio::setState(State s)
{
    SmilMedia::setState(s);
    switch(s) {
        case Active:
            if (player) {
                player->play(audioData, audioType);
            }
            break;
        case Idle:
        case End:
            if (player) {
                player->stop();
            }
            break;
        default:
            break;
    }
}

void SmilAudio::setData(const QByteArray &data, const QString &type)
{
    waiting = false;
    sys->transferServer()->endData(this, source);
    audioData = data;
    audioType = type;
    SmilMedia::setData(data, type);
}

Duration SmilAudio::implicitDuration()
{
    Duration dur;
    dur.setIndefinite(true);
    return dur;
}

void SmilAudio::paint(QPainter *p)
{
    SmilMedia::paint(p);
    if (vis) {
        // may want to use this to display volume control.
    }
}

#ifdef MEDIA_SERVER
class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    enum State { None = 0, MediaSet, PlaybackError, ControlAvailable, VideoControlAvailable };

public:
    VideoPlayer(QObject* parent = 0);
    ~VideoPlayer();

    QWidget* videoWidget(QWidget* parent = 0);

public slots:
    void play();
    void stop();
    void reset();

public:
    void setData(const QByteArray& data, const QString& type);
    int playLength() const;

signals:
    void widgetAvailable();

private slots:
    void controlAvailable(const QString& controlName);
    void mediaError(const QString& errorString);
    void controlValid();

private:
    static const int retryPlayTimeout = 100; //ms
    static const int maxRetryCount = 20;

private:
    QTemporaryFile* m_tempFile;
    QContent m_content;
    QMediaContent* m_mediaContent;
    QMediaControl* m_mediaControl;
    QMediaVideoControl* m_videoControl;
    State m_state;
};

VideoPlayer::VideoPlayer(QObject* parent)
    :
    QObject(parent),
    m_tempFile(0),
    m_mediaContent(0),
    m_mediaControl(0),
    m_videoControl(0),
    m_state(None)
{
}

VideoPlayer::~VideoPlayer()
{
    reset();
}

QWidget* VideoPlayer::videoWidget(QWidget* parent)
{
    if(m_videoControl)
        return m_videoControl->createVideoWidget(parent);

    return 0;
}

void VideoPlayer::play()
{
    static int retryCount = 0;
    if(m_state >= ControlAvailable)
    {
        retryCount = 0;
        m_mediaControl->start();
    }
    else if(m_state == MediaSet)
    {
        retryCount++;
        if(retryCount > maxRetryCount)
        {
            qWarning() << "Playback retried " << retryCount << " times. Aborting.";
            retryCount = 0;
            return;
        }

        qWarning() << "Media control not yet available, retrying..";
        QTimer::singleShot(retryPlayTimeout,this,SLOT(play()));
    }
}

void VideoPlayer::stop()
{
    if(m_state >= ControlAvailable)
        m_mediaControl->stop();
}

void VideoPlayer::reset()
{
    m_videoControl->deleteLater(); m_videoControl = 0;
    m_mediaControl->deleteLater(); m_mediaControl = 0;
    m_mediaContent->deleteLater(); m_mediaContent = 0;
    delete m_tempFile; m_tempFile = 0;
    m_state = None;

    //remove content entry
    if (!m_content.isNull()) {
        QContent::uninstall(m_content.id());
        m_content = QContent();
    }
}

void VideoPlayer::setData(const QByteArray& data, const QString& type)
{
    reset();

    QString ext = QMimeType(type).extension();
    if(ext.isEmpty())
        qWarning() << "Unknown mimetype extension for media " << type;

    m_tempFile = new QTemporaryFile(tempFileTemplate + "." + ext);
    m_tempFile->open();
    m_tempFile->write(data);
    m_tempFile->flush();

    m_content = QContent(m_tempFile->fileName());
    m_mediaContent = new QMediaContent(m_content);

    connect(m_mediaContent, SIGNAL(controlAvailable(QString)), this, SLOT(controlAvailable(QString)));
    connect(m_mediaContent, SIGNAL(mediaError(QString)), this, SLOT(mediaError(QString)));
    m_state = MediaSet;
}

int VideoPlayer::playLength() const
{
    if(m_state >= ControlAvailable)
        return m_mediaControl->length();
    return 0;
}

void VideoPlayer::controlAvailable(const QString& controlName)
{
    if(controlName == QMediaControl::name())
    {
        if(m_mediaControl)
            delete m_mediaControl;
        m_mediaControl = new QMediaControl(m_mediaContent);
        connect(m_mediaControl,SIGNAL(valid()),this,SLOT(controlValid()));
        m_state = ControlAvailable;
    }
    else if(controlName == QMediaVideoControl::name() && m_state == ControlAvailable)
    {
        if(m_videoControl)
            delete m_videoControl;
        m_videoControl = new QMediaVideoControl(m_mediaContent);
        m_state = VideoControlAvailable;
        emit widgetAvailable();
    }
}

void VideoPlayer::controlValid()
{
    //currently not needed
}

void VideoPlayer::mediaError(const QString& errorString)
{
    qWarning() << "Playback error: " << errorString;
    m_state = PlaybackError;
}

SmilVideo::SmilVideo(SmilSystem* system, SmilElement* parent , const QString& name, const QXmlStreamAttributes& atts)
    :
    QObject(),
    SmilMedia(system,parent,name,atts),
    m_videoPlayer(0),
    m_videoWidget(0),
    m_waiting(false)
{
    source = atts.value("src").toString();
}

SmilVideo::~SmilVideo()
{
    delete m_videoPlayer;
    delete m_videoWidget;
}

void SmilVideo::setData(const QByteArray& data, const QString& type)
{
    Q_ASSERT(m_videoPlayer);
    m_videoPlayer->setData(data,type);
    m_waiting = false;
    sys->transferServer()->endData(this, source);
    SmilMedia::setData(data,type);
}

void SmilVideo::process()
{
    SmilMedia::process();
    if (!source.isEmpty() && !m_waiting) {
        if(!m_videoPlayer)
        {
            m_videoPlayer = new VideoPlayer();
            connect(m_videoPlayer,SIGNAL(widgetAvailable()),this,SLOT(widgetAvailable()));
        }
        m_waiting = true;
        sys->transferServer()->requestData(this, source);
    }
}

void SmilVideo::setState(State s)
{

    SmilMedia::setState(s);
    switch(s) {
        case Active:
        {
            Q_ASSERT(m_videoPlayer);
            m_videoPlayer->play();
        }
        break;
        case Idle:
        case End:
        {
            if(m_videoPlayer)            
                m_videoPlayer->stop();
            if(m_videoWidget)
                m_videoWidget->setVisible(false);
        }
        break;
        default:
            break;
    }
}

Duration SmilVideo::implicitDuration()
{
    Duration d;
    if(m_videoPlayer)
        d = Duration(m_videoPlayer->playLength());
    else
        d = Duration(5000); //set a short duration to allow time for the implicit timing update later.
   return d;
}

void SmilVideo::paint(QPainter* p)
{
    SmilMedia::paint(p);
}

void SmilVideo::reset()
{
    if(m_videoPlayer)
        m_videoPlayer->reset();
}

void SmilVideo::widgetAvailable()
{
    m_videoWidget = m_videoPlayer->videoWidget(system()->targetWidget());
    m_videoWidget->setGeometry(rect());
    m_videoWidget->setVisible(true);

    //media api does not allow retrieval of play duration before actual playback, so  we need
    //to update the timing settings after playback has started.

    SmilTimingAttribute* ta = static_cast<SmilTimingAttribute*>(moduleAttribute("Timing"));
    ta->updateImplicitTiming();
}
#endif //MEDIA_SERVER

SmilMediaParam::SmilMediaParam(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts)
    : SmilElement(sys, p, n, atts), valueType(Data)
{
    name = atts.value("name").toString();
    value = atts.value("value").toString();
    type = atts.value("type").toString();
    QString tv = atts.value("typeval").toString();
    if (tv == "ref")
        valueType = Ref;
    else if (tv == "object")
        valueType = Object;
}

SmilMediaModule::SmilMediaModule()
    : SmilModule()
{
}

SmilMediaModule::~SmilMediaModule()
{
}

SmilElement *SmilMediaModule::beginParseElement(SmilSystem *sys, SmilElement *e, const QString &qName, const QXmlStreamAttributes &atts)
{
    if (qName == "text") {
        return new SmilText(sys, e, qName, atts);
    } else if (qName == "img") {
        return new SmilImg(sys, e, qName, atts);
    } else if (qName == "audio") {
        return new SmilAudio(sys, e, qName, atts);
    }
#ifdef MEDIA_SERVER
      else if (qName == "video") {
        return new SmilVideo(sys,e,qName,atts);
    }
#endif
      else if (qName == "ref") {
        // try to guess the actual type.
        QString source = atts.value("src").toString();
        if ( source.contains("image/"))
            return new SmilImg(sys, e, qName, atts);
        else if (source.contains("audio/"))
            return new SmilImg(sys, e, qName, atts);
        else if (source.contains("text/"))
            return new SmilText(sys, e, qName, atts);
#ifdef MEDIA_SERVER
        else if (source.contains("video/"))
            return new SmilVideo(sys,e,qName,atts);
#endif
    } else if (qName == "param") {
        return new SmilMediaParam(sys, e, qName, atts);
    }

    return 0;
}

bool SmilMediaModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlStreamAttributes &)
{
    return false;
}

void SmilMediaModule::endParseElement(SmilElement *, const QString &)
{
}

QStringList SmilMediaModule::elementNames() const
{
    QStringList l;
    l.append("text");
    l.append("img");
    l.append("audio");
#ifdef MEDIA_SERVER
    l.append("video");
#endif
    l.append("ref");
    l.append("param");
    return l;
}

QStringList SmilMediaModule::attributeNames() const
{
    QStringList l;
    return l;
}

#include "media.moc"

