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

#include "advanced.h"

#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

AdvancedPage::AdvancedPage( const QtopiaNetworkProperties& prop, QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags), GPRS( false )
{
    ui.setupUi( this );
    init();
    readConfig( prop );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );
}

AdvancedPage::~AdvancedPage()
{
}

void AdvancedPage::init()
{
    QtopiaApplication::setInputMethodHint( ui.device, QtopiaApplication::Text );

    connect( ui.external_box, SIGNAL(stateChanged(int)),
            this, SLOT(externalState(int)) );
    ui.device->installEventFilter( this );
}

void AdvancedPage::selectCombo( QComboBox* combo, const QString& item )
{
    for (int i=0; i<combo->count(); ++i) {
        if ( combo->itemText(i) == item ) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    combo->addItem(item);
    combo->setCurrentIndex(combo->count()-1);
}

void AdvancedPage::readConfig( const QtopiaNetworkProperties& prop )
{
    if ( prop.value("Serial/DefaultRoute").toString() != "n" )
        ui.routing->setCheckState( Qt::Checked );
    else
        ui.routing->setCheckState( Qt::Unchecked );

#ifdef QTOPIA_CELL
    if ( prop.value("Serial/Type").toString() != "external" )
        ui.external_box->setCheckState( Qt::Unchecked );
    else
        ui.external_box->setCheckState( Qt::Checked );
    ui.external_box->setEnabled( true );
#else
    ui.external_box->setCheckState( Qt::Checked );
    ui.external_box->setEnabled( false );
#endif
    externalState( ui.external_box->checkState() );

    const QString device = prop.value("Serial/SerialDevice").toString();
    if ( device.isEmpty() )
        ui.device->setText( tr("<Use PCMCIA>") );
    else
        ui.device->setText(prop.value("Serial/SerialDevice").toString());

    QString atstring = prop.value("Serial/ATDial").toString();
    if (atstring.isNull())
        ui.atdial->setCurrentIndex( 0 );
    else
        selectCombo( ui.atdial, atstring );

    QString speed = prop.value("Serial/Speed").toString() ;
    if (speed.isEmpty())
        ui.speed->setCurrentIndex( ui.speed->count() - 1 );
    else
        selectCombo( ui.speed, speed );

    if ( prop.value("Serial/Crtscts").toString() == "n" )
        ui.flow->setCheckState( Qt::Unchecked );
    else
        ui.flow->setCheckState( Qt::Checked );

    int delay = 5;
    if (prop.value("Serial/ConnectDelay").isValid())
        delay = prop.value("Serial/ConnectDelay").toInt();
    ui.connectDelay->setValue( delay );

    if (prop.value("Serial/GPRS").toString() != "y") {
        setObjectName( "dialup-advanced" );
        if ( prop.value("Serial/SilentDial").toString() == "n" )
            ui.silence->setCheckState( Qt::Unchecked );
        else
            ui.silence->setCheckState( Qt::Checked );
    } else {
        setObjectName( "gprs-advanced" );
        GPRS = true;
        ui.silence->hide();
    }

}

QtopiaNetworkProperties AdvancedPage::properties()
{
    QtopiaNetworkProperties prop;

    if ( ui.routing->checkState() == Qt::Checked )
        prop.insert( "Serial/DefaultRoute", QString("y") );
    else
        prop.insert( "Serial/DefaultRoute", QString("n") );

    QString devType = "internal"; //no tr
    if ( ui.external_box->checkState() == Qt::Checked )
        devType = "external"; //no tr
    prop.insert( "Serial/Type",  devType );
    if ( ui.device->text() != tr("<Use PCMCIA>") )
        prop.insert( "Serial/SerialDevice", ui.device->text() );

    prop.insert( "Serial/ATDial", ui.atdial->currentText() );
    prop.insert( "Serial/Speed", ui.speed->currentText() );

    if ( ui.flow->checkState() == Qt::Checked )
        prop.insert( "Serial/Crtscts", QString("y") );
    else
        prop.insert( "Serial/Crtscts", QString("n") );

    prop.insert( "Serial/ConnectDelay", ui.connectDelay->value() );

    if ( !GPRS && ui.silence->checkState() == Qt::Checked )
        prop.insert("Serial/SilentDial", QString("y") );
    else
        prop.insert("Serial/SilentDial", QString("n") );

    return prop;
}

void AdvancedPage::externalState( int state )
{
    ui.speed->setEnabled( state == Qt::Checked );
    ui.device->setEnabled( state == Qt::Checked );
    ui.atdial->setEnabled( state == Qt::Checked );
}

bool AdvancedPage::eventFilter( QObject* receiver, QEvent* event )
{
    if ( receiver == ui.device ) {
        if ( event->type() == QEvent::FocusIn && ui.device->text() == tr("<Use PCMCIA>") ) {
            ui.device->clear();
        } else if ( event->type() == QEvent::FocusOut && ui.device->text().isEmpty() ) {
            ui.device->setText( tr("<Use PCMCIA>") );
        }
    }
    return false;
}
