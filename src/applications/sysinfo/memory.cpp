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

#include <QLabel>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include "graph.h"
#include "memory.h"

MemoryInfo::MemoryInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(30, this, SLOT(init()));
}

MemoryInfo::~MemoryInfo()
{
    delete data;
}

void MemoryInfo::init()
{
    QVBoxLayout *vb = new QVBoxLayout( this );

    totalMem = new QLabel( this );
    vb->addWidget( totalMem );

    data = new GraphData();
    graph = new BarGraph( this );
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph, 1 );
    graph->setData( data );

    legend = new GraphLegend( this );
    vb->addWidget( legend );
    legend->setData( data );

    vb->addStretch(100);

    updateData();

    startTimer(5000);
}

void MemoryInfo::timerEvent(QTimerEvent *)
{
    if (isVisible())
        updateData();
}

void MemoryInfo::updateData()
{
    QFile file( "/proc/meminfo" );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream t( &file );
        QString all = t.readAll();
        int total=0, memfree=0, buffers=0, cached = 0;
        int pos = 0;
        QRegExp regexp("(MemTotal:|MemFree:|Buffers:|\\bCached:)\\s*(\\d+) kB");
        while ( (pos = regexp.indexIn( all, pos )) != -1 ) {
            if ( regexp.cap(1) == "MemTotal:" )
                total = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "MemFree:" )
                memfree = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Buffers:" )
                buffers = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Cached:" )
                cached = regexp.cap(2).toInt();
            pos += regexp.matchedLength();
        }

        int realUsed = total - ( buffers + cached + memfree );
        data->clear();
        data->addItem( tr("Used (%1 kB)").arg(realUsed), realUsed );
        data->addItem( tr("Buffers (%1 kB)").arg(buffers), buffers );
        data->addItem( tr("Cached (%1 kB)").arg(cached), cached );
        data->addItem( tr("Free (%1 kB)").arg(memfree), memfree );
        totalMem->setText( tr( "Total Memory: %1 kB" ).arg( total ) );
        graph->repaint();
        legend->update();
    }
}

