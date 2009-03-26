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

#include "siminfo.h"
#include "graph.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QSimInfo>
#include <QDebug>

SimInfo::SimInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(70, this, SLOT(init()));
}

SimInfo::~SimInfo()
{
    delete smsData;
    delete pbData;
}

void SimInfo::init()
{
    simInf = new QSimInfo();
    sms = new QSMSReader( QString(), this );
    pb = new QPhoneBook( QString(), this );
    pbused = -1;
    pbtotal = -1;

    connect( simInf, SIGNAL(removed()), this, SLOT(updateData()));
    connect( simInf, SIGNAL(inserted()), this, SLOT(updateData()));
    connect( sms, SIGNAL(unreadCountChanged()), this, SLOT(updateData()) );
    connect( sms, SIGNAL(messageCount(int)), this, SLOT(updateData()) );

    connect( pb, SIGNAL(limits(QString,QPhoneBookLimits)),
             this, SLOT(limits(QString,QPhoneBookLimits)) );
    pb->requestLimits();

    QVBoxLayout *vb = new QVBoxLayout( this );

    header = new QLabel(this);
    vb->addWidget( header);
    smsData = new GraphData();
    smsGraph = new BarGraph( this );
    smsGraph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( smsGraph, 1 );
    smsGraph->setData( smsData );
    smsLegend = new GraphLegend( this );
    vb->addWidget( smsLegend );
    smsLegend->setData( smsData );

    pbData = new GraphData();
    pbGraph = new BarGraph( this );
    pbGraph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( pbGraph, 1 );
    pbGraph->setData( pbData );
    pbLegend = new GraphLegend( this );
    vb->addWidget( pbLegend );
    pbLegend->setData( pbData );

    vb->addStretch(100);

    updateData();
}

void SimInfo::updateData()
{
    if(simInf->identity() != QString("")) {
        header->setText(tr("SIM ID: %1").arg( simInf->identity()));
        // Update the sms information.
        int smsUsed = sms->usedMessages();
        int smsTotal = sms->totalMessages();
        smsData->clear();
        if ( smsUsed != -1 && smsTotal != -1 ) {
            smsData->addItem( tr("SMS Used: %1", "%1=number").arg(smsUsed), smsUsed );
            smsData->addItem( tr("SMS Free: %1", "%1=number").arg(smsTotal - smsUsed),
                          smsTotal - smsUsed );
        } else {
            smsData->addItem( tr("SMS Used: Please wait"), 0 );
            smsData->addItem( tr("SMS Free: Please wait"), 1 );
        }
        smsGraph->repaint();
        smsLegend->update();
        smsGraph->show();
        smsLegend->show();

        // Update the phone book information.
        pbData->clear();
        if ( pbused != -1 && pbtotal != -1 ) {
            pbData->addItem( tr("Contacts Used: %1", "%1=number").arg(pbused), pbused );
            pbData->addItem( tr("Contacts Free: %1", "%1=number").arg(pbtotal - pbused),
                          pbtotal - pbused );
        } else {
            pbData->addItem( tr("Contacts Used: Please wait"), 0 );
            pbData->addItem( tr("Contacts Free: Please wait"), 1 );
        }
        pbGraph->repaint();
        pbLegend->update();
        pbGraph->show();
        pbLegend->show();
    } else {
        header->setText(tr("No SIM detected..."));
        smsGraph->hide();
        smsLegend->hide();
        pbGraph->hide();
        pbLegend->hide();
    }
}

void SimInfo::limits( const QString& store, const QPhoneBookLimits& value )
{
    if ( store == "SM" ) {    // No tr
        pbused = (int)( value.used() );
        pbtotal = (int)( value.lastIndex() - value.firstIndex() + 1 );
        if ( pbtotal <= 1 ) {
            // No information available.
            pbused = -1;
            pbtotal = -1;
        }
        updateData();
    }
}
