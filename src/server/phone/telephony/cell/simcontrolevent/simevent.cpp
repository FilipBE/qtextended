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

#include "simevent.h"
#include "qabstractmessagebox.h"
#include "qtopiaserverapplication.h"

#include <QCommServiceManager>
#include <QSimControlEvent>
#include <QSimToolkit>
#include <QSoftMenuBar>

/*!
  \class SimControlEventHandlerTask
    \inpublicgroup QtCellModule
  \brief The SimControlEventHandlerTask class provides feedback about SIM control 
  events to the user.
  \ingroup QtopiaServer::Telephony

  When the user initiates a phone call or sends SMS messages on a SIM-equipped 
  mobile phone, the SIM can make modifications to the request before it is sent
  to the network, or deny the request outright. This process is described in section 9 of 3GPP TS 11.14.

  \sa QSimToolkit, QSimControlEvent
  */

/*!
  Creates a new SimControlEventHandlerTask instance with the given \a parent.
*/
SimControlEventHandlerTask::SimControlEventHandlerTask( QObject *parent )
    : QObject( parent ), serviceManager(0)
{
    serviceManager = new QCommServiceManager( this );
    connect( serviceManager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );
    servicesChanged();
}

void SimControlEventHandlerTask::servicesChanged()
{
    if ( !serviceManager )
        return;

    if ( serviceManager->supports<QSimToolkit>().count() > 0 ) {
        simToolkit = new QSimToolkit( QString(), this ); //we don't care which service
        QObject::connect( simToolkit, SIGNAL(controlEvent(QSimControlEvent)),
                this, SLOT(simControlEvent(QSimControlEvent)) );
        serviceManager->disconnect();
        serviceManager->deleteLater();
        serviceManager = 0;
    }
}

void SimControlEventHandlerTask::simControlEvent( const QSimControlEvent& e )
{
    if ( !e.text().isEmpty() ) {
        QString title;
        switch ( e.result() ) {
            case QSimControlEvent::Allowed: title = tr( "Allowed" ); break;
            case QSimControlEvent::NotAllowed: title = tr( "Not Allowed" ); break;
            case QSimControlEvent::AllowedWithModifications: title = tr( "Number Modified" ); break;
        }

        static QAbstractMessageBox *simMsgBox = 0;
        if ( !simMsgBox ) {
            simMsgBox = QAbstractMessageBox::messageBox
                ( 0, title, e.text(), QAbstractMessageBox::Information );
            QSoftMenuBar::removeMenuFrom( simMsgBox, QSoftMenuBar::menuFor( simMsgBox ) );
        } else {
            simMsgBox->setTitle( title );
            simMsgBox->setText( e.text() );
        }
        if ( e.result() == QSimControlEvent::NotAllowed )
            simMsgBox->setTimeout(3000, QAbstractMessageBox::NoButton);
        QtopiaApplication::execDialog(simMsgBox);
    }
}

QTOPIA_TASK(SimControlEventHandlerTask, SimControlEventHandlerTask);

