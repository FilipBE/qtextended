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

#include "videomailcomposer.h"

#include <qmailmessage.h>
#include <qmailstore.h>
#include <QLineEdit>
#include <QToolButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <qmailaccount.h>
#include <private/homewidgets_p.h>
#include <private/qtopiainputdialog_p.h>
#include <QStackedWidget>
#include <QDebug>
#include <QContent>
#include <QMediaVideoControl>
#include <QMediaControl>
#include <QMediaContentContext>
#include <QMediaSeekWidget>
#include <QDSAction>
#include <QMimeType>
#include <private/addressselectorwidget_p.h>
#include <private/accountconfiguration_p.h>

static QString tempFileTemplate(Qtopia::documentDir()+"videomail.XXXXXX.mp4");
static QString timeFormat("m:ss");

static QIcon playIcon()
{
    static QIcon *icon = 0;
    if (!icon)
        icon = new QIcon(":icon/play");

    return *icon;
}

static QIcon stopIcon()
{
    static QIcon *icon = 0;
    if (!icon)
        icon = new QIcon(":icon/stop");

    return *icon;
}


class SeekWidget : public QWidget
{
    Q_OBJECT

public:
    SeekWidget(QWidget* parent = 0);

public slots:
    void setLength(quint32 val);
    void setPosition(quint32 val);
    void reset();

signals:
    void beginSeek();
    void endSeek(quint32 val);

private slots:
    void sliderPressed();
    void sliderReleased();

private:
    void init();

private:
    QSlider* m_timeSlider;
    QLabel* m_timeLabel;
};

SeekWidget::SeekWidget(QWidget* parent)
    :
    QWidget(parent),
    m_timeSlider(0),
    m_timeLabel(0)
{
    init();
}

void SeekWidget::setLength(quint32 val)
{
    m_timeSlider->setRange(0,val);
    m_timeSlider->setValue(0);
}

void SeekWidget::setPosition(quint32 val)
{
    m_timeSlider->setValue(val);
    QTime time(0, 0);
    time = time.addMSecs(val);
    m_timeLabel->setText(time.toString(timeFormat));
}

void SeekWidget::reset()
{
    setPosition(0);
    setLength(0);
}

void SeekWidget::sliderPressed()
{
    emit beginSeek();
}

void SeekWidget::sliderReleased()
{
    emit endSeek(static_cast<quint32>(m_timeSlider->value()));
}

void SeekWidget::init()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(0,0,2,0);

    m_timeSlider = new QSlider(Qt::Horizontal,this);
    m_timeSlider->setRange(0,0);
    m_timeSlider->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_timeSlider->setValue(0);
    connect(m_timeSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(m_timeSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    layout->addWidget(m_timeSlider);

    m_timeLabel = new QLabel(QTime(0,0,0,0).toString(timeFormat));
    layout->addWidget(m_timeLabel);
}

class PlayButton: public HomeActionButton
{

    Q_OBJECT

public:
    PlayButton(QWidget* parent = 0);
    void reset();

signals:
    void play();
    void stop();

public slots:
    void setState(QtopiaMedia::State s);

private slots:
    void toggle();

private:
    QtopiaMedia::State m_state;
};

PlayButton::PlayButton(QWidget* parent)
    :
    HomeActionButton(),
    m_state(QtopiaMedia::Stopped)
{
    setParent(parent);
    connect(this,SIGNAL(clicked()),this,SLOT(toggle()));
    reset();
}

void PlayButton::reset()
{
    setState(QtopiaMedia::Stopped);
}

void PlayButton::setState(QtopiaMedia::State state)
{
    m_state = state;
    if(m_state == QtopiaMedia::Playing)
    {
        setIcon(stopIcon());
        setText("Stop");
    }
    else
    {
        setIcon(playIcon());
        setText("Play");
    }
}

void PlayButton::toggle()
{
    if(m_state == QtopiaMedia::Playing)
        emit stop();
    else emit play();
}

class PlayLabel : public QLabel
{
    Q_OBJECT

public:
    PlayLabel(QWidget* parent = 0);
    bool playEnabled() const;

public slots:
    void setPlayEnabled(bool val);

signals:
    void play();

protected:
    void mousePressEvent(QMouseEvent* e);

private:
    bool m_playEnabled;
};

PlayLabel::PlayLabel(QWidget* parent)
    :
    QLabel(parent),
    m_playEnabled(false)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Window,QColor(Qt::black));
    pal.setBrush(QPalette::Text,QColor(Qt::white));
    QLabel::setPalette(pal);
    QLabel::setAutoFillBackground(true);
    QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setPlayEnabled(m_playEnabled);
}

bool PlayLabel::playEnabled() const
{
    return m_playEnabled;
}

void PlayLabel::setPlayEnabled(bool val)
{
    m_playEnabled = val;

    QLabel::clear();

    if(m_playEnabled)
        QLabel::setPixmap(playIcon().pixmap(QSize(20,20)));
    else
        QLabel::setText(tr("No video"));
}

void PlayLabel::mousePressEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
    if(m_playEnabled)
        emit play();
    QLabel::mousePressEvent(e);
}

class VideoRecorderWidget : public QWidget
{
    Q_OBJECT

public:
    VideoRecorderWidget(QWidget* parent = 0);
    virtual ~VideoRecorderWidget();

    QContent mediaFile() const;
    QTime playTime() const {
        QTime length(0,0,0,0);
        if(m_mediaControl)
            length = length.addMSecs(m_mediaControl->length());
        return length;
    }

private:
    void init();

public slots:
    void stop();
    void play();
    void record();
    void setMediaFile(const QContent& file);

private slots:
    void createMediaControl();
    void createVideoWidget();
    void playerStateChanged(QtopiaMedia::State);
    void beginSeek();
    void endSeek(quint32);

private:
    enum State
    {
        InitState,
        CreatedControlState,
        CreatedVideoWidgetState
    };

    HomeActionButton* m_recordButton;
    PlayButton* m_playButton;
    PlayLabel* m_playLabel;
    QStackedWidget* m_widgetStack;
    QWidget* m_videoWidget;
    QMediaContentContext* m_mediaContext;
    QContent m_mediaFile;
    QMediaContent* m_mediaContent;
    QMediaControl* m_mediaControl;
    State m_state;
    QMediaVideoControl* m_videoControl;
    SeekWidget* m_seekWidget;
    QTemporaryFile* m_tempFile;
};

VideoRecorderWidget::VideoRecorderWidget(QWidget* parent)
    :
    QWidget(parent),
    m_recordButton(0),
    m_playButton(0),
    m_widgetStack(0),
    m_videoWidget(0),
    m_mediaContext(0),
    m_mediaContent(0),
    m_mediaControl(0),
    m_state(InitState),
    m_videoControl(0),
    m_seekWidget(0),
    m_tempFile(0)
{
    init();
}

VideoRecorderWidget::~VideoRecorderWidget()
{
    delete m_mediaControl;
    delete m_mediaContent;
    delete m_tempFile;
    m_mediaFile.removeFiles();
    m_mediaFile.commit();
}

QContent VideoRecorderWidget::mediaFile() const
{
    return m_mediaFile;
}

void VideoRecorderWidget::init()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    QWidget* buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
    layout->addWidget(buttonPanel);

    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonPanel);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0,0,0,0);
    buttonPanel->setLayout(buttonLayout);

    m_recordButton = new HomeActionButton(QtopiaHome::Red);
    m_recordButton->setText("Record");
    m_recordButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
    m_recordButton->setFocusPolicy(Qt::NoFocus);
    m_recordButton->setMaximumWidth(52);
    m_recordButton->setMinimumHeight(30);
    connect(m_recordButton,SIGNAL(clicked()),this,SLOT(record()));
    buttonLayout->addWidget(m_recordButton);

    m_playButton = new PlayButton();
    m_playButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
    m_playButton->setFocusPolicy(Qt::NoFocus);
    m_playButton->setMaximumWidth(52);
    m_playButton->setMinimumHeight(30);
    m_playButton->setVisible(false);
    connect(m_playButton,SIGNAL(play()),this,SLOT(play()));
    connect(m_playButton,SIGNAL(stop()),this,SLOT(stop()));
    buttonLayout->addWidget(m_playButton);

    buttonLayout->addStretch();

    QVBoxLayout* playerLayout = new QVBoxLayout();
    playerLayout->setSpacing(0);
    playerLayout->setContentsMargins(0,0,0,0);
    layout->addLayout(playerLayout);

    m_widgetStack = new QStackedWidget(this);
    m_widgetStack->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    playerLayout->addWidget(m_widgetStack);

    m_playLabel = new PlayLabel(this);
    connect(m_playLabel,SIGNAL(play()),this,SLOT(play()));
    m_widgetStack->addWidget(m_playLabel);

    m_seekWidget = new SeekWidget(this);
    connect(m_seekWidget, SIGNAL(beginSeek()), this, SLOT(beginSeek()));
    connect(m_seekWidget, SIGNAL(endSeek(quint32)), this, SLOT(endSeek(quint32)));
    playerLayout->addWidget(m_seekWidget);

    m_mediaContext = new QMediaContentContext(this);

    QMediaControlNotifier* controlNotifier = new QMediaControlNotifier(QMediaControl::name(), this);
    connect(controlNotifier, SIGNAL(valid()), this, SLOT(createMediaControl()));
    m_mediaContext->addObject(controlNotifier);

    QMediaControlNotifier* notifier = new QMediaControlNotifier(QMediaVideoControl::name(), this);
    connect(notifier, SIGNAL(valid()), this, SLOT(createVideoWidget()));
    m_mediaContext->addObject(notifier);

    m_tempFile = new QTemporaryFile(tempFileTemplate,this);
}

void VideoRecorderWidget::setMediaFile(const QContent& file)
{
    if(m_mediaControl)
        m_mediaControl->stop();
    m_mediaFile = file;
    m_state = InitState;
    m_playButton->setVisible(m_mediaFile.isValid());
    m_playLabel->setPlayEnabled(m_mediaFile.isValid());
}

void VideoRecorderWidget::record()
{
    m_tempFile->open();
    QByteArray requestData;
    QDataStream stream(&requestData, QIODevice::WriteOnly);
    stream << QString(m_tempFile->fileName()) << 256000 << 32000;
    QDSAction cameraAction("getVideo", "Camera");
    cameraAction.exec(QDSData(requestData,QMimeType("x-parameters/x-videoparameters")));
    setMediaFile(QContent(m_tempFile->fileName()));
}

void VideoRecorderWidget::play()
{
    if(m_state == InitState)
    {
        if (!m_mediaFile.isValid()) {
            if(!m_mediaFile.fileName().isEmpty())
                qWarning() << "Failed to load" << m_mediaFile.fileName();
        }
        else
        {
            m_mediaContent->deleteLater();
            m_mediaContent = new QMediaContent(m_mediaFile);
            m_mediaContext->setMediaContent(m_mediaContent);
        }
    }
}

void VideoRecorderWidget::stop()
{
    if(m_mediaControl)
        m_mediaControl->stop();
    m_state = InitState;
    m_seekWidget->reset();
}

void VideoRecorderWidget::createMediaControl()
{
    if(m_state == InitState)
    {
        m_mediaControl = new QMediaControl(m_mediaContent);

        connect(m_mediaControl,
                SIGNAL(lengthChanged(quint32)),
                m_seekWidget,
                SLOT(setLength(quint32)));
        connect(m_mediaControl,
                SIGNAL(positionChanged(quint32)),
                m_seekWidget,
                SLOT(setPosition(quint32)));
        connect(m_mediaControl,
                SIGNAL(playerStateChanged(QtopiaMedia::State)),
                m_playButton,
                SLOT(setState(QtopiaMedia::State)));
        connect(m_mediaControl,
                SIGNAL(playerStateChanged(QtopiaMedia::State)),
                this,
                SLOT(playerStateChanged(QtopiaMedia::State)));

        m_state = CreatedControlState;
        m_mediaControl->start();
    }
}

void VideoRecorderWidget::createVideoWidget()
{
    if(m_state == CreatedControlState)
    {
        delete m_videoWidget;

        // create video widget
        m_videoControl = new QMediaVideoControl(m_mediaContent);
        m_videoWidget = m_videoControl->createVideoWidget(this);
        if (!m_videoWidget) {
            qWarning("Failed to create video widget");
            return;
        }
        m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // display the video widget on the screen
        m_widgetStack->addWidget(m_videoWidget);
        m_widgetStack->setCurrentWidget(m_videoWidget);

        //evil-yes, but we need the widget stack to have flipped over and painted before
        //video playback commences
        qApp->processEvents();

        m_state = CreatedVideoWidgetState;
    }
}

void VideoRecorderWidget::playerStateChanged(QtopiaMedia::State s)
{
    if(s == QtopiaMedia::Stopped)
    {
        m_widgetStack->setCurrentWidget(m_playLabel);
        m_state = InitState;
    }
}

void VideoRecorderWidget::beginSeek()
{
    if(m_state == CreatedVideoWidgetState && m_mediaControl && m_mediaControl->playerState() == QtopiaMedia::Playing)
        m_mediaControl->pause();
}

void VideoRecorderWidget::endSeek(quint32 pos)
{
    if(m_state == CreatedVideoWidgetState && m_mediaControl && m_mediaControl->playerState() == QtopiaMedia::Paused)
    {
        m_mediaControl->seek(pos);
        m_mediaControl->start();
    }
}

VideomailComposerInterface::VideomailComposerInterface(QWidget* parent)
:
    QMailComposerInterface(parent),
    m_sizer(0),
    m_contactsButton(0),
    m_toEdit(0),
    m_videoRecorderWidget(0),
    m_title(QString()),
    m_fromAddress(QString())
{
    init();
}

VideomailComposerInterface::~VideomailComposerInterface()
{
    delete m_sizer;
}

bool VideomailComposerInterface::isEmpty() const
{
    return !m_videoRecorderWidget->mediaFile().isValid();
}

QMailMessage VideomailComposerInterface::message() const
{
    if( isEmpty() )
        return QMailMessage();

    //add addressing info

    QMailMessage mail;
    mail.setMultipartType(QMailMessage::MultipartMixed);
    mail.setTo(QMailAddress(to()));
    mail.setSubject("Videomail message");
    mail.setMessageType( QMailMessage::Email );
    mail.setContent(QMailMessage::VideoContent);

    QMailAccountKey key(QMailAccountKey::MessageType, QMailMessage::Email);
    foreach (const QMailAccountId& id, QMailStore::instance()->queryAccounts(key)) {
        AccountConfiguration config(id);

        QString emailAddress = config.emailAddress();
        if (!emailAddress.isEmpty()) {
            if (emailAddress == from() || !mail.parentAccountId().isValid()) {
                mail.setFrom(QMailAddress(emailAddress));
                mail.setParentAccountId(id);
                break;
            }
        }
    }

    //add text part

    QMailMessageContentType textType("text/plain; charset=UTF-8");
    QString messageText = QString("Videomail message\nFrom: %1\nLength: %2 seconds")
                          .arg(mail.from().toString())
                          .arg(m_videoRecorderWidget->playTime().toString(timeFormat));
    QMailMessagePart textPart;
    textPart.setBody(QMailMessageBody::fromData(messageText.toUtf8(), textType, QMailMessageBody::Base64));
    mail.appendPart(textPart);

    //add video part

    QContent video = m_videoRecorderWidget->mediaFile();
    QFile videoFile(video.fileName());
    videoFile.open(QIODevice::ReadOnly);
    QDataStream videoStream(&videoFile);
    QMailMessageContentType contentType(video.type().toLatin1());
    QMailMessagePart videoPart = QMailMessagePart::fromStream(videoStream,
                                                              QMailMessageContentDisposition::Attachment,
                                                              contentType,
                                                              QMailMessageBody::Base64,
                                                              QMailMessageBody::RequiresEncoding);
    mail.appendPart(videoPart);

    return mail;
}

void VideomailComposerInterface::setDefaultAccount(const QMailAccountId& id)
{
    Q_UNUSED(id);
}

void VideomailComposerInterface::setTo(const QString& toAddress)
{
    m_toEdit->setField(toAddress);
}

void VideomailComposerInterface::setFrom(const QString& fromAddress)
{
    m_fromAddress = fromAddress;
}

QString VideomailComposerInterface::from() const
{
    return m_fromAddress;
}

QString VideomailComposerInterface::to() const
{
    return m_toEdit->field();
}

bool VideomailComposerInterface::isReadyToSend() const
{
    bool result(!isEmpty() && !m_toEdit->field().isEmpty());
    return result;
}

bool VideomailComposerInterface::isDetailsOnlyMode() const
{
    return false;
}

void VideomailComposerInterface::setDetailsOnlyMode(bool val)
{
    Q_UNUSED(val);
}

QString VideomailComposerInterface::contextTitle() const
{
    return m_title;
}

QMailAccount VideomailComposerInterface::fromAccount() const
{
    return QMailAccount();
}

void VideomailComposerInterface::setMessage(const QMailMessage& mail)
{
    Q_UNUSED(mail);
}

void VideomailComposerInterface::clear()
{
    m_videoRecorderWidget->setMediaFile(QContent());
}

void VideomailComposerInterface::setBody(const QString &text, const QString &type)
{
    Q_UNUSED(text);
    Q_UNUSED(type);
    //no explicit body for video messages
}

void VideomailComposerInterface::attach(const QContent &lnk,
                                QMailMessage::AttachmentsAction action)
{
    Q_UNUSED(lnk);
    Q_UNUSED(action);
    //no attachments for video messages
}

void VideomailComposerInterface::setSignature( const QString &sig )
{
    Q_UNUSED(sig);
    //no signature for video messages
}

void VideomailComposerInterface::reply(const QMailMessage& source, int type)
{
    Q_UNUSED(source);
    Q_UNUSED(type);
}

void VideomailComposerInterface::setSubject(const QString& val)
{
    Q_UNUSED(val);
}

void VideomailComposerInterface::selectRecipients()
{
    static const QString addressSeparator(", ");

    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle(tr("Select Contacts"));

    QVBoxLayout *vbl = new QVBoxLayout(&selectionDialog);
    selectionDialog.setLayout(vbl);

    AddressSelectorWidget* addressSelector= new AddressSelectorWidget(AddressSelectorWidget::EmailSelection,
                                                                      &selectionDialog);
    vbl->addWidget(addressSelector);
    addressSelector->setSelectedAddresses(to().split(addressSeparator,QString::SkipEmptyParts));

    if(QtopiaApplication::execDialog(&selectionDialog) == QDialog::Accepted)
    {
        QStringList selectedAddresses = addressSelector->selectedAddresses();
        m_toEdit->setField(selectedAddresses.join(addressSeparator));
    }
}

void VideomailComposerInterface::toClicked()
{
    bool ok = false;
    QString ret = QtopiaInputDialog::getText(this,
                                             tr("To"),
                                             tr("To"),QLineEdit::Normal,
                                             QtopiaApplication::Words,
                                             QString(), m_toEdit->field(), &ok);
    if(ok)
        m_toEdit->setField(ret);
}

void VideomailComposerInterface::init()
{
    m_sizer = new ColumnSizer();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    QWidget::setLayout(layout);

    QLayout* toLayout = new QHBoxLayout();
    layout->addLayout(toLayout);

    m_toEdit = new HomeFieldButton("To:","",*m_sizer,true);
    connect(m_toEdit,SIGNAL(clicked()),this,SLOT(toClicked()));
    toLayout->addWidget(m_toEdit);

    m_contactsButton= new HomeActionButton("Contact",QtopiaHome::Green);
    m_contactsButton->setMaximumWidth(45);
    m_contactsButton->setMinimumWidth(45);
    connect(m_contactsButton,SIGNAL(clicked()),this,SLOT(selectRecipients()));
    toLayout->addWidget(m_contactsButton);

    m_videoRecorderWidget = new VideoRecorderWidget(this);
    m_videoRecorderWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    layout->addWidget(m_videoRecorderWidget);

    setContext("Create " + displayName(QMailMessage::Email));
}

void VideomailComposerInterface::setContext(const QString& context)
{
    m_title = context;
    emit contextChanged();
}

QTOPIA_EXPORT_PLUGIN( VideomailComposerPlugin )

VideomailComposerPlugin::VideomailComposerPlugin()
    : QMailComposerPlugin()
{
}

QMailComposerInterface* VideomailComposerPlugin::create( QWidget *parent )
{
    return new VideomailComposerInterface(parent);
}

#include <videomailcomposer.moc>

