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

// Qt4 Headers
#include <QGridLayout>
#include <QBoxLayout>

#include <QTimerEvent>
#include <QTimer>
#include <QMenu>
#include <QSettings>
#include <QDesktopWidget>
#include <QDebug>
#include <QPainter>
#include <QResizeEvent>
// Local includes
#include "worldtime.h"

// Qtopia includes
#include <qtopiaapplication.h>
#include <QWorldmap>
#include <QWorldmapDialog>

#include <qtimestring.h>
#include <qtimezone.h>
#include <qtimezoneselector.h>
#include <qtopialog.h>
#ifndef QTOPIA_HOST
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#endif
#include <qsoftmenubar.h>
#include <QAction>
#include <QSpacerItem>

WorldTime::WorldTime( QWidget *parent,
                      Qt::WFlags fl )
   : QStackedWidget( parent )
{
    if (fl) setWindowFlags(fl);
    setWindowTitle(tr("World Time"));

    maxVisibleZones = 5; // XXX variable?

    isEditMode = false;

    // Time zones page...

    int columns/*,rows*/;
    columns = 3;/*rows = 3;*/
    zones = new QWidget(this);
    QBoxLayout *gl = new QBoxLayout(QBoxLayout::TopToBottom,  zones);
    gl->addStretch(6);

    for (int i = 0; i < maxVisibleZones; i++) {
        listCities.append(new QPushButton(zones));

         listCities.at(i)->setMinimumHeight( qApp->desktop()->availableGeometry().height()
                                             / maxVisibleZones + 1);
        connect(listCities.at(i), SIGNAL(clicked()),
                this, SLOT(slotSetZone()));

        listTimes.append(new CityInfo(zones));
        listTimes.at(i)->hide();
        gl->addWidget( listCities.at(i), 1,  Qt::Alignment(Qt::AlignBottom));

    }

    gl->setSpacing(4);
    gl->setMargin(0);

    readInTimes();
    changed = false;
    QObject::connect( qApp, SIGNAL(clockChanged(bool)),
                     this, SLOT(showTime()));
    addWidget(zones);


    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addSeparator();

    QAction *a = new QAction(QIcon(":icon/cancel"), tr("Show Map"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(showMap()));
    contextMenu->addAction(a);
    contextMenu->addSeparator();

    a = new QAction(QIcon(":icon/cancel"), tr("Cancel"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(cancelChanges()));
    contextMenu->addAction(a);


    QSoftMenuBar::addMenuTo( this, contextMenu );

     if( !Qtopia::mousePreferred())
         QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);

// now start the timer so we can update the time quickly every second
    timerEvent( 0 );

    //delay the init of this big thing
    QTimer::singleShot(200, this, SLOT(initWorldDialog()));

}

WorldTime::~WorldTime()
{
}

void WorldTime::initWorldDialog()
{
    worldMapDialog = new QWorldmapDialog(this);
}

void WorldTime::saveChanges()
{
    if (changed) {
        writeTimezoneChanges();
        readInTimes();
        changed = false;
    }
}

void WorldTime::cancelChanges()
{
    changed = false;
    viewMode();
}

void WorldTime::writeTimezoneChanges()
{
    changed = true;
    QSettings cfg("Trolltech", "WorldTime");
    cfg.beginGroup("TimeZones");

    int i;
    for ( i = 0;  i < maxVisibleZones; i++) {
        if ( !strCityTz[i].isNull() ) {
            cfg.setValue("Zone"+QString::number(i), strCityTz[i]);
        }
    }

    cfg.sync();

    emit timeZoneListChange();
}

void WorldTime::timerEvent( QTimerEvent *)
{
    if ( timerId ){
        killTimer( timerId );
        timerId = 0;
    }
    // change the time again!!
    showTime();
    int ms = 1000 - QTime::currentTime().msec();
    ms += (60-QTime::currentTime().second())*1000;
    timerId = startTimer( ms );
}

void WorldTime::keyPressEvent( QKeyEvent* ke )
{
    if ( ke->key()==Qt::Key_Back ) {
        if ( !isEditMode ) {
            saveChanges();
        }
    }
    QStackedWidget::keyPressEvent(ke);
}

void WorldTime::showTime( void )
{
    QDateTime curUtcTime = QTimeZone::utcDateTime();

    for (int i=0; i< maxVisibleZones; i++)
        listTimes.at(i)->setUtcTime(curUtcTime);
    readInTimes();

}

void WorldTime::slotSetZone()
{

    QPushButton *sendButton = qobject_cast<QPushButton *>(sender());
    for (selButton = 0; selButton < maxVisibleZones && listCities.at(selButton)
                     != sendButton; selButton++)
        ;
    if (selButton == maxVisibleZones)
        return;

    QTimeZone zone( strCityTz[selButton].toLocal8Bit());
    worldMapDialog->setZone(zone);
    editMode();
}

void WorldTime::editMode()
{
    setWindowTitle(tr("Select City"));
    isEditMode = true;
    changed = true;
    if ( QtopiaApplication::execDialog(worldMapDialog) == QDialog::Accepted
       && worldMapDialog->selectedZone().isValid()) {
              slotNewTz( worldMapDialog->selectedZone() );
        }
}


void WorldTime::viewMode()
{
    setWindowTitle(tr("World Time"));
    isEditMode = false;
    setCurrentWidget(zones);
    saveChanges();

}

void WorldTime::slotNewTz( const QTimeZone& zone )
{
    if (selButton > -1 ) {
        strCityTz[selButton] = zone.id();
        listCities.at(selButton)->setText( zone.city());
        listTimes.at(selButton)->setZone( zone.id());
        changed = true;
    }
    viewMode();
}

void WorldTime::slotNewTzCancelled()
{
    qWarning() << "cancelled";
    viewMode();
}

void WorldTime::readInTimes( )
{
    QSettings cfg("Trolltech", "WorldTime");
    cfg.beginGroup("TimeZones");

    int i;
    QString zn;

    //create zoneslist
    for (i = 0; i < maxVisibleZones; i++ ) {
        zn = cfg.value("Zone" + QString::number(i)).toString();
        strCityTz[i] = zn;

        if ( zn.isEmpty() )
            break;

        QString nm =  zn.section("/",-1) ;
        nm = nm.replace("_"," ");
        strCityTz[i] = zn;

        zn = cfg.value("Zone" + QString::number(i), QString(i)).toString();
        listTimes.at(i)->setZone(zn);

          listCities.at(i)->setText( nm + "    "+ listTimes.at(i)->text() );
    }
}

void WorldTime::resizeEvent(QResizeEvent * /*event*/)
{
    QTimer::singleShot(0,this, SLOT(resetButtons()));
}

void WorldTime::resetButtons()
{
    qWarning() <<  qApp->desktop()->availableGeometry().width();
    for (int i = 0; i < maxVisibleZones; i++) {
        listCities.at(i)->setMinimumHeight( qApp->desktop()->availableGeometry().height() / maxVisibleZones + 1);
    }
}

void WorldTime::showMap()
{
    QtopiaApplication::execDialog(worldMapDialog);
    //  QWorldMap frmMap = new QWorldmap(0);
    //  QSizePolicy sp = frmMap->sizePolicy();

}
