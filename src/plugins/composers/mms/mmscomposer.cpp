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

#include "mmscomposer.h"
#include <private/accountconfiguration_p.h>
#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qcolorselector.h>
#include <qmailmessage.h>
#include <qmimetype.h>
#include <qaudiosourceselector.h>
#include <qimagesourceselector.h>
#include <QAction>
#include <QBuffer>
#include <QPainter>
#include <QLayout>
#include <QStackedWidget>
#include <QDialog>
#include <QImage>
#include <QImageReader>
#include <QSpinBox>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QTextStream>
#include <QDataStream>
#include <QBitArray>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QXmlStreamReader>
#include <private/detailspage_p.h>
#include <QMailAccount>
#include <QPushButton>
#include "videoselector.h"
#include <QFlags>
#include <QTemporaryFile>

static const unsigned int kilobyte = 1024;
static const unsigned int maxMessageSize = 300; //kB
static const char* imageFormat(){static const char* f = "JPG"; return f;};
static const int imageQuality = 100;

class NoMediaButton : public QPushButton
{
    Q_OBJECT
public:
    NoMediaButton(QWidget* parent = 0);

    QSize sizeHint() const;

signals:
    void leftPressed();
    void rightPressed();

protected:
    void keyPressEvent(QKeyEvent* e);
};

NoMediaButton::NoMediaButton(QWidget* parent)
:
    QPushButton(parent)
{
    setText("No media");
}

void NoMediaButton::keyPressEvent(QKeyEvent* e)
{
    switch( e->key() )
    {
    case Qt::Key_Left:
        emit leftPressed();
        e->accept();
        break;
    case Qt::Key_Right:
        emit rightPressed();
        e->accept();
        break;
    default:
        QPushButton::keyPressEvent(e);
        break;
    }
}

QSize NoMediaButton::sizeHint() const
{
    QWidget *par = 0;
    if( parent() && parent()->isWidgetType() )
        par = static_cast<QWidget *>(parent());
    QRect mwr = QApplication::desktop()->availableGeometry();
    int w = par ? par->width() : mwr.width(),
        h = par ? par->height() : mwr.height();
    return QSize(w/3*2, h/3*2);
}

class MediaSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    enum Action{ None = 0x0, SelectVideo=0x01, SelectImage=0x02 };
    Q_DECLARE_FLAGS(Actions,Action);

public:
    MediaSelectionDialog(Actions availableActions, QWidget* parent = 0);
    Action action() const;

private slots:
    void buttonClicked();

private:
    QPushButton* m_imageMediaButton;
    QPushButton* m_videoMediaButton;
    Action m_action;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaSelectionDialog::Actions);

MediaSelectionDialog::MediaSelectionDialog(Actions availableActions, QWidget* parent)
:
    QDialog(parent),
    m_imageMediaButton(new QPushButton("Image",this)),
    m_videoMediaButton(new QPushButton("Video",this)),
    m_action(None)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_imageMediaButton);
    l->addWidget(m_videoMediaButton);
    QWidget::setTabOrder(m_imageMediaButton,m_videoMediaButton);
    connect(m_imageMediaButton,SIGNAL(clicked()),this,SLOT(buttonClicked()));
    connect(m_videoMediaButton,SIGNAL(clicked()),this,SLOT(buttonClicked()));

    Q_ASSERT(availableActions != None);
    m_videoMediaButton->setVisible(availableActions & SelectVideo);
    m_imageMediaButton->setVisible(availableActions & SelectImage);

    this->setWindowTitle("Slide media");
}

MediaSelectionDialog::Action MediaSelectionDialog::action() const
{
    return m_action;
}

void MediaSelectionDialog::buttonClicked()
{
    if(sender() == m_imageMediaButton)
        m_action =  SelectImage;
    else if(sender() == m_videoMediaButton)
        m_action = SelectVideo;
    else m_action = None;

    accept();
}

MMSSlideImage::MMSSlideImage(QWidget *parent)
    : QLabel(parent)
{
    setAlignment( Qt::AlignCenter );
    connect( this, SIGNAL(clicked()), this, SLOT(select()) );
    setFocusPolicy( Qt::StrongFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    // would like to specify a value relative to parent here but Qt makes it hard..
    setMinimumSize( 0, 30 );
    loadQueue().enqueue(this);
}

MMSSlideImage::~MMSSlideImage()
{
    loadQueue().removeAll(this);
}

QContent& MMSSlideImage::document()
{
    return m_content;
}

QString MMSSlideImage::mimeType() const
{
    //TODO this needs to be more robust. We need to be given the correct mimetype from QContent
    QString id;
    if(m_image.isNull())
        id = QMimeType(m_content).id();
    else
        id = "image/jpeg"; //written out as JPEG later
    return id;
}

quint64 MMSSlideImage::numBytes() const
{
    if(m_content.isValid())
        return m_content.size();
    else
    {
        QBuffer buf;
        m_image.save(&buf,imageFormat(),imageQuality);
        return buf.data().size();
    }
}

void MMSSlideImage::loadImages(const QSize& maxSize)
{
    //preload all known image slides based on maxSize
    while(!loadQueue().isEmpty())
    {
        MMSSlideImage* s = loadQueue().dequeue();
        s->loadImage(maxSize);
    }
}

void MMSSlideImage::mousePressEvent( QMouseEvent *event )
{
    if( rect().contains( event->pos() ) )
        m_pressed = true;
}

void MMSSlideImage::mouseReleaseEvent( QMouseEvent *event )
{
    if( m_pressed && rect().contains( event->pos() ) )
        emit clicked();
    m_pressed = false;
}

void MMSSlideImage::keyPressEvent( QKeyEvent *event )
{
    if( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch( keyEvent->key() )
        {
            case Qt::Key_Left:
                emit leftPressed();
                break;
            case Qt::Key_Right:
                emit rightPressed();
                break;
            case Qt::Key_Select:
                emit clicked();
                break;
            default:
                QLabel::keyPressEvent( event );
                break;
        }
    }
}

void MMSSlideImage::paintEvent( QPaintEvent *event )
{
    QLabel::paintEvent( event );
    if( hasFocus() )
    {
        QPainter p( this );
        QPen pen(palette().highlight().color());
        p.setPen( pen );
        p.drawRect( 0, 0, width(), height() );
        p.drawRect( 1, 1, width()-2, height()-2 );
    }
}

QRect MMSSlideImage::contentsRect() const
{
    if (isEmpty())
        return QRect();

    QPoint pnt( rect().x() + (width() - m_image.width()) / 2 + 2,
                rect().y() + (height() - m_image.height()) / 2 + 2);
    pnt = mapToParent( pnt );
    return QRect( pnt.x(), pnt.y(), m_image.width(), m_image.height() );
}

QSize MMSSlideImage::sizeHint() const
{
    QWidget *par = 0;
    if( parent() && parent()->isWidgetType() )
        par = static_cast<QWidget *>(parent());
    QRect mwr = QApplication::desktop()->availableGeometry();
    int w = par ? par->width() : mwr.width(),
        h = par ? par->height() : mwr.height();
    return QSize(w/3*2, h/3*2);
}

void MMSSlideImage::select()
{
    QImageSourceSelectorDialog *selector = new QImageSourceSelectorDialog(this);
    selector->setObjectName("slideImageSelector");
    selector->setMaximumImageSize(QSize(80, 96));
    selector->setContent(m_content);
    selector->setModal(true);
    selector->setWindowTitle(tr("Slide photo"));

    int result = QtopiaApplication::execDialog( selector );
    if( result == QDialog::Accepted )
    {
        bool ok = true;
        emit aboutToChange(ok,selector->content().size()-numBytes());
        if(ok)
            setImage( selector->content() );
    }
    delete selector;
}

void MMSSlideImage::resizeEvent( QResizeEvent *event )
{
    QLabel::resizeEvent( event );

    if (isVisible()) {
        if( !m_image.isNull() ) {
            m_image = scale( m_image );
            setPixmap( m_image );
        }
    }
}

QPixmap MMSSlideImage::scale( const QPixmap &src ) const
{
    if (src.isNull())
        return src;

    if( (src.width() >= width()) || (src.height() >= height()) )
        return src.scaled( size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );

    return src;
}

QQueue<MMSSlideImage*>& MMSSlideImage::loadQueue()
{
    static QQueue<MMSSlideImage*> loadqueue;
    return loadqueue;
}

void MMSSlideImage::loadImage(const QSize& explicitSize)
{
    bool preloadContext = !explicitSize.isEmpty();

    if(!m_content.isValid())
    {
        if(!preloadContext)
            setImage(QPixmap());
        return;
    }

    // Load the image to fit our display or the provided explicit size
    QImageReader imageReader( m_content.open() );

    if (imageReader.supportsOption(QImageIOHandler::Size)) {
        QSize fileSize(imageReader.size());

        QSize bounds;
        if(preloadContext)
            bounds = explicitSize;
        else
            bounds = (isVisible() ? size() : QApplication::desktop()->availableGeometry().size());

        // See if the image needs to be scaled during load
        if ((fileSize.width() > bounds.width()) || (fileSize.height() > bounds.height()))
        {
            // And the loaded size should maintain the image aspect ratio
            QSize imageSize(fileSize);
            imageSize.scale(bounds, Qt::KeepAspectRatio);
            imageReader.setQuality( 49 ); // Otherwise Qt smooth scales
            imageReader.setScaledSize(imageSize);
        }
    }
    QPixmap m = QPixmap::fromImage(imageReader.read());
    setImage(m);
}

void MMSSlideImage::setImage( const QContent& document )
{
    m_content = document;
    loadImage();
}

void MMSSlideImage::setImage( const QPixmap& image )
{
    m_image = image;
    m_contentSize = m_image.size();

    if( m_image.isNull() && !m_content.isValid() )
        setText( tr("Slide image") );
    else setPixmap( m_image );

    emit changed();
}

QImage MMSSlideImage::image() const
{
    return m_image.toImage().convertToFormat(QImage::Format_RGB32);
}

bool MMSSlideImage::isEmpty() const
{
    return (m_image.isNull() && !m_content.isValid());
}

MMSSlideVideo::MMSSlideVideo(QWidget* parent)
:
    QWidget(parent),
    m_tempFile(0),
    m_videoWidget(0)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);

    m_videoWidget = new QPushButton(this);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    l->addWidget(m_videoWidget);
    connect(m_videoWidget,SIGNAL(clicked()),this,SLOT(select()));
    setFocusProxy(m_videoWidget);

    setFocusPolicy( Qt::StrongFocus );
}

MMSSlideVideo::~MMSSlideVideo()
{
    if(m_tempFile)
    {
        if (!m_videoMedia.isNull())
            QContent::uninstall(m_videoMedia.id());
        delete m_tempFile; m_tempFile = 0;
    }
}

bool MMSSlideVideo::isEmpty() const
{
    return !m_videoMedia.isValid();
}

QContent MMSSlideVideo::document() const
{
    if(m_tempFile)
        return QContent();
    return m_videoMedia;
}

void MMSSlideVideo::setVideo(const QContent& c)
{
    m_videoMedia = c;
    m_videoWidget->setText(tr("Video File\n") + m_videoMedia.name());
    emit changed();
}

void MMSSlideVideo::setVideo(const QByteArray& data, const QString& name)
{
    delete m_tempFile;
    QString filename = QString(name).remove(QRegExp("[<>]"));
    m_tempFile = new QTemporaryFile("XXXXXXXX-" + filename);
    m_tempFile->open();
    m_tempFile->write(data);
    m_tempFile->flush();
    QString filePath = QFileInfo(*m_tempFile).absoluteFilePath();
    QContent tempContent(filePath);
    tempContent.setName(filename);
    bool ok = true;
    emit aboutToChange(ok,tempContent.size()-numBytes());
    if(ok) setVideo(tempContent);
}

QByteArray MMSSlideVideo::video() const
{
    if(m_tempFile)
    {
        m_tempFile->seek(0);
        return m_tempFile->readAll();
    }
    return QByteArray();
}

QString MMSSlideVideo::mimeType() const
{
    //TODO this needs to be more robust. We need to be given the correct mimetype from QContent.
    QString id = QMimeType(m_videoMedia).id();
    if(id.contains("audio/"))
            id.replace("audio","video",Qt::CaseInsensitive);
    return id;
}

quint64 MMSSlideVideo::numBytes() const
{
    if(!m_videoMedia.isNull())
        return m_videoMedia.size();
    return 0;
}

void MMSSlideVideo::select()
{
    VideoSourceSelectorDialog selector(this);
    selector.setContent(m_videoMedia);
    selector.setModal(true);
    selector.setWindowTitle(tr("Slide video"));

    int result = QtopiaApplication::execDialog(&selector);
    if(result == QDialog::Accepted)
    {
        bool ok = true;
        emit aboutToChange(ok,selector.content().size()-numBytes());
        if(ok)
            setVideo(selector.content());
    }
}

void MMSSlideVideo::keyPressEvent(QKeyEvent* e)
{
    if( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        switch( keyEvent->key() )
        {
            case Qt::Key_Left:
                emit leftPressed();
                break;
            case Qt::Key_Right:
                emit rightPressed();
                break;
            case Qt::Key_Select:
                emit clicked();
                break;
            default:
                QWidget::keyPressEvent(e);
                break;
        }
    }
}

MMSSlideText::MMSSlideText(QWidget *parent)
    : QTextEdit(parent), defaultText( QObject::tr("Your text here...") ), m_hasFocus( false )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                            QSizePolicy::MinimumExpanding ) );
    setWordWrapMode(QTextOption::WordWrap);
    setText( QString() );
    connect(this,SIGNAL(textChanged()),this,SIGNAL(changed()));
}

bool MMSSlideText::event( QEvent *e )
{
    bool a = QTextEdit::event( e );
    if( e->type() == QEvent::EnterEditFocus && text().isNull() ) {
        clear();
    } else if( ( ( e->type() == QEvent::LeaveEditFocus ) ||
                 ( e->type() == QEvent::FocusOut && m_hasFocus ) ) &&
               ( text().isEmpty() ) ) {
        // Reset the text back to the placeholder
        setText( QString() );
    }

    return a;
}

void MMSSlideText::mousePressEvent( QMouseEvent * )
{
    if( !m_hasFocus )
    {
        if (text().isNull())
            clear();
        else
            selectAll();
        m_hasFocus = true;
    }
}

void MMSSlideText::keyPressEvent( QKeyEvent *e )
{
    if (!Qtopia::mousePreferred()) {
        if (!hasEditFocus()) {
            if (e->key() == Qt::Key_Left) {
                emit leftPressed();
                e->accept();
                return;
            } else if (e->key() == Qt::Key_Right) {
                emit rightPressed();
                e->accept();
                return;
            }
            //else fall through
        }
    }

    //check if we can accomodate text.

    QString text = e->text();
    if(!text.isEmpty() && hasEditFocus())
    {
        bool ok = true;
        emit aboutToChange(ok,text.toUtf8().count());
        if(!ok)
            e->accept();
    }

    QTextEdit::keyPressEvent( e );
    updateGeometry();
}

QRect MMSSlideText::contentsRect() const
{
    if( text().isNull() )
        return QRect();

    QPoint pnt = rect().topLeft();
    pnt = mapToParent( pnt );
    return QRect( pnt.x(), pnt.y(), rect().width()-2, rect().height() - 2 );
}

quint64 MMSSlideText::numBytes() const
{
    QByteArray data = text().toUtf8();
    return data.count();
}

void MMSSlideText::setText( const QString &txt )
{
    if( txt.trimmed().isEmpty() ) {
        QTextEdit::setPlainText( defaultText );
        selectAll();
    } else {
        QTextEdit::setPlainText( txt );
    }
    updateGeometry();
}

QString MMSSlideText::text() const
{
    QString t = QTextEdit::toPlainText().simplified();
    if( t == MMSSlideText::defaultText )
        t = QString();
    return t;
}

QSize MMSSlideText::sizeHint() const
{
    return QSize(0,0);
}

bool MMSSlideText::isEmpty() const
{
    return text().isEmpty();
}

MMSSlideAudio::MMSSlideAudio(QWidget *parent)
    : QPushButton(parent)
{
    setIcon(QIcon(":icon/sound"));
    connect(this, SIGNAL(clicked()), this, SLOT(select()));
}

QContent& MMSSlideAudio::document()
{
    return audioContent;
}

quint64 MMSSlideAudio::numBytes() const
{
    if(!audioData.isEmpty())
        return audioData.count();
    else if(!audioContent.isNull())
        return audioContent.size();
    return 0;
}

void MMSSlideAudio::setAudio( const QContent &doc )
{
    audioContent = doc;
    audioData.resize(0);
    audioName = QString();
    audioType = QString();

    setText(audioContent.name());

    emit changed();
}

void MMSSlideAudio::setAudio( const QByteArray &d, const QString &loc )
{
    audioContent = QContent();
    audioData = d;
    audioName = loc;

    setText(loc.toLatin1());

    emit changed();
}

QByteArray MMSSlideAudio::audio() const
{
    if (audioContent.isValid())
    {
        QIODevice* io = audioContent.open();
        audioData = io->readAll();
        delete io;
    }

    return audioData;
}

void MMSSlideAudio::select()
{
    QAudioSourceSelectorDialog *selector = new QAudioSourceSelectorDialog(this);
    selector->setObjectName("slideAudioSelector");
    selector->setDefaultAudio("audio/amr", "amr", 8000, 1);
    if (audioContent.isValid())
        selector->setContent(audioContent);
    else
        selector->setContent(QContent(audioName));
    selector->setWindowTitle(tr("Slide audio"));
    selector->setModal(true);

    int result = QtopiaApplication::execDialog( selector );
    if ( result == QDialog::Accepted ) {
        bool ok = true;
        emit aboutToChange(ok,selector->content().size()-numBytes());
        if(ok)
            setAudio(selector->content());
    }
    delete selector;
}

QString MMSSlideAudio::mimeType() const
{
    if (audioType.isEmpty()) {
        // guess
        char buf[7];
        memcpy(buf, audioData.data(), qMin(6, audioData.size()));
        buf[6] = '\0';
        QString head(buf);
        if (head == "#!AMR") {
            audioType = "audio/amr";
        } else if (head == "RIFF") {
            audioType = "audio/x-wav";
        }
    }

    return audioType;
}

void MMSSlideAudio::keyPressEvent( QKeyEvent *e )
{
    switch (e->key()) {
        case Qt::Key_Left:
            emit leftPressed();
            break;
        case Qt::Key_Right:
            emit rightPressed();
            break;
        default:
            QPushButton::keyPressEvent(e);
    }
}

bool MMSSlideAudio::isEmpty() const
{
    return !audioContent.isValid();
}

MMSSlide::MMSSlide(QWidget *parent)
    : QWidget(parent), m_duration( 5000 )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);
    l->setSpacing(0);

    m_mediaStack = new QStackedWidget(this);
    m_mediaStack->setFrameStyle(QFrame::Box);
    m_mediaStack->setFocusPolicy(Qt::StrongFocus);
    l->addWidget(m_mediaStack,6);

    m_noMediaButton = new NoMediaButton(this);
    connect(m_noMediaButton,SIGNAL(clicked()),this,SLOT(selectMedia()));
    connect(m_noMediaButton, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect(m_noMediaButton, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );

    m_mediaStack->addWidget(m_noMediaButton);
    m_mediaStack->setFocusProxy(m_noMediaButton);
    m_mediaStack->addWidget(m_noMediaButton);

    m_imageContent = new MMSSlideImage(this);
    m_mediaStack->addWidget( m_imageContent);
    connect(m_imageContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect(m_imageContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );
    connect(m_imageContent, SIGNAL(changed()),this, SLOT(mediaChanged()));
    connect(m_imageContent, SIGNAL(changed()),this, SIGNAL(changed()));
    connect(m_imageContent, SIGNAL(aboutToChange(bool&,quint64)),this, SIGNAL(aboutToChange(bool&,quint64)));

    m_videoContent = new MMSSlideVideo(this);
    m_mediaStack->addWidget(m_videoContent);
    connect(m_videoContent,SIGNAL(leftPressed()),this,SIGNAL(leftPressed()));
    connect(m_videoContent,SIGNAL(rightPressed()),this,SIGNAL(rightPressed()));
    connect(m_videoContent,SIGNAL(changed()),this,SLOT(mediaChanged()));
    connect(m_videoContent, SIGNAL(changed()),this, SIGNAL(changed()));
    connect(m_videoContent,SIGNAL(aboutToChange(bool&,quint64)),this, SIGNAL(aboutToChange(bool&,quint64)));

    m_textContent = new MMSSlideText( this );
    l->addWidget(m_textContent);
    connect(m_textContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect(m_textContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );
    connect(m_textContent, SIGNAL(changed()),this, SIGNAL(changed()));
    connect(m_textContent, SIGNAL(aboutToChange(bool&,quint64)),this, SIGNAL(aboutToChange(bool&,quint64)));

    m_audioContent = new MMSSlideAudio( this );
    l->addWidget(m_audioContent);
    connect(m_audioContent, SIGNAL(leftPressed()), this, SIGNAL(leftPressed()) );
    connect(m_audioContent, SIGNAL(rightPressed()), this, SIGNAL(rightPressed()) );
    connect(m_audioContent, SIGNAL(changed()), this, SLOT(mediaChanged()));
    connect(m_audioContent, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(m_audioContent, SIGNAL(aboutToChange(bool&,quint64)),this, SIGNAL(aboutToChange(bool&,quint64)));
}

void MMSSlide::setDuration( int t )
{
    if( t != m_duration )
    {
        m_duration = t;
        emit durationChanged( m_duration );
    }
}

int MMSSlide::duration() const
{
    return m_duration;
}

quint64 MMSSlide::numBytes() const
{
    quint64 size = 0;
    size += m_textContent->numBytes();
    size += m_imageContent->numBytes();
    size += m_videoContent->numBytes();
    size += m_audioContent->numBytes();

    return size;
}

bool MMSSlide::isEmpty() const
{
    return m_textContent->isEmpty() &&
           m_imageContent->isEmpty() &&
           m_videoContent->isEmpty() &&
           m_audioContent->isEmpty();
}

void MMSSlide::selectMedia()
{
    //MMS conformance spec 1.2 stipulates video content
    //and slide audio content are mutually exclusive.

    if(audioContent()->isEmpty())
    {
        MediaSelectionDialog::Actions actions = MediaSelectionDialog::SelectImage | MediaSelectionDialog::SelectVideo;
        MediaSelectionDialog selector(actions,this);
        selector.setModal(true);

        if(QtopiaApplication::execDialog(&selector) == QDialog::Accepted)
        {
            switch(selector.action())
            {
            case MediaSelectionDialog::SelectImage:
            {
                m_mediaStack->setCurrentWidget(m_imageContent);
                m_imageContent->select();
            }   break;
            case MediaSelectionDialog::SelectVideo:
                m_videoContent->select();
                break;
            case MediaSelectionDialog::None:
                break;
            }
        }
    }
    else
        m_imageContent->select();

    m_audioContent->setVisible(m_videoContent->isEmpty());
}

void MMSSlide::mediaChanged()
{
    if(sender() == m_imageContent)
    {
        if(!m_imageContent->isEmpty())
        {
            m_mediaStack->setCurrentWidget(m_imageContent);
            return;
        }
    }
    else if(sender() == m_videoContent)
    {
        if(!m_videoContent->isEmpty())
        {
            m_mediaStack->setCurrentWidget(m_videoContent);
            return;
        }
    }
    else if(sender() == m_audioContent)
        return;

    m_mediaStack->setCurrentWidget(m_noMediaButton);
    m_audioContent->setVisible(m_videoContent->isEmpty());
}

void MMSSlide::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    MMSSlideImage::loadImages(m_mediaStack->size());
}

MMSSlideImage *MMSSlide::imageContent() const
{
    return m_imageContent;
}

MMSSlideText *MMSSlide::textContent() const
{
    return m_textContent;
}

MMSSlideAudio *MMSSlide::audioContent() const
{
    return m_audioContent;
}

MMSSlideVideo* MMSSlide::videoContent() const
{
    return m_videoContent;
}

class SmilHandler
{
public:
    QList<MMSSmilPart> parts;
    MMSSmil smil;
    SmilHandler() : m_insidePart( false ) {}

    bool parse( QXmlStreamReader& xml )
    {
        while ( !xml.atEnd() ) {
            xml.readNext();

            if ( xml.isStartElement() ) {
                const QStringRef& name = xml.name();
                const QXmlStreamAttributes& atts = xml.attributes();

                if ( name == "smil" ) {
                    smil.fgColor = QColor();
                    smil.bgColor = QColor();
                    smil.parts.clear();
                } else if ( name == "par" ) {
                    m_insidePart = true;
                    MMSSmilPart newPart;
                    QString duration = atts.value( "dur" ).toString();
                    if ( duration.length() ) {
                        QRegExp exp( "(\\d*)(\\w*)" );
                        if ( exp.indexIn( duration ) == 0 ) {
                            newPart.duration = exp.cap( 1 ).toInt();
                            if ( exp.cap( 2 ).toLower() == "s" )
                                newPart.duration *= 1000;
                        }
                    }
                    smil.parts.append( newPart );
                } else if ( name == "region" ) {
                    if ( atts.value( "background-color" ).length() )
                        smil.bgColor.setNamedColor( atts.value( "background-color" ).toString() );
                } else if ( m_insidePart ) {
                    QString src = atts.value( "src" ).toString();
                    if ( src.length() ) {
                        if ( name == "img" )
                            smil.parts.last().image = src;
                        else if ( name == "text" )
                            smil.parts.last().text = src;
                        else if ( name == "audio" )
                            smil.parts.last().audio = src;
                        else if (name == "video" )
                            smil.parts.last().video = src;
                    }
                }
            } else if ( xml.isEndElement() ) {
                if ( xml.name() == "par" ) {
                    m_insidePart = false;
                }
            }
        }

        return !xml.hasError();
    }

private:
    bool m_insidePart;
};

static void addActionsFromWidget(QWidget* sourceWidget, QMenu* targetMenu)
{
    if(!sourceWidget) return;
    foreach(QAction* a,sourceWidget->actions())
        targetMenu->addAction(a);
}

// This logic is replicated in the SMIL viewer...
static QString smilStartMarker(const QMailMessage& mail)
{
    QMailMessageContentType type(mail.headerField("X-qtmail-internal-original-content-type"));
    if (type.isNull()) {
        type = QMailMessageContentType(mail.headerField("Content-Type"));
    }
    if (!type.isNull()) {
        QString startElement = type.parameter("start");
        if (!startElement.isEmpty())
            return startElement;
    }

    return QString("<presentation-part>");
}

static uint smilStartIndex(const QMailMessage& mail)
{
    QString startMarker(smilStartMarker(mail));

    for (uint i = 0; i < mail.partCount(); ++i)
        if (mail.partAt(i).contentID() == startMarker)
            return i;

    return 0;
}

static bool smilPartMatch(const QString identifier, const QMailMessagePart& part)
{
    // See if the identifer is a Content-ID marker
    QString id(identifier);
    if (id.toLower().startsWith("cid:"))
        id.remove(0, 4);

    return ((part.contentID() == id) || (part.displayName() == id) || (part.contentLocation() == id));
}

MMSComposerInterface::MMSComposerInterface( QWidget *parent )
    : QMailComposerInterface( parent ),
    m_widgetStack(0), m_composerWidget(0), m_detailsWidget(0),
    m_slideLabel(0), m_sizeLabel(0), m_slideStack(0),
    m_curSlide(-1), m_internalUpdate(false), m_removeSlide(0),
    m_nextSlide(0), m_previousSlide(0)
{
    init();
}

MMSComposerInterface::~MMSComposerInterface()
{
    qDeleteAll(m_slides);
}

void MMSComposerInterface::init()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    QWidget::setLayout(layout);

    //widget stack
    m_widgetStack = new QStackedWidget(this);
    layout->addWidget(m_widgetStack);

    //composer widget
    m_composerWidget = new QWidget(m_widgetStack);
    m_widgetStack->addWidget(m_composerWidget);

    QVBoxLayout *l = new QVBoxLayout(m_composerWidget);
    l->setMargin(0);

    //duration label
    m_sizeLabel= new QLabel(m_composerWidget);
    m_sizeLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    //slide number label
    m_slideLabel = new QLabel(m_composerWidget);
    m_slideLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    //slide stack
    m_slideStack = new QStackedWidget(m_composerWidget);
    l->addWidget( m_slideStack );

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget( m_slideLabel );
    labelLayout->addWidget( m_sizeLabel);
    l->addLayout( labelLayout );

    //details widget
    m_detailsWidget = new DetailsPage(m_widgetStack);
    m_detailsWidget->setType(QMailMessage::Mms);
    connect( m_detailsWidget, SIGNAL(changed()), this, SIGNAL(changed()));
    connect( m_detailsWidget, SIGNAL(sendMessage()), this, SIGNAL(sendMessage()));
    connect( m_detailsWidget, SIGNAL(cancel()), this, SIGNAL(cancel()));
    connect( m_detailsWidget, SIGNAL(editMessage()), this, SLOT(composePage()));
    m_widgetStack->addWidget(m_detailsWidget);

    QWidget::setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    connect( this, SIGNAL(currentChanged(uint)), this, SLOT(updateLabels()) );
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);

    //menus
    m_removeSlide = new QAction(tr("Remove slide"), this);
    connect(m_removeSlide, SIGNAL(triggered()), this, SLOT(removeSlide()));

    m_nextSlide = new QAction(tr("Next slide"), this);
    connect(m_nextSlide, SIGNAL(triggered()), this, SLOT(nextSlide()));

    m_previousSlide = new QAction(tr("Previous slide"), this);
    connect(m_previousSlide, SIGNAL(triggered()), this, SLOT(previousSlide()));

    QAction *add = new QAction(tr("Add slide"), this);
    connect(add, SIGNAL(triggered()), this, SLOT(addSlide()));

    QAction* options = new QAction(tr("Slide options..."), this);
    connect(options, SIGNAL(triggered()), this, SLOT(slideOptions()));

    QMenu* menu = QSoftMenuBar::menuFor(this);
    menu->addSeparator();
    menu->addAction(add);
    menu->addAction(m_removeSlide);
    menu->addAction(m_nextSlide);
    menu->addAction(m_previousSlide);
    menu->addSeparator();
    menu->addAction(options);
    addActionsFromWidget(QWidget::parentWidget(),menu);

    setContext("Create " + displayName(QMailMessage::Mms));
    composePage();
    addSlide();
}

MMSSmil MMSComposerInterface::parseSmil( const QString &smil )
{
    QXmlStreamReader reader;
    reader.addData( smil );

    SmilHandler handler;
    if( !handler.parse( reader ) )
        qWarning( "MMSComposer unable to parse smil message." );
    return handler.smil;
}

void MMSComposerInterface::setContext(const QString& title)
{
    m_title = title;
    emit contextChanged();
}

bool MMSComposerInterface::isEmpty() const
{
    for( uint i = 0 ; i < slideCount() ; ++i )
        if( !slide(i)->imageContent()->isEmpty() ||
            !slide(i)->textContent()->isEmpty() ||
            !slide(i)->audioContent()->isEmpty() ||
            !slide(i)->videoContent()->isEmpty()) {
            return false;
        }
    return true;
}

static bool isForwardLocked(const QMailMessagePart& part)
{
    return !part.headerField("X-Mms-Forward-Locked").isNull();
}

void MMSComposerInterface::setMessage( const QMailMessage &mail )
{
    clear();
    MMSSlide *curSlide = slide( currentSlide() );

    if (mail.partCount() > 1)
    {
        // This message must contain SMIL
        uint smilPartIndex = smilStartIndex(mail);
        MMSSmil smil = parseSmil( mail.partAt( smilPartIndex ).body().data() );

        // For each SMIL slide...
        int numSlides = 0;
        foreach( const MMSSmilPart& smilPart, smil.parts )
        {
            if( numSlides )
                addSlide();

            MMSSlide *curSlide = slide( slideCount() -1 );
            curSlide->setDuration( smilPart.duration );

            // ...for each part in the message...
            for( uint i = 0 ; i < mail.partCount(); ++i ) {
                if(i == smilPartIndex)
                    continue;

                const QMailMessagePart& part = mail.partAt( i );

                //don't add forward locked data
                if(isForwardLocked(part))
                    continue;

                QString fileName(part.attachmentPath());

                // ...see if this part is used in this slide
                if (smilPartMatch(smilPart.text, part)) {
                    QString t = part.body().data();
                    curSlide->textContent()->setText( t );
                } else if (smilPartMatch(smilPart.image, part)) {
                    if (!fileName.isEmpty()) {
                        curSlide->imageContent()->setImage( QContent(fileName) );
                    } else {
                        QPixmap pix;
                        pix.loadFromData(part.body().data(QMailMessageBody::Decoded));
                        curSlide->imageContent()->setImage( pix );
                    }
                } else if(smilPartMatch(smilPart.video, part)) {

                    if(!fileName.isEmpty())
                        curSlide->videoContent()->setVideo(QContent(fileName));
                    else
                    {
                        QByteArray data = part.body().data(QMailMessageBody::Decoded);
                        curSlide->videoContent()->setVideo( data, part.displayName() );
                    }

                } else if (smilPartMatch(smilPart.audio, part)) {
                    if (!fileName.isEmpty()) {
                        curSlide->audioContent()->setAudio( QContent(fileName) );
                    } else {
                        QByteArray data = part.body().data(QMailMessageBody::Decoded);
                        curSlide->audioContent()->setAudio( data, part.displayName() );
                    }
                }
            }

            bool isEmptySlide = curSlide->isEmpty() && numSlides;
            if(isEmptySlide)
                removeSlide();
            else
                ++numSlides;
        }
        if( smil.bgColor.isValid() )
            setBackgroundColor( smil.bgColor );
        if( smil.fgColor.isValid() )
            setTextColor( smil.fgColor );
    } else {
        QString bodyData(mail.body().data());
        if (!bodyData.isEmpty()) {
            curSlide->textContent()->setText(bodyData);
        }
        for (uint i = 0 ; i < mail.partCount() ; ++i) {
            QMailMessagePart part = mail.partAt( i );

            //don't add forward locked data
            if(isForwardLocked(part))
                continue;

            QMailMessageContentType contentType = part.contentType();
            QString fileName(part.attachmentPath());

            if( contentType.type().toLower() == "text" ) {
                QString t = part.body().data();
                curSlide->textContent()->setText( t );
            } else if( contentType.type().toLower() == "image" ) {
                if (!fileName.isEmpty()) {
                    curSlide->imageContent()->setImage( QContent(fileName) );
                } else {
                    QPixmap pix;
                    pix.loadFromData(part.body().data(QMailMessageBody::Decoded));
                    curSlide->imageContent()->setImage( pix );
                }
            } else if( contentType.type().toLower() == "audio" ) {
                if (!fileName.isEmpty()) {
                    curSlide->audioContent()->setAudio( QContent(fileName) );
                } else {
                    QByteArray data = part.body().data(QMailMessageBody::Decoded);
                    curSlide->audioContent()->setAudio( data, part.displayName() );
                }
            } else if(contentType.type().toLower() == "video") {
                if (!fileName.isEmpty()) {
                    curSlide->videoContent()->setVideo( QContent(fileName) );
                } else {
                    QByteArray data = part.body().data(QMailMessageBody::Decoded);
                    curSlide->videoContent()->setVideo( data, part.displayName() );
                }
            } else {
                qWarning() << "Unhandled MMS part:" << part.displayName();
            }
        }
    }

    m_removeSlide->setVisible(slideCount() > 1);
    m_nextSlide->setVisible(slideCount() > 1);
    m_previousSlide->setVisible(false);
    setCurrentSlide(0);

    //set the details
    m_detailsWidget->setDetails(mail);
}

typedef struct PartDetailsData
{
    QString filename;
    QByteArray contentType;
    QByteArray name;
    QByteArray data;

    bool operator==(const PartDetailsData& other)
    {
        return filename == other.filename &&
               contentType == other.contentType &&
               name == other.name &&
               data == other.data;
    };

    static PartDetailsData fromFile(const QString& filename, const QString& contentType)
    {
        PartDetailsData d;
        d.filename = filename;
        d.contentType = contentType.toLatin1();
        return d;
    }

    static PartDetailsData fromData(const QString& contentType, const QString& name, const QByteArray& data)
    {
        PartDetailsData d;
        d.contentType = contentType.toLatin1();
        d.name = name.toLatin1();
        d.data = data;
        return d;
    }

    bool isFileData() const { return !filename.isEmpty(); };

} PartDetails;

QMailMessage MMSComposerInterface::message() const
{
    QMailMessage mmsMail;

    QList<PartDetails> partDetails;

    //clean slate, generate document
    static const QString docTemplate =
    "<smil>\n"
    "   <head>\n"
    "       <meta name=\"title\" content=\"mms\"/>\n"
    "       <meta name=\"author\" content=\"%1\"/>\n"
    "       <layout>\n"
    "%2"
    "       </layout>\n"
    "   </head>\n"
    "   <body>\n"
    "%3"
    "   </body>\n"
    "</smil>\n"
    ; // 1.author 2.rootlayout&regions 3.parts
    static const QString rootLayoutTemplate =
    "           <root-layout width=\"%1\" height=\"%2\"/>\n"
    ; // 1.width 2.height
    static const QString regionTemplate =
    "           <region id=\"%1\" width=\"%2\" height=\"%3\" left=\"%4\" top=\"%5\"%6/>\n"
    ; // 1.id 2.width 3.height 4.left 5.top 6.background-color-spec
    static const QString partTemplate =
    "      <par dur=\"%1\">\n"
    "%2"
    "      </par>\n"
    ; // 1.duration 2.contentitems
    static const QString imageTemplate =
    "         <img src=\"%1\" region=\"%2\"/>\n"
    ; // 1.src 2.region
    static const QString videoTemplate =
    "         <video src=\"%1\" region=\"%2\"/>\n"

    ; // 1.src 2.region
    static const QString textTemplate =
    "         <text src=\"%1\" region=\"%2\"/>\n"
    ; // 1.src 2.region
    static const QString audioTemplate =
    "         <audio src=\"%1\"/>\n"
    ; // 1.src

    /*
       if the composer only contains one piece of content
       either an image or some text, then we don't need to
       generate smil output
    */
    int contentCount = 0;
    MMSSlideImage *imageContent = 0;
    MMSSlideText *textContent = 0;
    MMSSlideAudio *audioContent = 0;
    MMSSlideVideo* videoContent = 0;
    for( uint s = 0 ; s < slideCount() ; ++s )
    {
        MMSSlide *curSlide = slide( s );
        if( !curSlide->imageContent()->isEmpty() )
        {
            imageContent = curSlide->imageContent();
            ++contentCount;
        }
        if( !curSlide->textContent()->isEmpty() )
        {
            textContent = curSlide->textContent();
            ++contentCount;
        }
        if( !curSlide->audioContent()->isEmpty() )
        {
            audioContent = curSlide->audioContent();
            ++contentCount;
        }
        if( !curSlide->videoContent()->isEmpty() )
        {
            videoContent = curSlide->videoContent();
            ++contentCount;
        }

        if( contentCount > 1 )
            break;
    }
    if( contentCount == 1 )
    {
        // Don't write a SMIL presentation - add the single part to the message
        if( textContent ) {
            // Add the text as the message body
            QMailMessageContentType type("text/plain; charset=UTF-8");
            mmsMail.setBody(QMailMessageBody::fromData(textContent->text(), type, QMailMessageBody::Base64));
        } else if( imageContent ) {
            if (imageContent->document().isValid()) {
                // Add the image as an attachment
                mmsMail.setMultipartType(QMailMessagePartContainer::MultipartMixed);
                PartDetails imagePartDetails = PartDetails::fromFile(imageContent->document().fileName(),
                                                                         imageContent->mimeType());
                partDetails.append(imagePartDetails);
            } else {
                // Write the image out to a buffer in JPEG format
                QBuffer buffer;
                imageContent->image().save(&buffer,imageFormat(),imageQuality);

                // Add the image data as the message body
                QMailMessageContentType type("image/jpeg");
                mmsMail.setBody(QMailMessageBody::fromData(buffer.data(), type, QMailMessageBody::Base64));
            }
        } else if(videoContent) {
            if (videoContent->document().isValid()) {
                // Add the image as an attachment
                mmsMail.setMultipartType(QMailMessagePartContainer::MultipartMixed);
                PartDetails videoPartDetails = PartDetails::fromFile(videoContent->document().fileName(),
                                                                         videoContent->mimeType());
                partDetails.append(videoPartDetails);
            } else {
                //assumes that the video content is not contained within the content system therefore cannot be added as
                //an attachment and must be added from byte data.

                // Add the video data as the message body
                QMailMessageContentType type( videoContent->mimeType().toLatin1() );
                mmsMail.setBody(QMailMessageBody::fromData(videoContent->video(), type, QMailMessageBody::Base64));
            }

        }
        else if (audioContent) {
            if (audioContent->document().isValid()) {
                // Add the audio as an attachment
                mmsMail.setMultipartType(QMailMessagePartContainer::MultipartMixed);
                PartDetails audioPartDetails = PartDetails::fromFile(audioContent->document().fileName(),
                                                                         audioContent->mimeType());
                partDetails.append(audioPartDetails);
            } else {
                // Add the audio data as the message body
                QMailMessageContentType type( audioContent->mimeType().toLatin1() );
                mmsMail.setBody(QMailMessageBody::fromData(audioContent->audio(), type, QMailMessageBody::Base64));
            }
        }
    }
    else if( contentCount > 1 )
    {
        // Generate the full SMIL show
        QString parts;
        QRect largestText, largestImage;

        for( int s = 0 ; s < static_cast<int>(slideCount()) ; ++s )
        {
            MMSSlide *curSlide = slide( s );
            imageContent = curSlide->imageContent();
            textContent = curSlide->textContent();
            audioContent = curSlide->audioContent();
            videoContent = curSlide->videoContent();

            QString part;
            if( !textContent->isEmpty() )
            {
                QRect cr = textContent->contentsRect();
                if( cr.height() > largestText.height() )
                    largestText.setHeight( cr.height() );

                QString textFileName = "mmstext" + QString::number( s ) + ".txt";
                part += textTemplate.arg( Qt::escape(textFileName) ).arg( "text" );

                // Write the text to a buffer in UTF-8 format
                QByteArray buffer;
                {
                    QTextStream out(&buffer);
                    out.setCodec("UTF-8");
                    out << textContent->text();
                }

                const QByteArray type("text/plain; charset=UTF-8");

                PartDetails details = PartDetails::fromData(type,textFileName,buffer);
                if (!partDetails.contains(details))
                    partDetails.append(details);
            }

            if( !videoContent->isEmpty())
            {
                //TODO
                //check if we need to do something with the video dimensions etc as in the image section
                QString videoFileName;
                if (videoContent->document().isValid()) {
                    videoFileName = videoContent->document().fileName();

                    PartDetails details = PartDetails::fromFile(videoFileName,videoContent->mimeType());
                    if (!partDetails.contains(details))
                        partDetails.append(details);

                    QFileInfo fi(videoFileName);
                    videoFileName = fi.fileName();

                } else {
                    QMimeType mimeType(videoContent->mimeType());
                    QString ext = mimeType.extension();
                    videoFileName = "mmsvideo" + QString::number( s ) + '.' + ext;

                    QByteArray data  = videoContent->video();

                    PartDetails details = PartDetails::fromData(mimeType.id(), videoFileName,data);
                    if (!partDetails.contains(details))
                        partDetails.append(details);
                }
                //add the smil part with the region identifier
                part += videoTemplate.arg( Qt::escape(videoFileName) ).arg("image");
            }
            if( !imageContent->isEmpty() )
            {
                QRect cr = imageContent->contentsRect();
                if( cr.width() > largestImage.width() )
                    largestImage.setWidth( cr.width() );
                if( cr.height() > largestImage.height() )
                    largestImage.setHeight( cr.height() );

                QString imgFileName;
                if (imageContent->document().isValid()) {
                    imgFileName = imageContent->document().fileName();

                    PartDetails details = PartDetails::fromFile(imgFileName,imageContent->mimeType());
                    if (!partDetails.contains(details))
                        partDetails.append(details);

                    QFileInfo fi(imgFileName);
                    imgFileName = fi.fileName();
                } else {
                    imgFileName = "mmsimage" + QString::number( s ) + ".jpg";

                    // Write the data to a buffer in JPEG format
                    QBuffer buffer;
                    imageContent->image().save(&buffer,imageFormat(),imageQuality);

                    const QString type("image/jpeg");

                    PartDetails details = PartDetails::fromData(type,imgFileName,buffer.data());
                    if (!partDetails.contains(details))
                        partDetails.append(details);
                }

                part += imageTemplate.arg( Qt::escape(imgFileName) ).arg( "image" );
            }
            if( !audioContent->isEmpty() ) {
                QString audioFileName;
                if (audioContent->document().isValid()) {
                    audioFileName = audioContent->document().fileName();

                    PartDetails details = PartDetails::fromFile(audioFileName,audioContent->mimeType());
                    if (!partDetails.contains(details))
                        partDetails.append(details);

                    QFileInfo fi(audioFileName);
                    audioFileName = fi.fileName();
                } else {
                    QMimeType mimeType(audioContent->mimeType());
                    QString ext = mimeType.extension();
                    audioFileName = "mmsaudio" + QString::number( s ) + '.' + ext;

                    PartDetails details = PartDetails::fromData(mimeType.id(),audioFileName,audioContent->audio());
                    if (!partDetails.contains(details))
                        partDetails.append(details);
                }

                part += audioTemplate.arg( Qt::escape(audioFileName) );
            }
            if( !part.isEmpty() )
            {
                part = partTemplate.arg( QString::number( curSlide->duration() ) + "ms" ).arg( part );
                parts += part;
            }
        }

        QRect imageRect;
        imageRect.setX( 0 );
        imageRect.setY( 0 );
        imageRect.setWidth( contentsRect().width() );
        imageRect.setHeight( contentsRect().height() - largestText.height() );

        largestText.setX( 0 );
        int h = largestText.height();
        largestText.setY( imageRect.height() );
        largestText.setHeight( h );
        largestText.setWidth(contentsRect().width());

        QString regions;
        QString backgroundParam;
        if( backgroundColor().isValid() )
        {
            backgroundParam = QString(" background-color=\"%1\"").arg(backgroundColor().name().toUpper());
        }
        if( imageRect.width() > 0 && imageRect.height() > 0 )
        {
            regions += regionTemplate.arg( "image" )
                                     .arg( "100%" )
                                     .arg( "100%" )
                                     .arg( imageRect.x() )
                                     .arg( imageRect.y() )
                                     .arg( backgroundParam );
        }
        if( largestText.width() > 0 && largestText.height() > 0 )
        {
            regions += regionTemplate.arg( "text" )
                                     .arg( "100%" )
                                     .arg( "100%" )
                                     .arg( largestText.x() )
                                     .arg( largestText.y() )
                                     .arg( backgroundParam );
        }

        QString rootLayout = rootLayoutTemplate.arg( contentsRect().width() ).arg( contentsRect().height() );
        QString doc = docTemplate.arg( Qtopia::ownerName() ) .arg( rootLayout + regions ) .arg( parts );

        // add the smil document to the message
        mmsMail.setMultipartType( QMailMessage::MultipartRelated );

        QMailMessagePart docPart;
        docPart.setContentID( "<presentation-part>" );

        QMailMessageContentType type("application/smil");
        docPart.setBody( QMailMessageBody::fromData(doc.toLatin1(), type, QMailMessageBody::EightBit, QMailMessageBody::AlreadyEncoded));

        mmsMail.appendPart( docPart );
    }

    mmsMail.setMessageType( QMailMessage::Mms );
    mmsMail.setHeaderField( "X-Mms-Message-Type", "m-send-req" );

    // add binary data as mail attachments
    foreach (const PartDetails& details, partDetails) {
        QMailMessagePart part;
        QByteArray identifier;

        if (details.isFileData()) {
            // Ensure that the nominated file is accessible
            QFileInfo fi(details.filename);
            QString path = fi.absoluteFilePath();

            {
                QFile f(path);
                if( !f.open( QIODevice::ReadOnly ) ) {
                    qWarning() << "Could not open MMS attachment for reading" << path;
                    continue;
                }
            }

            // Add this part from the file
            identifier = fi.fileName().toLatin1();

            QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
            disposition.setFilename(identifier);

            //set content type explicity
            QMailMessageContentType type(details.contentType);
            type.setName(identifier);

            part = QMailMessagePart(QMailMessagePart::fromFile(path, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding));
        } else {
            // Add the part from the data
            identifier = details.name;

            QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
            disposition.setFilename(identifier);

            QMailMessageContentType type(details.contentType);
            type.setName(identifier);

            part = QMailMessagePart(QMailMessagePart::fromData(details.data, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding));
        }

        // Set the Content-ID and Content-Location fields to prevent them being synthesized by MMSC
        part.setContentID(identifier);
        part.setContentLocation(identifier);

        mmsMail.appendPart(part);
    }

    m_detailsWidget->getDetails(mmsMail);

    return mmsMail;
}

void MMSComposerInterface::attach( const QContent &lnk, QMailMessage::AttachmentsAction action )
{
    if (action != QMailMessage::LinkToAttachments) {
        // TODO: handle temporary files
        qWarning() << "MMS: Unable to handle attachment of transient document!";
        return;
    }

    if (!slideCount())
        addSlide();

    MMSSlide *curSlide = slide( slideCount()-1 );

    if (lnk.type().startsWith("image/")) {
        if (lnk.isValid()) {
            if (!curSlide->imageContent()->isEmpty()) {
                // If there is already an image in the last slide, add a new one
                addSlide();
                curSlide = slide( slideCount()-1 );
            }
            curSlide->imageContent()->setImage(lnk);
        }
    } else if(lnk.type() == "video/"){

        if (!curSlide->videoContent()->isEmpty()) {
            // If there is already audio in the last slide, add a new one
            addSlide();
            curSlide = slide( slideCount()-1 );
        }
        curSlide->videoContent()->setVideo(lnk);

    } else if (lnk.type() == "text/plain") {
        QFile file(lnk.fileName());
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            if (!curSlide->textContent()->isEmpty()) {
                // If there is already text in the last slide, add a new one
                addSlide();
                curSlide = slide( slideCount()-1 );
            }
            curSlide->textContent()->setText(ts.readAll());
        }
    } else if (lnk.type().startsWith("audio/")) {
        if (!curSlide->audioContent()->isEmpty()) {
            // If there is already audio in the last slide, add a new one
            addSlide();
            curSlide = slide( slideCount()-1 );
        }
        curSlide->audioContent()->setAudio(lnk);
    } else {
        // TODO: deal with other attachments
    }
}

void MMSComposerInterface::clear()
{
    while( slideCount() > 1 )
        removeSlide( slideCount() - 1 );
    if( slideCount() )
    {
        MMSSlide *cur = slide(currentSlide());
        cur->imageContent()->setImage(QContent());
        cur->textContent()->setText(QString());
        cur->audioContent()->setAudio(QContent());
        cur->videoContent()->setVideo(QContent());
    }
}

int MMSComposerInterface::currentSlide() const
{
    return m_curSlide;
}

uint MMSComposerInterface::slideCount() const
{
    return m_slides.count();
}

MMSSlide *MMSComposerInterface::slide( uint slide ) const
{
    if( slide >= slideCount() )
        return 0;
    return m_slides.at(slide);
}

QRect MMSComposerInterface::contentsRect() const
{
    QRect r = rect();
    r.setHeight( r.height() - qMax( m_slideLabel->height(), m_sizeLabel->height() ) );
    return r;
}


void MMSComposerInterface::setDefaultAccount(const QMailAccountId& id)
{
    m_detailsWidget->setDefaultAccount(id);
}

void MMSComposerInterface::setTo(const QString& toAddress)
{
    m_detailsWidget->setTo(toAddress);
}

void MMSComposerInterface::setFrom(const QString& fromAddress)
{
    m_detailsWidget->setFrom(fromAddress);
}

void MMSComposerInterface::setCc(const QString& ccAddress)
{
    m_detailsWidget->setCc(ccAddress);
}

void MMSComposerInterface::setBcc(const QString& bccAddress)
{
    m_detailsWidget->setBcc(bccAddress);
}

void MMSComposerInterface::setSubject(const QString& subject)
{
    m_detailsWidget->setSubject(subject);
}

QString MMSComposerInterface::from() const
{
    return m_detailsWidget->from();
}

QString MMSComposerInterface::to() const
{
    return m_detailsWidget->to();
}

QString MMSComposerInterface::cc() const
{
    return m_detailsWidget->cc();
}

QString MMSComposerInterface::bcc() const
{
    return m_detailsWidget->bcc();
}

bool MMSComposerInterface::isReadyToSend() const
{
    return !to().trimmed().isEmpty() ||
           !cc().trimmed().isEmpty() ||
           !bcc().trimmed().isEmpty();
}

bool MMSComposerInterface::isDetailsOnlyMode() const
{
    return m_detailsWidget->isDetailsOnlyMode();
}

void MMSComposerInterface::setDetailsOnlyMode(bool val)
{
    m_detailsWidget->setDetailsOnlyMode(val);
    if(val)
        detailsPage();
}

QString MMSComposerInterface::contextTitle() const
{
    return m_title;
}

QMailAccount MMSComposerInterface::fromAccount() const
{
    return m_detailsWidget->fromAccount();
}

quint64 MMSComposerInterface::numBytes() const
{
    quint64 size = 0;
    foreach(MMSSlide* slide, m_slides)
        size += slide->numBytes();

    return size;
}

void MMSComposerInterface::addSlide()
{
    addSlide( -1 );
}

void MMSComposerInterface::addSlide( int slideIndex )
{
    int count = static_cast<int>(slideCount());

    if (slideIndex < 0) {
        if (currentSlide() == -1)
            slideIndex = 0;
        else
            slideIndex = currentSlide();
    } else if (slideIndex >= count) {
        slideIndex = count - 1;
    }

    if (count)
        ++slideIndex; // add to the next slide

    MMSSlide *newSlide = new MMSSlide(m_slideStack);
    connect(newSlide, SIGNAL(leftPressed()), this, SLOT(previousSlide()));
    connect(newSlide, SIGNAL(rightPressed()), this, SLOT(nextSlide()));
    connect(newSlide, SIGNAL(durationChanged(int)), this, SLOT(updateLabels()));
    connect(newSlide, SIGNAL(aboutToChange(bool&,quint64)),this,SLOT(slideAboutToChange(bool&,quint64)));
    connect(newSlide, SIGNAL(changed()),this,SLOT(updateLabels()));

    m_slideStack->addWidget(newSlide);
    m_slides.insert(slideIndex, newSlide);
    ++count;

    QMenu *thisMenu = QSoftMenuBar::menuFor(this);
    QSoftMenuBar::addMenuTo(newSlide, thisMenu);
    QSoftMenuBar::addMenuTo(newSlide->m_textContent, thisMenu);
    QSoftMenuBar::addMenuTo(newSlide->m_imageContent, thisMenu);

    connect(newSlide->m_textContent, SIGNAL(textChanged()), this, SLOT(elementChanged()));
    connect(newSlide->m_imageContent, SIGNAL(changed()), this, SLOT(elementChanged()));
    connect(newSlide->m_audioContent, SIGNAL(changed()), this, SLOT(elementChanged()));
    connect(newSlide->m_videoContent, SIGNAL(changed()), this, SLOT(elementChanged()));

    m_removeSlide->setVisible(count > 1);
    m_nextSlide->setVisible(slideIndex < (count - 1));
    m_previousSlide->setVisible(slideIndex > 0);

    m_internalUpdate = true;
    setCurrentSlide(slideIndex);
}

void MMSComposerInterface::setCurrentSlide( int slideIndex )
{
    int count = static_cast<int>(slideCount());
    if (slideIndex >= count)
        return;

    if (slideIndex < 0) {
        m_curSlide = -1;
        return;
    }

    if (m_internalUpdate || slideIndex != m_curSlide) {
        m_internalUpdate = false;
        m_curSlide = slideIndex;
        MMSSlide* currentSlide = slide(m_curSlide);
        m_slideStack->setCurrentWidget(currentSlide);
        m_composerWidget->setFocusProxy(currentSlide->m_mediaStack->currentWidget());
        emit currentChanged(m_curSlide);
    }
}

void MMSComposerInterface::setBackgroundColor( const QColor &col )
{
    m_backgroundColor = col;

    // Set the FG to a contrasting colour
    int r, g, b;
    col.getRgb(&r, &g, &b);
    m_textColor = (((r + g + b) / 3) > 128 ? Qt::black : Qt::white);

    QPalette pal = m_slideStack->palette();
    pal.setColor( QPalette::Background, m_backgroundColor );
    pal.setColor( QPalette::Base, m_backgroundColor );
    pal.setColor( QPalette::Foreground, m_textColor );
    pal.setColor( QPalette::Text, m_textColor );
    m_slideStack->setPalette( pal );
}

void MMSComposerInterface::setTextColor( const QColor &col )
{
    m_textColor = col;
    QPalette pal = m_slideStack->palette();
    pal.setColor( QPalette::Foreground, m_textColor );
    pal.setColor( QPalette::Text, m_textColor );
    m_slideStack->setPalette( pal );
}

QColor MMSComposerInterface::backgroundColor() const
{
    return m_backgroundColor;
}

void MMSComposerInterface::removeSlide()
{
    removeSlide(-1);
}

void MMSComposerInterface::removeSlide( int slideIndex )
{
    int count = static_cast<int>(slideCount());
    if (count <= 1)
        return;

    if (slideIndex == -1)
        slideIndex = currentSlide();
    if (slideIndex < 0 || slideIndex >= count)
        return;

    m_slideStack->removeWidget(slide(slideIndex));
    delete m_slides.takeAt(slideIndex);
    --count;

    if (slideIndex >= count)
        slideIndex = count - 1;
    if (slideIndex >= 0)
        m_internalUpdate = true;
    setCurrentSlide(slideIndex);

    m_removeSlide->setVisible(count > 1);
    m_nextSlide->setVisible(slideIndex < (count - 1));
    m_previousSlide->setVisible(slideIndex > 0);
}

void MMSComposerInterface::updateLabels()
{
    QString baseLabel = tr("Slide %1 of %2");

    float messageSize = numBytes() / static_cast<float>(kilobyte);

    m_slideLabel->setText( baseLabel.arg( QString::number( currentSlide()+1 ) )
                           .arg( QString::number( slideCount() ) ));
    m_sizeLabel->setText(QString("(%1kB)").arg(messageSize,0,'f',1));
}

void MMSComposerInterface::slideOptions()
{
    MMSSlide *cur = slide( currentSlide() );
    if( !cur )
        return;
    QDialog *dlg = new QDialog(this);
    dlg->setModal(true);
    dlg->setWindowTitle( tr("Slide options") );
    QGridLayout *l = new QGridLayout( dlg );
    int rowCount = 0;

    QSpinBox *durBox = new QSpinBox( dlg );
    durBox->setMinimum( 1 );
    durBox->setMaximum( 10 );
    durBox->setValue( cur->duration()/1000 );
    durBox->setSuffix( tr("secs") );
    QLabel *la = new QLabel( tr("Duration", "duration between images in a slide show"), dlg );
    la->setBuddy( durBox );
    l->addWidget( la, rowCount, 0 );
    l->addWidget( durBox, rowCount, 1 );
    ++rowCount;

    QColorButton *bg = new QColorButton( dlg );
    bg->setColor( backgroundColor() );
    la = new QLabel( tr("Slide color"), dlg );
    la->setBuddy( bg );
    l->addWidget( la, rowCount, 0 );
    l->addWidget( bg, rowCount, 1 );

    int r = QtopiaApplication::execDialog( dlg );
    if( r == QDialog::Accepted && bg->color().isValid() )
    {
        setBackgroundColor( bg->color() );

        cur->setDuration( durBox->value()*1000 );
    }
}

void MMSComposerInterface::elementChanged()
{
    QSoftMenuBar::setLabel(this, Qt::Key_Back, (isEmpty() ? QSoftMenuBar::Cancel : QSoftMenuBar::Next));

    emit changed();
}

void MMSComposerInterface::detailsPage()
{
    if(isEmpty() && !isDetailsOnlyMode())
        emit cancel();
    else
    {
        m_widgetStack->setCurrentWidget(m_detailsWidget);
        QWidget::setFocusProxy(m_detailsWidget);
        m_detailsWidget->setFocus();
        setContext(displayName(QMailMessage::Mms) + " " + tr("details", "<MMS> details" ));
    }
}

void MMSComposerInterface::composePage()
{
    m_widgetStack->setCurrentWidget(m_composerWidget);
    QWidget::setFocusProxy(m_composerWidget);
    setContext("Create " + displayName(QMailMessage::Mms));
}

void MMSComposerInterface::slideAboutToChange(bool& ok, quint64 sizeDelta)
{
    //determine if the message will exceed maximum size for its class

    quint64 predictedSize = numBytes() + sizeDelta;
    static const quint64 maxSize = maxMessageSize * kilobyte;

    ok = predictedSize <= maxSize;
    if(!ok)
    {
        static const QString msg("Unable to insert media. Maximum message size is %1kB.");
        QMessageBox::critical(this, "Error",msg.arg(maxMessageSize));
    }
}

void MMSComposerInterface::nextSlide()
{
    int count = static_cast<int>(slideCount());
    if (!count)
        return;

    int cur = currentSlide();
    if (cur == -1 || ++cur >= count)
        cur = 0;
    setCurrentSlide(cur);

    m_nextSlide->setVisible(cur != (count - 1));
    m_previousSlide->setVisible(cur != 0);
}

void MMSComposerInterface::previousSlide()
{
    int count = static_cast<int>(slideCount());
    if (!count)
        return;

    int cur = currentSlide();
    --cur;
    if (cur < 0)
        cur = count - 1;
    setCurrentSlide(cur);

    m_nextSlide->setVisible(cur != (count - 1));
    m_previousSlide->setVisible(cur != 0);
}

void MMSComposerInterface::reply(const QMailMessage& source, int action)
{
    const QString fwdIndicator(tr("Fwd"));
    const QString shortFwdIndicator(tr("Fw", "2 letter short version of Fwd for forward"));
    const QString replyIndicator(tr("Re"));

    const QString subject = source.subject().toLower();

    QString toAddress;
    QString fromAddress;
    QString ccAddress;
    QString subjectText;

    QMailMessage mail;

    if (source.parentAccountId().isValid()) {
        QMailAccount sendingAccount(source.parentAccountId());

        if (sendingAccount.id().isValid()) {
            AccountConfiguration config(sendingAccount.id());
            fromAddress = config.emailAddress();
        }
    }

    // work out the kind of mail to response
    // type of reply depends on the type of message
    // a reply to an mms is just a new mms message with the sender as recipient
    // a reply to an sms is a new sms message with the sender as recipient


    // MMS
    if (action == Forward) {
        // Copy the existing mail
        mail = source;
        mail.setId(QMailMessageId());

        if ((subject.left(fwdIndicator.length() + 1) == (fwdIndicator.toLower() + ":")) ||
            (subject.left(shortFwdIndicator.length() + 1) == (shortFwdIndicator.toLower() + ":"))) {
            subjectText = source.subject();
        } else {
            subjectText = fwdIndicator + ": " + source.subject();
        }
        setMessage( mail );

        detailsPage();
    } else {
        if (subject.left(replyIndicator.length() + 1) == (replyIndicator.toLower() + ":")) {
            subjectText = source.subject();
        } else {
            subjectText = replyIndicator + ": " + source.subject();
        }

        QMailAddress replyAddress(source.replyTo());
        if (replyAddress.isNull())
            replyAddress = source.from();

        toAddress = replyAddress.address();
    }

    if (!toAddress.isEmpty())
        setTo( toAddress );
    if (!fromAddress.isEmpty())
        setFrom( fromAddress );
    if (!ccAddress.isEmpty())
        setCc( ccAddress );
    if (!subjectText.isEmpty())
        setSubject( subjectText );

    QString task;
    if ((action == Create) || (action == Forward)) {
        task = (action == Create ? tr("Create") : tr("Forward"));
        task += " " + displayName(QMailMessage::Mms);
    } else if (action == Reply) {
        task = tr("Reply");
    } else if (action == ReplyToAll) {
        task = tr("Reply to all");
    }
    setContext(task);

}

void MMSComposerInterface::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Back) {
        e->accept();
        detailsPage();
        return;
    }

    QWidget::keyPressEvent(e);
}


QTOPIA_EXPORT_PLUGIN( MMSComposerPlugin )

MMSComposerPlugin::MMSComposerPlugin()
    : QMailComposerPlugin()
{
}

QMailComposerInterface* MMSComposerPlugin::create( QWidget *parent )
{
    return new MMSComposerInterface( parent );
}

#include <mmscomposer.moc>

