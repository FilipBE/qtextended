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

#include "qmediawidgets.h"
#include <QtopiaApplication>

#include "mediastyle_p.h"
#include "activitymonitor_p.h"

static QImage load_scaled_image( const QString& filename, const QSize& size, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio )
{
    QImageReader reader( filename );

    QSize scaled;
    if( mode == Qt::IgnoreAspectRatio ) {
        scaled = size;
    } else {
        scaled = reader.size();
        scaled.scale( size, mode );
    }

    reader.setQuality( 49 ); // Otherwise Qt smooth scales
    reader.setScaledSize( scaled );
    return reader.read();
}

static QImage invert( const QImage& image )
{
    QImage ret;

    switch( image.format() )
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        ret = image;
        break;
    case QImage::Format_ARGB32_Premultiplied:
        ret = image.convertToFormat( QImage::Format_ARGB32 );
        break;
    default:
        // Unsupported
        return image;
    }

    quint32 *p = (quint32*)ret.bits();
    quint32 *end = p + ret.numBytes()/4;
    while( p != end ) {
        *p++ ^= 0x00ffffff;
    }

    return ret;
}

static QImage contrast( const QColor& color, const QImage& image )
{
    static const int INVERT_THRESHOLD = 128;

    int x, v;
    color.getHsv( &x, &x, &v );
    if( v > INVERT_THRESHOLD ) {
        return invert( image );
    }

    return image;
}

/*!
    \class QMediaStateLabel
    \inpublicgroup QtMediaModule


    \brief The QMediaStateLabel class displays icons for the various states of
    media playback.

    \ingroup multimedia

    The QMediaStateLabel class is useful for displaying standard icons
    for conveying state information about media playback.

    The display type can be set either by the constructor or by the
    setState() slot.

    \code
        QMediaStateLabel *label = new QMediaStateLabel;
        label->setState( QtopiaMedia::Playing );
    \endcode

    \image qmediastatelabel.jpg "QMediaStateLabel"
*/

class QMediaStateLabelPrivate
{
public:
    QImage buffer;
};

/*!
    \fn QMediaStateLabel::QMediaStateLabel( QtopiaMedia::State state, QWidget* parent )

    Constructs a state label with a default display type of \a state.

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaStateLabel::QMediaStateLabel( QtopiaMedia::State state, QWidget* parent )
    : QWidget( parent ), m_state( state )
{
    m_d = new QMediaStateLabelPrivate;
}

/*!
    \fn QMediaStateLabel::~QMediaStateLabel()

    Destroys the state label.
*/

QMediaStateLabel::~QMediaStateLabel()
{
    delete m_d;
}

/*!
    \fn QtopiaMedia::State QMediaStateLabel::state() const

    Returns the current state display.
*/
QtopiaMedia::State QMediaStateLabel::state() const
{
    return m_state;
}

/*!
    \reimp
*/
QSize QMediaStateLabel::sizeHint() const { return QSize( QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize), QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) ); }

/*!
    \fn void QMediaStateLabel::setState( QtopiaMedia::State state )

    Displays the icon for \a state.
*/
void QMediaStateLabel::setState( QtopiaMedia::State state )
{
    m_state = state;

    m_d->buffer = QImage();
    update();
}

/*!
    \reimp
*/
void QMediaStateLabel::paintEvent( QPaintEvent* )
{
    QColor textcolor = palette().windowText().color();

    if( m_d->buffer.isNull() ) {
        switch( m_state )
        {
        case QtopiaMedia::Playing:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":image/mediaplayer/black/play", size() ) );
            break;
        case QtopiaMedia::Paused:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":image/mediaplayer/black/pause", size() ) );
            break;
        case QtopiaMedia::Stopped:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":icon/mediaplayer/black/stop", size() ) );
            break;
        default:
            // Ignore
            break;
        }
    }

    QPainter painter( this );
    painter.drawImage( QPoint( 0, 0 ), m_d->buffer );
}

/*!
    \reimp
*/
void QMediaStateLabel::resizeEvent( QResizeEvent* )
{
    m_d->buffer = QImage();
    update();
}

static const int MS_PER_S = 1000; // 1000 milliseconds per second
static const int S_PER_MIN = 60; // 60 seconds per minute

class Millisecond
{
public:
    Millisecond( quint32 ms )
        : m_ms( ms )
    {
        m_s = m_ms / MS_PER_S;
        m_min = m_s / S_PER_MIN;
        m_minr = m_s % S_PER_MIN;
    }

    quint32 seconds() const { return m_s; }

    // Return time in the following format 0:00
    QString toString() const;

private:
    quint32 m_ms, m_s, m_min, m_minr;
};

QString Millisecond::toString() const
{
    static const QString TIME_TEMPLATE = QString( "%1:%2%3" );

    return TIME_TEMPLATE.arg( m_min ).arg( m_minr / 10 ).arg( m_minr % 10 );
}

class SimpleLabel : public QWidget
{
public:
    SimpleLabel( QWidget* parent = 0, Qt::WindowFlags f = 0 )
        : QWidget( parent, f ), m_alignment( Qt::AlignLeft )
    { }

    void setText( const QString& text );

    void setAlignment( Qt::Alignment alignment ) { m_alignment = alignment; }
    Qt::Alignment alignment() const { return m_alignment; }

    void setSize( const QSize& size ) { m_size = size; }

    // QWidget
    QSize sizeHint() const;

protected:
    // QWidget
    void paintEvent( QPaintEvent* e );

private:
    QString m_text;
    Qt::Alignment m_alignment;
    QSize m_size;
};

void SimpleLabel::setText( const QString& text )
{
    m_text = text;

    update();
}

QSize SimpleLabel::sizeHint() const
{
    return m_size;
}

void SimpleLabel::paintEvent( QPaintEvent* )
{
    QPainter painter( this );
    painter.drawText( rect(), m_alignment, m_text );
}

/*!
    \class QMediaProgressLabel
    \inpublicgroup QtMediaModule


    \brief The QMediaProgressLabel class displays the current progress of a
    playing media content as a text label.

    \ingroup multimedia

    The QMediaProgressLabel class can be used in conjunction with the
    QMediaContent class to display the playback progress of a media
    content object.

    \code
        QMediaProgressLabel *label = new QMediaProgressLabel( this );
        label->setProgressType( QMediaProgressLabel::ElapsedTotalTime );

        QMediaContent *content = new QMediaContent( url, this );
        label->setMediaContent( content );
    \endcode

    Alternatively the progress can be set directly using the setElapsed() and
    setTotal() slots. For example, the QMediaSeekWidget class emits
    positionChanged() and lengthChanged() signals during a seek. Connect the
    QMediaSeekWidget signals to the QMediaProgressLabel slots to display the
    current seek position as the user performs a seek.

    \code
        QMediaProgressLabel *label = new QMediaProgressLabel( this );

        QMediaSeekWidget *seek = new QMediaSeekWidget( this );
        connect( seek, SIGNAL(positionChanged(quint32)),
            label, SLOT(setElapsed(quint32)) );
        connect( seek, SIGNAL(lengthChanged(quint32)),
            label, SLOT(setTotal(quint32)) );

        QMediaContent *content = new QMediaContent( url, this );
        seek->setMediaContent( content );
    \endcode

    The progress display type can be set either by the constructor or by the
    setProgressType() method.

    \image qmediaprogresslabel.jpg "QMediaProgressLabel"

    QMediaProgressLabel is a \l {player object}.
*/

/*!
    \enum QMediaProgressLabel::Type

    This enum type specifies progress display formats.

    \value ElapsedTime
        Displays the elapsed time with the format \tt {0:00}.
    \value ElapsedTotalTime
        Displays the elapsed and total time with the format \tt {0:00 / 0:00}.
    \value RemainingTime
        Displays the remaining time with the format \tt {-0:00}.
*/

class QMediaProgressLabelPrivate
{
public:
    SimpleLabel *time;
};

/*!
    \fn QMediaProgressLabel::QMediaProgressLabel( Type type, QWidget* parent )

    Constructs a progress label with the format \a type. Elapsed and total time are initialized to \tt {0:00}.

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaProgressLabel::QMediaProgressLabel( Type type, QWidget* parent )
    : QWidget( parent ), m_type( type ), m_content( 0 ), m_control( 0 ), m_elapsed( 0 ), m_total( 0 )
{
    m_d = new QMediaProgressLabelPrivate;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_d->time = new SimpleLabel;
    layout->addWidget( m_d->time );

    switch( m_type )
    {
    case ElapsedTime:
        m_d->time->setSize( fontMetrics().boundingRect( "00:00" ).size() );
        m_d->time->setAlignment( Qt::AlignLeft );
        break;
    case ElapsedTotalTime:
        m_d->time->setSize( fontMetrics().boundingRect( "00:00 / 00:00" ).size() );
        m_d->time->setAlignment( Qt::AlignRight );
        break;
    case RemainingTime:
        m_d->time->setSize( fontMetrics().boundingRect( "-00:00" ).size() );
        m_d->time->setAlignment( Qt::AlignRight );
        break;
    }

    setLayout( layout );

    // Construct interface notifier
    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );

    deactivate();
}

/*!
    \fn QMediaProgressLabel::~QMediaProgressLabel()

    Destroys the label.
*/
QMediaProgressLabel::~QMediaProgressLabel()
{
    delete m_d;
}

/*!
    \fn Type QMediaProgressLabel::progressType() const

    Returns the current progress type.
*/
QMediaProgressLabel::Type QMediaProgressLabel::progressType() const
{
    return m_type;
}

/*!
    \fn void QMediaProgressLabel::setProgressType( Type type )

    The \a type sets kind of progress to display.
*/
void QMediaProgressLabel::setProgressType( Type type )
{
    m_type = type;

    updateProgress();
}

/*!
    \fn QMediaContent* QMediaProgressLabel::content() const

    Returns the media content object currently being watched.
*/
QMediaContent* QMediaProgressLabel::content() const
{
    return m_content;
}

/*!
    \fn void QMediaProgressLabel::setMediaContent( QMediaContent* content )

    Sets \a content as the media content object to watch.
*/
void QMediaProgressLabel::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( content );
}

/*!
    \fn void QMediaProgressLabel::setElapsed( quint32 ms )

    Sets the elapsed time to \a ms in milliseconds.

    Calling this method should not be necessary if a media content is set.
*/
void QMediaProgressLabel::setElapsed( quint32 ms )
{
    m_elapsed = ms;

    updateProgress();
}

/*!
    \fn void QMediaProgressLabel::setTotal( quint32 ms )

    Sets the total time to \a ms in milliseconds.

    Calling this method should not be necessary if a media content is set.
*/
void QMediaProgressLabel::setTotal( quint32 ms )
{
    m_total = ms;

    updateProgress();
}

void QMediaProgressLabel::activate()
{
    // Connect to media control
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(lengthChanged(quint32)),
        this, SLOT(setTotal(quint32)) );
    connect( m_control, SIGNAL(positionChanged(quint32)),
        this, SLOT(setElapsed(quint32)) );
}

void QMediaProgressLabel::deactivate()
{
    delete m_control;
    m_control = 0;

    setElapsed( 0 );
    setTotal( 0 );
}

void QMediaProgressLabel::updateProgress()
{
    static const QString ELAPSED_TOTAL_TEMPLATE = QString( "%1 / %2" );
    static const QString REMAINING_TEMPLATE = QString( "-%1" );

    switch( m_type )
    {
    case ElapsedTime:
        m_d->time->setText( Millisecond( m_elapsed ).toString() );
        break;
    case ElapsedTotalTime:
        m_d->time->setText( ELAPSED_TOTAL_TEMPLATE
            .arg( Millisecond( m_elapsed ).toString() )
            .arg( Millisecond( m_total ).toString() ) );
        break;
    case RemainingTime:
        m_d->time->setText( REMAINING_TEMPLATE
            .arg( Millisecond( m_total - m_elapsed ).toString() ) );
        break;
    }
}

class Task
{
public:
    virtual ~Task() { }

    virtual void start() = 0;
    virtual void stop() = 0;
};

class IterativeTask : public QObject,
    public Task
{
public:
    IterativeTask( int interval, QObject* parent = 0 )
        : QObject( parent ), m_interval( interval ), m_timerid(0)
    { }

    // Task
    void start();
    void stop();

    virtual void execute() = 0;

protected:
    void timerEvent( QTimerEvent* e );

private:
    int m_interval;
    int m_timerid;
};

void IterativeTask::start()
{
    m_timerid = startTimer( m_interval );
}

void IterativeTask::stop()
{
    if ( m_timerid ) {
        killTimer( m_timerid );
        m_timerid = 0;
    }
}

void IterativeTask::timerEvent( QTimerEvent* e )
{
    if( e->timerId() == m_timerid ) {
        execute();
    }
}

static const int ITERATIVE_TASK_INTERVAL = 60;

class SlimlineProgress : public QProgressBar
{
public:
    SlimlineProgress( QWidget* parent = 0 );

    ~SlimlineProgress()
    {
        delete m_style;
    }

    QSize sizeHint() const { return QSize( fontMetrics().height(), fontMetrics().height() ); }

private:
    MediaStyle *m_style;
};

SlimlineProgress::SlimlineProgress( QWidget* parent )
    : QProgressBar( parent )
{
    static const QColor GREEN = QColor( 166, 206, 57 );
    static const QColor BLACK = QColor::fromCmykF( 0, 0, 0, 0.9 );

    m_style = new MediaStyle;
    setStyle( m_style );

    QPalette custom = palette();
    custom.setColor( QPalette::Highlight, GREEN );
    custom.setColor( QPalette::Shadow, BLACK );
    setPalette( custom );
}

/*!
    \class QMediaProgressWidget
    \inpublicgroup QtMediaModule


    \brief The QMediaProgressWidget class displays the current progress of a
    playing media content as a progress bar.

    \ingroup multimedia

    The QMediaProgressWidget class can be used in conjunction with the
    QMediaContent class to display the playback progress of a media
    content object.

    \code
        QMediaProgressWidget *progress = new QMediaProgressWidget( this );

        QMediaContent *content = new QMediaContent( url, this );
        progress->setMediaContent( content );
    \endcode

    \image qmediaprogresswidget.jpg "QMediaProgressWidget"

    QMediaProgressWidget is a \l {player object}.
*/

class QMediaProgressWidgetPrivate
{
public:
    SlimlineProgress *progress;
};

/*!
    \fn QMediaProgressWidget::QMediaProgressWidget( QWidget* parent )

    Constructs an progress widget. The progress is initialized to nill.

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaProgressWidget::QMediaProgressWidget( QWidget* parent )
    : QWidget( parent ), m_control( 0 )
{
    m_d = new QMediaProgressWidgetPrivate;

    // Construct progress bar
    m_d->progress = new SlimlineProgress;
    m_d->progress->setTextVisible( false );
    m_d->progress->setMaximum( 0 );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    layout->addWidget( m_d->progress );
    setLayout( layout );

    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
}

/*!
    \fn QMediaProgressWidget::~QMediaProgressWidget()

    Destroys the widget.
*/
QMediaProgressWidget::~QMediaProgressWidget()
{
    delete m_d;
}

/*!
    \fn QMediaContent* QMediaProgressWidget::content() const

    Returns the media content object currently being watched.
*/
QMediaContent* QMediaProgressWidget::content() const
{
    return m_content;
}

/*!
    \fn void QMediaProgressWidget::setMediaContent( QMediaContent* content )

    Sets \a content as the media content object to watch.
*/
void QMediaProgressWidget::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( m_content );
}

void QMediaProgressWidget::setPosition( quint32 ms )
{
    m_d->progress->setValue( Millisecond( ms ).seconds() );
}

void QMediaProgressWidget::setLength( quint32 ms )
{
    m_d->progress->setMaximum( Millisecond( ms ).seconds() );
}

void QMediaProgressWidget::activate()
{
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(positionChanged(quint32)),
        this, SLOT(setPosition(quint32)) );
    connect( m_control, SIGNAL(lengthChanged(quint32)),
        this, SLOT(setLength(quint32)) );
}

void QMediaProgressWidget::deactivate()
{
    delete m_control;
    m_control = 0;

    m_d->progress->reset();
}

/*!
    \class QMediaVolumeLabel
    \inpublicgroup QtMediaModule


    \brief The QMediaVolumeLabel class displays icons for the various levels of
    volume.

    \ingroup multimedia

    The QMediaVolumeLabel class is useful for displaying standard icons
    for conveying volume information.

    The display type can be set either by the constructor or by the
    setVolumeType() method.

    \code
        QMediaVolumeLabel *label = new QMediaVolumeLabel;
        label->setVolumeType( QMediaVolumeLabel::MuteVolume );
    \endcode

    \image qmediavolumelabel.jpg "QMediaVolumeLabel"
*/

/*!
    \enum QMediaVolumeLabel::Type

    This enum type specifies the volume level icon.

    \value MinimumVolume
        Displays the icon representing minimum volume.
    \value MaximumVolume
        Displays the icon representing maximum volume.
    \value MuteVolume
        Displays the icon representing mute volume.
*/

class QMediaVolumeLabelPrivate
{
public:
    QImage buffer;
};

/*!
    \fn QMediaVolumeLabel::QMediaVolumeLabel( Type type, QWidget* parent )

    Constructs a volume label with a default icon of \a type.

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaVolumeLabel::QMediaVolumeLabel( Type type, QWidget* parent )
    : QWidget( parent ), m_type( type )
{
    m_d = new QMediaVolumeLabelPrivate;
}

/*!
    \fn QMediaVolumeLabel::~QMediaVolumeLabel()

    Destroys the label.
*/
QMediaVolumeLabel::~QMediaVolumeLabel()
{
    delete m_d;
}

/*!
    \fn Type QMediaVolumeLabel::volumeType() const

    Returns the current display type.
*/
QMediaVolumeLabel::Type QMediaVolumeLabel::volumeType() const
{
    return m_type;
}

/*!
    \fn void QMediaVolumeLabel::setVolumeType( Type type )

    Displays the icon for \a type.
*/
void QMediaVolumeLabel::setVolumeType( Type type )
{
    m_type = type;

    m_d->buffer = QImage();
    update();
}

/*!
    \reimp
*/
QSize QMediaVolumeLabel::sizeHint() const
{
    return QSize( QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize), QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) );
}

/*!
    \reimp
*/
void QMediaVolumeLabel::paintEvent( QPaintEvent* )
{
    QColor textcolor = palette().windowText().color();

    if( m_d->buffer.isNull() ) {
        switch( m_type )
        {
        case MinimumVolume:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":icon/mediaplayer/black/volume-min", size() ) );
            break;
        case MaximumVolume:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":icon/mediaplayer/black/volume-max", size() ) );
            break;
        case MuteVolume:
            m_d->buffer = contrast( textcolor, load_scaled_image( ":icon/mediaplayer/black/volume-mute", size() ) );
            break;
        }
    }

    QPainter painter( this );
    painter.drawImage( QPoint( 0, 0 ), m_d->buffer );
}

/*!
    \reimp
*/
void QMediaVolumeLabel::resizeEvent( QResizeEvent* )
{
    m_d->buffer = QImage();
    update();
}

static const int VOLUME_MAX = 100;
static const int VOLUME_MIN = 2;

class VolumeTask : public IterativeTask
{
    Q_OBJECT
public:
    enum Type { IncreaseVolume, DecreaseVolume };

    VolumeTask( Type type, QMediaControl* control, QProgressBar* progress );

    ~VolumeTask();

    // IterativeTask
    void execute();

signals:
    void iterationComplete();

private:
    QMediaControl *m_control;
    QProgressBar *m_progress;
    int m_volume, m_delta;
};

VolumeTask::VolumeTask( Type type, QMediaControl* control, QProgressBar* progress )
    : IterativeTask( ITERATIVE_TASK_INTERVAL ), m_control( control ), m_progress( progress )
{
    static const int STEP = 2;

    switch( type )
    {
    case IncreaseVolume:
        m_delta = STEP;
        break;
    case DecreaseVolume:
        m_delta = -STEP;
        break;
    }

    m_volume = m_control->volume();
}

VolumeTask::~VolumeTask()
{
    m_control->setVolume( m_volume );
}

void VolumeTask::execute()
{
    if( m_delta ) {
        // Increase/decrease volume
        m_volume += m_delta;

        if( m_volume < VOLUME_MIN || m_volume > VOLUME_MAX ) {
            if( m_delta < 0 ) {
                m_volume = VOLUME_MIN;
            } else {
                m_volume = VOLUME_MAX;
            }

            m_delta = 0;
        }

        if( !(m_volume % 4) ) m_control->setVolume( m_volume );
        m_progress->setValue( m_volume );
    }

    emit iterationComplete();
}

/*!
    \class QMediaVolumeWidget
    \inpublicgroup QtMediaModule


    \brief The QMediaVolumeWidget class displays the current volume level of a
    media content object as a progress bar.

    \ingroup multimedia

    The QMediaVolumeWidget class can be used in conjunction with the
    QMediaContent class to display the current volume of a media content
    object.

    \code
        QMediaVolumeWidget *volume = new QMediaVolumeWidget( this );

        QMediaContent *content = new QMediaContent( url, this );
        progress->setMediaContent( content );
    \endcode

    In addition the user is able to control the volume by holding down the left
    and right arrow keys. A hold of Qt::Key_Left decreases the volume while a
    hold of Qt::Key_Right increases the volume.

    \image qmediavolumewidget.jpg "QMediaVolumeWidget"

    QMediaVolumeWidget is a \l {player object}.
*/

class QMediaVolumeWidgetPrivate
{
public:
    Task *task;
    SlimlineProgress *progress;
    ActivityMonitor *monitor;
};

/*!
    \fn QMediaVolumeWidget::QMediaVolumeWidget( QWidget* parent )

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaVolumeWidget::QMediaVolumeWidget( QWidget* parent )
    : QWidget( parent ), m_control( 0 )
{
    static const int VOLUME_SUSPEND = 2000;

    m_d = new QMediaVolumeWidgetPrivate;
    m_d->task = 0;

    // Construct progress bar
    m_d->progress = new SlimlineProgress;
    m_d->progress->setTextVisible( false );
    m_d->progress->setMaximum( VOLUME_MAX );
    m_d->progress->setValue( VOLUME_MAX );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    layout->addWidget( m_d->progress );
    setLayout( layout );

    m_d->monitor = new ActivityMonitor( VOLUME_SUSPEND, this );
    connect( m_d->monitor, SIGNAL(active()), this, SLOT(suspend()) );
    connect( m_d->monitor, SIGNAL(inactive()), this, SLOT(resume()) );

    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
}

/*!
    \fn QMediaVolumeWidget::~QMediaVolumeWidget()

    Destroys the widget.
*/
QMediaVolumeWidget::~QMediaVolumeWidget()
{
    delete m_d;
}

/*!
    \fn QMediaContent* QMediaVolumeWidget::content() const

    Returns the media content object currently being used.
*/
QMediaContent* QMediaVolumeWidget::content() const
{
    return m_content;
}

/*!
    \fn void QMediaVolumeWidget::setMediaContent( QMediaContent* content )

    Sets \a content as the media content object to use.
*/
void QMediaVolumeWidget::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( content );
}

void QMediaVolumeWidget::setVolume( int volume )
{
    // Update current volume
    m_d->progress->setValue( volume );
}

void QMediaVolumeWidget::activate()
{
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(volumeChanged(int)),
        this, SLOT(setVolume(int)) );

    setVolume( m_control->volume() );
}

void QMediaVolumeWidget::deactivate()
{
    delete m_control;
    m_control = 0;

    delete m_d->task;
    m_d->task = 0;

    m_d->progress->reset();
}

void QMediaVolumeWidget::suspend()
{
    disconnect( m_control, SIGNAL(volumeChanged(int)),
        this, SLOT(setVolume(int)) );
}

void QMediaVolumeWidget::resume()
{
    if(!m_control) return;
    connect( m_control, SIGNAL(volumeChanged(int)),
        this, SLOT(setVolume(int)) );

    setVolume( m_control->volume() );
}

/*!
    \reimp
*/
void QMediaVolumeWidget::keyPressEvent( QKeyEvent* e )
{
    if( !m_control ) return;

    switch( e->key() )
    {
    case Qt::Key_Left:
        {
        VolumeTask *increasevolume = new VolumeTask( VolumeTask::IncreaseVolume, m_control, m_d->progress );
        connect( increasevolume, SIGNAL(iterationComplete()), m_d->monitor, SLOT(update()) );
        m_d->task = increasevolume;
        m_d->task->start();
        }
        break;
    case Qt::Key_Right:
        {
        VolumeTask *decreasevolume = new VolumeTask( VolumeTask::DecreaseVolume, m_control, m_d->progress );
        connect( decreasevolume, SIGNAL(iterationComplete()), m_d->monitor, SLOT(update()) );
        m_d->task = decreasevolume;
        m_d->task->start();
        }
        break;
    default:
        // Ignore
        break;
    }
}

/*!
    \reimp
*/
void QMediaVolumeWidget::keyReleaseEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Left:
    case Qt::Key_Right:
        delete m_d->task;
        m_d->task = 0;
        break;
    default:
        // Ignore
        break;
    }
}

class SeekTask : public IterativeTask
{
    Q_OBJECT
public:
    enum Type { SeekForward, SeekBackward };

    SeekTask( Type type, QProgressBar* progress )
        : IterativeTask( ITERATIVE_TASK_INTERVAL ),
        m_type( type ), m_progress( progress )
    { }

    // IterativeTask
    void execute();

signals:
    void iterationComplete();

private:
    Type m_type;
    QProgressBar *m_progress;
};

void SeekTask::execute()
{
    static const int STEP = 4;

    // Seek forward/backward
    int position = m_progress->value();
    int length = m_progress->maximum();
    switch( m_type )
    {
    case SeekForward:
        position += STEP;
        if( position < length ) {
            m_progress->setValue( position );
        } else {
            m_progress->setValue( length );
        }
        break;
    case SeekBackward:
        position -= STEP;
        if( position > 0 ) {
            m_progress->setValue( position );
        } else {
            m_progress->setValue( 0 );
        }
        break;
    }

    // Inform observers
    emit iterationComplete();
}

class SeekProgress : public SlimlineProgress
{
    Q_OBJECT
public:
    SeekProgress( QWidget* parent = 0 );

    void setTrackingEnabled( bool enabled );
    void setSilhouetteValue( int value );

signals:
    void lengthChanged( quint32 );
    void positionChanged( quint32 );

public slots:
    void setMediaContent( QMediaContent* content );

    void setLength( quint32 ms );
    void setPosition( quint32 ms );
    void emitPositionChanged();

private slots:
    void activate();
    void deactivate();

protected:
    // QWidget
    void paintEvent( QPaintEvent* e );

private:
    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    bool m_istracking;
    int m_silhouette;
};

SeekProgress::SeekProgress( QWidget* parent )
    : SlimlineProgress( parent ), m_control( 0 ), m_istracking( true ), m_silhouette( 0 )
{
    connect( this, SIGNAL(valueChanged(int)), this, SLOT(emitPositionChanged()) );

    m_notifier = new QMediaControlNotifier( QMediaControl::name(), parent );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
}

void SeekProgress::activate()
{
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(lengthChanged(quint32)),
        this, SLOT(setLength(quint32)) );
    if( m_istracking ) {
        connect( m_control, SIGNAL(positionChanged(quint32)),
            this, SLOT(setPosition(quint32)) );
    }

    setLength( m_control->length() );
}

void SeekProgress::deactivate()
{
    delete m_control;
    m_control = 0;

    reset();
}

void SeekProgress::setTrackingEnabled( bool enabled )
{
    if( m_istracking != enabled ) {
        m_istracking = enabled;

        if( m_istracking ) {
            connect( m_control, SIGNAL(positionChanged(quint32)),
                this, SLOT(setPosition(quint32)) );
        } else {
            disconnect( m_control, SIGNAL(positionChanged(quint32)),
                this, SLOT(setPosition(quint32)) );
        }

        update();
    }
}

void SeekProgress::setSilhouetteValue( int value )
{
    m_silhouette = value;

    update();
}

void SeekProgress::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( content );
}

void SeekProgress::setLength( quint32 ms )
{
    setMaximum( Millisecond( ms ).seconds() );
    emit lengthChanged( ms );
}

void SeekProgress::setPosition( quint32 ms )
{
    setValue( Millisecond( ms ).seconds() );
}

void SeekProgress::emitPositionChanged()
{
    emit positionChanged( value() * MS_PER_S );
}

void SeekProgress::paintEvent( QPaintEvent* e )
{
    static const int SILHOUETTE_ALPHA = 200;

    QProgressBar::paintEvent( e );

    if( !m_istracking ) {
        // Paint silhouette
        QStylePainter painter( this );
        QStyleOptionProgressBar opt;
        opt.init( this );

        opt.minimum = minimum();
        opt.maximum = maximum();
        opt.textVisible = false;

        opt.progress = m_silhouette;
        opt.state = QStyle::State_Sunken;

        QColor color = opt.palette.color( QPalette::Highlight ).dark( 125 );
        color.setAlpha( SILHOUETTE_ALPHA );
        opt.palette.setColor( QPalette::Highlight, color );

        painter.drawControl( QStyle::CE_ProgressBarContents, opt );
    }
}

class SeekMonitor : public ActivityMonitor
{
    Q_OBJECT
public:
    SeekMonitor( SeekProgress* progress, int interval, QObject* parent = 0 );

public slots:
    void setMediaContent( QMediaContent* content );

    void initSeek();
    void performSeek();

private slots:
    void activate();
    void deactivate();

private:
    QMediaContent *m_content;
    QMediaControlNotifier *m_notifier;
    QMediaControl *m_control;
    SeekProgress *m_progress;
};

SeekMonitor::SeekMonitor( SeekProgress* progress, int interval, QObject* parent )
    : ActivityMonitor( interval, parent ), m_control( 0 ), m_progress( progress )
{
    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
    connect( m_notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );

    connect( this, SIGNAL(active()), this, SLOT(initSeek()) );
    connect( this, SIGNAL(inactive()), this, SLOT(performSeek()) );
}

void SeekMonitor::initSeek()
{
    // Disable seek tracking in view
    m_progress->setTrackingEnabled( false );

    // Set silhouette to current player position
    m_progress->setSilhouetteValue( m_progress->value() );
}

void SeekMonitor::performSeek()
{
    // Retrive seek value from view and perfom actual seek
    if( m_control ) {
        m_control->seek( m_progress->value() * MS_PER_S );
    }

    // Enable seek tracking in view
    m_progress->setTrackingEnabled( true );
}

void SeekMonitor::setMediaContent( QMediaContent* content )
{
    m_content = content;
    m_notifier->setMediaContent( content );
}

void SeekMonitor::activate()
{
    m_control = new QMediaControl( m_content );
}

void SeekMonitor::deactivate()
{
    delete m_control;
    m_control = 0;
}

/*!
    \class QMediaSeekWidget
    \inpublicgroup QtMediaModule


    \brief The QMediaSeekWidget class allows the user to seek within a media content
    object.

    \ingroup multimedia

    The QMediaSeekWidget class can be used in conjunction with the
    QMediaContent class to enable the user to seek within a media content
    object.

    \code
        QMediaSeekWidget *seek = new QMediaSeekWidget( this );

        QMediaContent *content = new QMediaContent( url, this );
        seek->setMediaContent( content );
    \endcode

    The user can seek within a media content object by holding down the left
    and right arrow keys. A hold of Qt::Key_Left seeks backward while a hold of
    Qt::Key_Right seeks forward. The actual seek within the media content
    object is deferred until the user releases the arrow keys.

    During the seek the positionChanged() signal will be emitted continually
    with the current seek position. While the user is not seeking the
    positionChanged() and lengthChanged() signals will be emitted when position
    and length have changed in the media content object.

    \image qmediaseekwidget.jpg "QMediaSeekWidget"

    QMediaSeekWidget is a \l {player object}.
*/

class QMediaSeekWidgetPrivate
{
public:
    Task *task;
    ActivityMonitor *monitor;
    SeekProgress *progress;
};

/*!
    \fn QMediaSeekWidget::QMediaSeekWidget( QWidget* parent )

    Constructs a seek widget.

    The \a parent argument is passed to the QWidget constructor.
*/
QMediaSeekWidget::QMediaSeekWidget( QWidget* parent )
    : QWidget( parent )
{
    static const int SEEK_THRESHOLD = 1000;

    m_d = new QMediaSeekWidgetPrivate;

    // Construct progress bar
    m_d->progress = new SeekProgress;
    connect( m_d->progress, SIGNAL(lengthChanged(quint32)),
        this, SIGNAL(lengthChanged(quint32)) );
    connect( m_d->progress, SIGNAL(positionChanged(quint32)),
        this, SIGNAL(positionChanged(quint32)) );
    // Construct seek monitor to perform actual seek
    m_d->monitor = new SeekMonitor( m_d->progress, SEEK_THRESHOLD, this );

    m_context = new QMediaContentContext( this );
    m_context->addObject( m_d->progress );
    m_context->addObject( m_d->monitor );

    m_d->progress->setTextVisible( false );
    m_d->progress->setMaximum( 0 );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    layout->addWidget( m_d->progress );
    setLayout( layout );
}

/*!
    \fn QMediaSeekWidget::~QMediaSeekWidget()

    Destroys the widget.
*/
QMediaSeekWidget::~QMediaSeekWidget()
{
    delete m_d;
}

/*!
    \fn void QMediaSeekWidget::lengthChanged( quint32 ms );

    This signal is emitted when the total time of the playing media content has
    changed. The time is given by \a ms in milliseconds.
*/

/*!
    \fn void QMediaSeekWidget::positionChanged( quint32 ms );

    This signal is emitted when the seek position has changed. While the user
    is not seeking this signal will be emitted when the position has changed
    in the media content object. The position is given by \a ms in
    milliseconds.

*/

/*!
    \fn QMediaContent* QMediaSeekWidget::content() const

    Returns the media content object currently being used.
*/
QMediaContent* QMediaSeekWidget::content() const
{
    return m_context->content();
}

/*!
    \fn void QMediaSeekWidget::setMediaContent( QMediaContent* content )

    Sets \a content as the media content object to use.
*/
void QMediaSeekWidget::setMediaContent( QMediaContent* content )
{
    m_context->setMediaContent( content );
}

/*!
    \reimp
*/
void QMediaSeekWidget::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Left:
        {
        SeekTask *seekbackward = new SeekTask( SeekTask::SeekBackward, m_d->progress );
        connect( seekbackward, SIGNAL(iterationComplete()), m_d->monitor, SLOT(update()) );
        m_d->task = seekbackward;
        m_d->task->start();
        }
        break;
    case Qt::Key_Right:
        {
        SeekTask *seekforward = new SeekTask( SeekTask::SeekForward, m_d->progress );
        connect( seekforward, SIGNAL(iterationComplete()), m_d->monitor, SLOT(update()) );
        m_d->task = seekforward;
        m_d->task->start();
        }
        break;
    default:
        // Ignore
        break;
    }
}

/*!
    \reimp
*/
void QMediaSeekWidget::keyReleaseEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Left:
    case Qt::Key_Right:
        delete m_d->task;
        break;
    default:
        // Ignore
        break;
    }
}

#include "qmediawidgets.moc"
