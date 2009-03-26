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

#include "qwaitwidget.h"
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHideEvent>
#include <QPainter>
#include <QBasicTimer>
#include <QDesktopWidget>
#include <QImageReader>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

static QRgb blendRgb( QRgb rgb, int sr, int sg, int sb )
{
    int tmp = ( rgb >> 16 ) & 0xff;
    int r = ( ( sr + tmp ) / 2 );
    tmp = ( rgb >> 8 ) & 0xff;
    int g = ( ( sg + tmp ) / 2 );
    tmp = rgb & 0xff;
    int b = ( ( sb + tmp ) / 2 );
    return qRgba( r, g, b, qAlpha( rgb ) );
}

class ClockLabel : public QWidget
{
    Q_OBJECT
public:
    ClockLabel(QWidget *parent);

    void start() { timer.start(250, this); }
    void stop() { timer.stop(); angle = 180; }

    void setBlendColor(const QColor &col) { blendColor = col; loadImage(); }

    QSize sizeHint() const;
    void loadImage();

protected:
    void paintEvent(QPaintEvent *e);
    void timerEvent(QTimerEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    QPixmap waitIcon;
    int angle;
    QColor blendColor;
    QBasicTimer timer;
};

ClockLabel::ClockLabel(QWidget *parent) : QWidget(parent), angle(180)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}

QSize ClockLabel::sizeHint() const
{
    QSize dsize = QApplication::desktop()->screenGeometry().size();
    return dsize/4;
}

void ClockLabel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap((width()-waitIcon.width())/2, (height()-waitIcon.height())/2,waitIcon);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 2));
    p.translate(qreal(rect().width())/2.0, qreal(rect().height())/2.0);
    p.rotate(angle);
    p.drawLine(0,0,0,waitIcon.width()/2-waitIcon.width()/8);
}

void ClockLabel::resizeEvent(QResizeEvent *)
{
    loadImage();
}

void ClockLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timer.timerId()) {
        angle += 6;
        if (angle >= 360)
            angle = 0;
        update();
    }
}

void ClockLabel::loadImage()
{
    int imgSize = qMin(size().width(), size().height());
    QImageReader imgReader(":image/qpe/clock");
    imgReader.setQuality( 49 ); // Otherwise Qt smooth scales
    imgReader.setScaledSize(QSize(imgSize, imgSize));
    QImage img = imgReader.read();

    if (blendColor.isValid()) {
        int sr, sg, sb;
        blendColor.getRgb( &sr, &sg, &sb );
        if ( img.depth() == 32 ) {
            QRgb *rgb = (QRgb *)img.bits();
            int bytesCount = img.bytesPerLine()/sizeof(QRgb)*img.height();
            for ( int r = 0; r < bytesCount; r++, rgb++ )
                *rgb = blendRgb( *rgb, sr, sg, sb );
        } else {
            QVector<QRgb> rgb = img.colorTable();
            for ( int r = 0; r < rgb.count(); r++ )
                rgb[r] = blendRgb( rgb[r], sr, sg, sb );
            img.setColorTable( rgb );
        }
    }

    waitIcon = QPixmap::fromImage(img);
}

//===========================================================================

class QWaitWidgetPrivate
{
public:
    QLabel *textLabel;
    ClockLabel *imageLabel;
    bool cancelEnabled;
    bool wasCancelled;
    int expiryTime;
};

/*!
    \class QWaitWidget
    \inpublicgroup QtBaseModule

    \brief The QWaitWidget class provides an informative idle screen
    for a slow operation.

    A wait widget is used to give the user an indication
    that an operation is going to take some time,
    and to demonstrate that the application has not frozen.
    It can also give the user an opportunity to abort the operation
    when setCancelEnabled() is set to be true.

    For example, construct a QWaitWidget to popup over \a parent.
    \code
        QWaitWidget *waitWidget = new QWaitWidget(this);
        waitWidget->show();
        // do time consuming operations
        delete waitWidget;
    \endcode

    Use setText() and setCancelEnabled() to give more
    feedback and control to the user. For example,
    \code
        QWaitWidget *waitWidget = new QWaitWidget(this);
        waitWidget->setCancelEnabled( true );
        waitWidget->setText( "Searching..." );
        waitWidget->show();

        QDir dir = QDir::current();
        QFileInfoList list = dir.entryInfoList();
        int totalSize = 0;

        for ( int i = 0; i < dir.count(); i++ ) {
            totalSize += list.at( i ).size();
           waitWidget->setText( QString( "Size: %1 bytes" ).arg( QString::number(totalSize) ) );
        }
        waitWidget->hide();
    \endcode
    The wait widget, in this example, emits a signal cancelled()
    when the user presses the Cancel button.
    \image qwaitwidget.png "A wait widget with text"
*/

/*!
  Constructs an QWaitWidget object with the given \a parent.
*/
QWaitWidget::QWaitWidget(QWidget *parent)
    : QDialog( parent )
{
    d = new QWaitWidgetPrivate;

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 2 );
    layout->setSpacing( 2 );

    d->textLabel = new QLabel( this );
    d->textLabel->setWordWrap( true );
    d->textLabel->setAlignment(Qt::AlignHCenter);
    d->imageLabel = new ClockLabel( this );

    layout->addStretch( 0 );
    layout->addWidget( d->imageLabel );
    layout->addWidget( d->textLabel );
    layout->addStretch( 0 );
    layout->setAlignment( d->textLabel, Qt::AlignHCenter | Qt::AlignVCenter );
    layout->setAlignment( d->imageLabel, Qt::AlignHCenter | Qt::AlignVCenter );

    setWindowTitle( tr( "Please wait" ) );
    setModal( true );
    setWindowFlags( Qt::SplashScreen );

    QSoftMenuBar::removeMenuFrom( this, QSoftMenuBar::menuFor( this ) );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::NoLabel );
    d->cancelEnabled = false;
    d->wasCancelled = false;
    d->expiryTime = 0;
}

/*!
    Destroys a QWaitWidget
    */
QWaitWidget::~QWaitWidget()
{
    delete d;
}

/*!
    \property QWaitWidget::cancelEnabled
    \brief indicates whether the Cancel button appears on the context menu.
*/

/*!
    Sets whether the Cancel button appears on the context menu to \a enabled.
    When the Cancel button is presed the signal cancelled() is emitted.

    \sa cancelled()
*/
void QWaitWidget::setCancelEnabled(bool enabled)
{
    if (enabled) {
        d->cancelEnabled = true;
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    }
    else {
        d->cancelEnabled = false;
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::NoLabel );
    }
}

/*!
    \property QWaitWidget::wasCancelled
    \brief indicates whether the widget was cancelled by the user when
    it was last shown.
*/

/*!
    Returns whether the widget was cancelled by the user when it was last
    shown.
 */
bool QWaitWidget::wasCancelled() const
{
    return d->wasCancelled;
}

/*!
    Hides the widget after \a msec.
*/
void QWaitWidget::setExpiryTime( int msec )
{
    d->expiryTime = msec;
}

/*!
  Shows the widget and its child widgets. The widget is shown maximized.
*/
void QWaitWidget::show()
{
    if ( d->expiryTime != 0 )
        QTimer::singleShot( d->expiryTime, this, SLOT(timeExpired()) );
    d->imageLabel->start();
    QDialog::showMaximized();

    d->wasCancelled = false;
}

/*!
  Hides the widget and its child widgets.
*/
void QWaitWidget::hide()
{
    reset();
    QDialog::hide();
}

void QWaitWidget::timeExpired()
{
    hide();
}

/*!
  \fn void QWaitWidget::setText( const QString &label )

   Sets the informative text \a label for this wait widget.
*/
void QWaitWidget::setText( const QString &str )
{
    d->textLabel->setText( str );
}

/*!
   Blends the image with color \a col.
*/
void QWaitWidget::setColor( const QColor &col )
{
    d->imageLabel->setBlendColor(col);
}

/*!
  \reimp
*/
void QWaitWidget::hideEvent( QHideEvent *e )
{
    reset();
    QDialog::hideEvent( e );
}

/*!
  \reimp
*/
void QWaitWidget::done(int r)
{
    if (d->cancelEnabled && d->wasCancelled)
        QDialog::done(r);
}

/*!
  \reimp
*/
void QWaitWidget::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Back && d->cancelEnabled ) {
        d->wasCancelled = true;
        emit cancelled();
        QDialog::keyPressEvent( e );
    } else {
        e->accept();
    }
}

void QWaitWidget::reset()
{
    d->imageLabel->stop();
}

/*!
  \fn void QWaitWidget::cancelled()

  This signal is emitted whenever the wait widget dialog is cancelled by user.
*/

#include "qwaitwidget.moc"
