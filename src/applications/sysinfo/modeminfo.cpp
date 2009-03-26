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

#include <QApplication>
#include <QKeyEvent>
#include <QLayout>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTelephonyConfiguration>
#include <QTimer>
#include "modeminfo.h"

ModemInfo::ModemInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(60, this, SLOT(init()));
}

ModemInfo::~ModemInfo()
{
}

void ModemInfo::init()
{
    infoDisplay = new QTextBrowser();
    infoDisplay->installEventFilter( this );
    infoDisplay->setFrameShape( QFrame::NoFrame );
    infoDisplay->setTextInteractionFlags(Qt::NoTextInteraction);
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 0 );
    vb->setMargin( 0 );
    vb->addWidget( infoDisplay );

    infoDisplay->setHtml( format() );

    QTelephonyConfiguration *config
            = new QTelephonyConfiguration( "modem" );   // No tr
    connect( config, SIGNAL(notification(QString,QString)),
             this, SLOT(configValue(QString,QString)) );
    config->request( "manufacturer" );      // No tr
    config->request( "model" );             // No tr
    config->request( "revision" );          // No tr
    config->request( "serial" );            // No tr
    config->request( "extraVersion" );      // No tr
}

bool ModemInfo::eventFilter( QObject* /*watched*/, QEvent *event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QScrollBar* sb = infoDisplay->verticalScrollBar();
        int key = ((QKeyEvent*)event)->key();
        if ( key == Qt::Key_Down )
            sb->triggerAction( QAbstractSlider::SliderSingleStepAdd );
        else if ( key == Qt::Key_Up )
            sb->triggerAction( QAbstractSlider::SliderSingleStepSub );
        else
            return false;
        return true;
    }
    return false;
}

void ModemInfo::configValue( const QString& name, const QString& value )
{
    if ( name == "manufacturer" )       // No tr
        manufacturer = value;
    else if ( name == "model" )         // No tr
        model = value;
    else if ( name == "revision" )      // No tr
        revision = value;
    else if ( name == "serial" )        // No tr
        serial = value;
    else if ( name == "extraVersion" )  // No tr
        extraVersion = value;
    infoDisplay->setHtml( format() );
}

QString ModemInfo::format()
{
    QString infoString;

    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        //simplify RTL case -> otherwise lots of RTL,LTR and neutral char mixes 
        //(depends on modem strings)
        infoString += Qt::escape(manufacturer) + "<br>"
                  + Qt::escape(model) + "<br>"
                  + Qt::escape(revision) + "<br>"
                  + Qt::escape(serial) + "<br>";
    } else {
        infoString += tr("Manufacturer:") +
                      ' ' + Qt::escape( manufacturer ) +
                      "<br/>";

        infoString += tr("Model:") +
                      ' ' + Qt::escape( model ) +
                      "<br/>";

        infoString += tr("Revision:") +
                      ' ' + Qt::escape( revision ) +
                      "<br/>";

        infoString += tr("Serial Number:") +
                      ' ' + Qt::escape( serial ) +
                      "<br/>";
    }

    if ( !extraVersion.isEmpty() ) {
        infoString += Qt::escape( extraVersion ).replace( "\n", "<br/>" );
    }

    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        infoString.prepend("<p align=\"right\">");
        infoString.append("</p>");
    }
    return infoString;
}
