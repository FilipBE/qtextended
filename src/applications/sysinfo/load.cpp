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

#include <stdio.h>
#include <sys/time.h>
#include <QLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include "load.h"

LoadInfo::LoadInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(20, this, SLOT(init()));
}

void LoadInfo::init()
{
    QVBoxLayout *vb = new QVBoxLayout( this );

    QString cpuInfo = getCpuInfo();
    if ( !cpuInfo.isNull() )
        vb->addWidget( new QLabel( cpuInfo, this ) );
    vb->addWidget( new Load( this ), 100 );

    QHBoxLayout *hb = new QHBoxLayout;
    vb->addLayout(hb);
    QLabel *l = new QLabel( this );
    l->setPixmap( makeLabel( Qt::red ) );
    hb->addWidget( l );
    l = new QLabel(tr("Application CPU (%)"), this );
    hb->addWidget( l );
    hb->addStretch(20);

    hb = new QHBoxLayout;
    vb->addLayout(hb);
    l = new QLabel( this );
    l->setPixmap( makeLabel( Qt::green ) );
    hb->addWidget( l, 1 );
    l = new QLabel(tr("System CPU (%)"), this );
    hb->addWidget( l );
    hb->addStretch(20);
    vb->addStretch(50);
}

QPixmap LoadInfo::makeLabel( const QColor &col )
{
    int h = fontMetrics().height();
    QPixmap pm( 20 , h );
    QPainter p( &pm );
    p.fillRect( pm.rect(), palette().background() );
    p.fillRect( 0, 0, 20, h, Qt::black );
    p.setPen( col );
    p.drawLine( 2, h/2, 17, h/2 );
    return pm;
}

QString LoadInfo::getCpuInfo()
{
    QString info = tr("Type: ");
    bool haveInfo = false;
    QFile f( "/proc/cpuinfo" );
    if ( f.open( QFile::ReadOnly ) ) {
        QTextStream ts( &f );

        while ( !ts.atEnd() ) {
            QString s = ts.readLine();
            if ( s.indexOf( "model name" ) == 0 ) { // No tr
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
            } else if ( s.indexOf( "cpu MHz" ) == 0 ) {
                double mhz = s.mid( s.indexOf( ':' ) + 2 ).toDouble();
                info += ' ' + QString::number( mhz, 'f', 0 );
                info += "MHz";
                break;
            } else if ( s.indexOf( "Processor" ) == 0 ) { // No tr
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
                break;
#ifdef __MIPSEL__
            } else if ( s.indexOf( "cpu model" ) == 0 ) { // No tr
                info += ' ' + s.mid( s.indexOf( ':' ) + 2 );
                break;
            } else if ( s.indexOf( "cpu" ) == 0 ) {
                info += s.mid( s.indexOf( ':' ) + 2 );
                haveInfo = true;
#endif
            }
        }
    }

    if ( !haveInfo )
        info = QString();

    return info;
}

Load::Load( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f ), lastUser(0), lastSys(0)
{
    setMinimumHeight( 30 );
    QPalette pal(palette());
    pal.setColor(backgroundRole(), Qt::black);
    setPalette(pal);

    points = 100;
    setMinimumWidth( points );
    userLoad = new double [points];
    systemLoad = new double [points];
    for ( int i = 0; i < points; i++ ) {
        userLoad[i] = 0.0;
        systemLoad[i] = 0.0;
    }
    maxLoad = 1.3;
    QTimer *timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(timeout()) );
    timer->start( 2000 );
    gettimeofday( &last, 0 );
    first = true;
    timeout();
}

Load::~Load()
{
    delete [] userLoad;
    delete [] systemLoad;
}

void Load::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int h = height() - 5;

    int mult = (int)(h / maxLoad);

    p.setPen( Qt::gray );
    p.drawLine( 0, h - mult, width(), h - mult );
    p.drawText( 0, h - mult, "100" );
    p.drawText( 0, h, "0" );

    p.setPen( Qt::green );
    for ( int i = 1; i < points; i++ ) {
        int x1 = (i - 1) * width() / points;
        int x2 = i * width() / points;
        p.drawLine( x1, h - int(systemLoad[i-1] * mult),
                    x2, h - int(systemLoad[i] * mult) );
    }

    p.setPen( Qt::red );
    for ( int i = 1; i < points; i++ ) {
        int x1 = (i - 1) * width() / points;
        int x2 = i * width() / points;
        p.drawLine( x1, h - int(userLoad[i-1] * mult),
                    x2, h - int(userLoad[i] * mult) );
    }
}

void Load::timeout()
{
    int user;
    int usernice;
    int sys;
    int idle;
    FILE *fp;
    fp = fopen( "/proc/stat", "r" );
    fscanf( fp, "cpu %d %d %d %d", &user, &usernice, &sys, &idle );
    user += usernice;
    fclose( fp );
    struct timeval now;
    gettimeofday( &now, 0 );
    int tdiff = now.tv_usec - last.tv_usec;
    tdiff += (now.tv_sec - last.tv_sec) * 1000000;
    tdiff /= 10000;

    int udiff = user - lastUser;
    int sdiff = sys - lastSys;
    if ( tdiff > 0 ) {
        double uload = (double)udiff / (double)tdiff;
        double sload = (double)sdiff / (double)tdiff;
        if ( !first ) {
            for ( int i = 1; i < points; i++ ) {
                userLoad[i-1] = userLoad[i];
                systemLoad[i-1] = systemLoad[i];
            }
            userLoad[points-1] = uload;
            systemLoad[points-1] = sload;
//          scroll( -width()/points, 0, QRect( 0, 0, width() - width()/points + 1, height() ) );
            repaint();
            double ml = 1.3;
            /*
            for ( int i = 0; i < points; i++ ) {
                if ( userLoad[i] > ml )
                    ml = userLoad[i];
            }
            */
            if ( maxLoad != ml ) {
                maxLoad = ml;
                update();
            }
        }

        last = now;
        lastUser = user;
        lastSys = sys;
        first = false;
    } else if ( tdiff < 0 ) {
        last = now;
    }
}
