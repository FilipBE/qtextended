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

#include "phonenetworks.h"
#include "modemnetwork.h"
#include "voipnetwork.h"

#include <qtopiaapplication.h>
#include <qcommservicemanager.h>
#include <qnetworkregistration.h>
#include <QtopiaItemDelegate>

#include <QVBoxLayout>

PhoneNetworks::PhoneNetworks( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();
}

void PhoneNetworks::init()
{
    setObjectName( "phonenetworks" );
    setWindowTitle( tr( "Call Networks" ) );
    showMaximized();
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing( 0 );

    QCommServiceManager manager;
    QStringList list = manager.supports<QNetworkRegistration>();
    list.sort();

    // if there are multiple networks, create a list widget.
    if ( list.count() > 1 ) {
        m_list = new QListWidget( this );
        m_list->setItemDelegate( new QtopiaItemDelegate );
        m_list->setFrameStyle(QFrame::NoFrame);
        layout->addWidget( m_list );

        QStringList::const_iterator i;
        for ( i = list.constBegin(); i != list.constEnd(); ++i ) {
            QNetworkRegistration nr( (*i), this );
            QString str = nr.currentOperatorTechnology();
            if ( str.isEmpty() && (*i) == "voip" )
                str = tr( "VoIP" );
            if ( str.isEmpty() )
                str = (*i);
            QListWidgetItem *item = new QListWidgetItem( str, m_list );
            item->setData( Qt::UserRole, QVariant( (*i) ) );
        }
        m_list->setCurrentRow( 0 );
        connect( m_list, SIGNAL(itemActivated(QListWidgetItem*)),
                this, SLOT(serviceSelected(QListWidgetItem*)) );
    } else if ( list.count() == 1 ) {
        QWidget *widget = 0;
        if ( list.at( 0 ) == "modem"  )
            widget = new ModemNetworkRegister( this );
        else if ( list.at( 0 ) == "voip" )
            widget = new VoipNetworkRegister( this );

        // set object name to refer to the correct help doc
        setObjectName( objectName() + "-" + list.at( 0 ) );

        if ( widget )
            layout->addWidget( widget );
    }
}

void PhoneNetworks::serviceSelected( QListWidgetItem *item )
{
    QString str = item->data( Qt::UserRole ).toString();

    QDialog *dlg = new QDialog( this );
    // set object name to refer to the correct help doc
    dlg->setObjectName( str );

    QVBoxLayout *layout = new QVBoxLayout( dlg );
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget *widget = 0;

    if ( str == "modem" ) {
        widget = new ModemNetworkRegister( this );
        dlg->setWindowTitle( tr( "Call Networks" ) );
    } else if ( str == "voip" ) {
        widget = new VoipNetworkRegister( this );
        dlg->setWindowTitle( tr( "VoIP Network" ) );
    }

    if ( widget ) {
        layout->addWidget( widget );
        dlg->showMaximized();
        QtopiaApplication::execDialog( dlg, false );
    }
}

