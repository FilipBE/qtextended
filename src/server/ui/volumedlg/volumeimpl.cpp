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

#include "volumeimpl.h"
#include <QKeyEvent>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QValueSpaceItem>

#include <math.h>
#include "qtopiaserverapplication.h"
#include "mediaservicestask.h"


class VolumeWidget : public QWidget
{

public:
    VolumeWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0)
        : QWidget(parent, flags), m_steps(10), m_current(0)
    {
    }

    ~VolumeWidget() {}

    int heightForWidth(int width) const
    {
        return qRound(0.33 * width);
    }

    int value() const
    {
        return m_current;
    }

    int steps() const
    {
        return m_steps;
    }

    void setCurrent(int i)
    {
        if ((i <= 0) || (i > m_steps))
            return;

        m_current = i;
        update();
    }

    QLabel *l;
    QLabel *m;

protected:
    void paintEvent(QPaintEvent* /*event*/)
    {
        int w = rect().width() - (3 * m_steps);
        barWidth = qRound(w / (m_steps - 1));
        barHeight = qRound(rect().height() / (m_steps - 1));

        QPainter painter(this);
        painter.setPen(palette().text().color());
        painter.setBrush(palette().highlight());

        for (int n = 1; n < m_current; n++) {
            QRect r;
            r.setTopLeft(QPoint(((n-1) * barWidth) + (n * 3) - 1, (m_steps-1-n)*barHeight));
            r.setWidth(barWidth);
            r.setHeight(n * barHeight);
            painter.drawRect(r);
        }
    }

    QSizePolicy sizePolicy() const
    {
        return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    }

    QSize sizeHint() const
    {
        return QSize(27, 9);
    }

private:
    int m_steps;
    int m_current;
    int barWidth;
    int barHeight;
};

class VolumeDialogImplPrivate
{
public:
    VolumeDialogImplPrivate();
    ~VolumeDialogImplPrivate();

    QValueSpaceItem* m_vsVolume;

    int  old_volume[5];
    bool state;
    bool firstUpdate;
};

VolumeDialogImplPrivate::VolumeDialogImplPrivate()
{
    m_vsVolume  = new QValueSpaceItem("/System/Volume/CurrentVolume");
    old_volume[0] = 100;
    old_volume[1] = 1;
    old_volume[2] = 2;
    old_volume[3] = 3;
    state = false;
    firstUpdate = false;
}

VolumeDialogImplPrivate::~VolumeDialogImplPrivate()
{
    delete m_vsVolume;
}


VolumeDialogImpl::VolumeDialogImpl( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), m_tid(0), m_oldValue(0), m_d(new VolumeDialogImplPrivate)
{
    screenUpdate();

    QColor c(Qt::black);
    c.setAlpha(255);     //XXX: Make fully opaque for now, for  DirectPainter widgets in the background

    setAttribute(Qt::WA_SetPalette, true);

    QPalette p = palette();
    p.setBrush(QPalette::Window, c);
    setPalette(p);

    QVBoxLayout *vBox = new QVBoxLayout(this);
    QHBoxLayout *hBox = new QHBoxLayout;

    volumeWidget = new VolumeWidget(this);

    QIcon icon(":icon/sound");
    QIcon mute(":icon/mute");

    QLabel* volumeLevel = new QLabel(this);
    volumeLevel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    volumeLevel->setMinimumWidth( fontMetrics().width("100%") );
    volumeLevel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    connect(this, SIGNAL(setText(QString)), volumeLevel, SLOT(setText(QString)));

    volumeWidget->l = new QLabel(this);
    volumeWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    volumeWidget->l->setPixmap(icon.pixmap(64, 64));
    volumeWidget->m = new QLabel(this);

    volumeWidget->m->hide();
    volumeWidget->m->setPixmap(mute.pixmap(64, 64));

    hBox->addStretch();
    hBox->addWidget(volumeWidget->l);
    hBox->addWidget(volumeWidget->m);
    hBox->addStretch();

    QHBoxLayout *wp = new QHBoxLayout;
    wp->addWidget(volumeWidget);
    wp->addWidget(volumeLevel);

    vBox->addLayout(hBox);
    vBox->addLayout(wp);

    connect(m_d->m_vsVolume, SIGNAL(contentsChanged()), this, SLOT(valueSpaceVolumeChanged()));
    initialized = 0;
    old_slot = 0;

    resetTimer();
}

void VolumeDialogImpl::screenUpdate()
{
    QRect d = QApplication::desktop()->screenGeometry();
    int dw = d.width();
    int dh = d.height();
    setGeometry(20*dw/100, 30*dh/100, 60*dw/100, 40*dh/100);
}

void VolumeDialogImpl::timerEvent( QTimerEvent *e )
{
    m_d->state = false;
    m_d->old_volume[2] = 0;
    m_d->old_volume[3] = 0;

    if (m_tid > 0)
        killTimer( m_tid );

    Q_UNUSED(e)
    close();
}

int VolumeDialogImpl::setVolume( bool up)
{
    int ret = (volumeWidget->value() == m_oldValue);
    m_oldValue = volumeWidget->value();
    volumeWidget->setCurrent( up ? m_oldValue + 1 : m_oldValue - 1 );
    int value = volumeWidget->value();

    if ( m_oldValue < value )
        emit volumeChanged( true );
    else if ( m_oldValue > value )
        emit volumeChanged( false );

    resetTimer();

    return ret;
}

void VolumeDialogImpl::resetTimer()
{
    if (m_tid > 0)
        killTimer( m_tid );
    m_tid = startTimer( TIMEOUT );
}

void VolumeDialogImpl::valueSpaceVolumeChanged()
{
    QValueSpaceItem vol("/System/Volume/CurrentVolume");
    int volume = vol.value().toInt();

    int slot =   qBound(1,(int)::ceil(volume/(volumeWidget->steps()))+1,10);
    int slot2 =  qBound(1,volume+1,100);

    volumeWidget->setCurrent( slot );
    initialized++;

    resetTimer();

    if(initialized < 3) // Ignore initial emit on connect to valueSpace item
        return;

    if((volume == 0) || (volume == 100)) {
        // Handle press and hold of volume keys at max or min!
        if(m_d->state) {
            m_d->old_volume[3] = m_d->old_volume[2];
            m_d->old_volume[2] = m_d->old_volume[1];
            m_d->old_volume[1] = m_d->old_volume[0];
            m_d->old_volume[0] = volume;
            if(!((m_d->old_volume[1] == volume) && (m_d->old_volume[2] == volume) &&
                        (m_d->old_volume[3] == volume)))
                return;
        } else {
            m_d->state = true;
            return;
        }
    } else if(volume == m_d->old_volume[0]) {
        return;
    } else {
        m_d->old_volume[3] = m_d->old_volume[2];
        m_d->old_volume[2] = m_d->old_volume[1];
        m_d->old_volume[1] = m_d->old_volume[0];
        m_d->old_volume[0] = volume;
    }

    screenUpdate();

    if(!m_d->firstUpdate) {
        // Ignore the first actual update as that would be startup
        m_d->firstUpdate = true;
        return;
    }

    QString str;
    str.setNum(volume);
    old_slot = slot2;
    emit setText(str+"%");
    if(volume > 0)
    {
        volumeWidget->m->hide();
        volumeWidget->l->show();
    }
    else
    {
        volumeWidget->l->hide();
        volumeWidget->m->show();
        volumeWidget->setCurrent( 1 );
    }
    this->show();
}

/*!
  \class VolumeDialogTask
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
  \ingroup QtopiaServer::GeneralUI
  \brief The VolumeDialogTask class provides a volume dialog that gives feedback on volume changes.

  The volume dialog appears in response to the VolumeControlTask::volumeChanged() signal which is triggered
  when the user presses the \c Qt::Key_VolumeUp or \c Qt::Key_VolumeDown keys.
  If the appearance of this dialog is not suitable for the target device this task should be replaced by a 
  more suitable implementation.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
  \internal
  */
VolumeDialogTask::VolumeDialogTask( QObject *parent )
    : QObject( parent )
{
    if(qtopiaTask<VolumeControlTask>()) {
        //QObject::connect(qtopiaTask<VolumeControlTask>(), SIGNAL(volumeChanged(bool)), this, SLOT(volumeChanged(bool)));
        // NOTE: The Volume Widget is now modified via ValueSpace Item: /Volume/GlobalVolume
        static VolumeDialogImpl *vd = 0;
        if ( !vd ) {
            vd = new VolumeDialogImpl(0, Qt::Tool| Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        }

    }
}


/*!
  \internal
  */
VolumeDialogTask::~VolumeDialogTask()
{
}

/*!
  \internal
  */
void VolumeDialogTask::volumeChanged(bool up)
{
    static VolumeDialogImpl *vd = 0;
    if ( !vd ) {
        vd = new VolumeDialogImpl(0, Qt::Tool| Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    }
    if(vd->setVolume(up))
        vd->show();
}

QTOPIA_TASK(VolumeDialogTask, VolumeDialogTask);
