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

#include "wirelessconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QSet>

#include <qtopiaapplication.h>
#include <hexkeyvalidator.h>
#include <qsoftmenubar.h>

WirelessPage::WirelessPage( const QtopiaNetworkProperties& prop, QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    ui.setupUi( this );
    init();
    initNetSelector( prop );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
}

WirelessPage::~WirelessPage()
{
}

void WirelessPage::init()
{
    ui.newButton->setIcon( QIcon(":icon/new") );
    ui.delButton->setIcon( QIcon(":icon/trash") );

    HexKeyValidator* macValidator = new HexKeyValidator( this, 12 );
    ui.ap->setValidator( macValidator );
    QtopiaApplication::setInputMethodHint( ui.ap, QtopiaApplication::Text );

    connect( ui.mode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChannelMode(int)) );
    connect( ui.essid, SIGNAL(textChanged(QString)), this, SLOT(setNewNetworkTitle(QString)) );
    connect( ui.newButton, SIGNAL(clicked()), this, SLOT(addWLAN()) );
    connect( ui.delButton, SIGNAL(clicked()), this, SLOT(removeWLAN()) );
}

void WirelessPage::setNewNetworkTitle( const QString& name )
{
    const int index = ui.netSelector->currentIndex();
    if ( index<0 || index >= ui.netSelector->count() )
        return;

    if ( !name.isEmpty() )
        ui.netSelector->setItemText( index, name );
    else
        ui.netSelector->setItemText( index, tr("<New Network>") );
}

void WirelessPage::initNetSelector( const QtopiaNetworkProperties& prop )
{
    changedSettings.clear();
    ui.netSelector->clear();
    const QList<QString> keys = prop.keys();
    const int numKnownNetworks = prop.value( QLatin1String("WirelessNetworks/size"), 0).toInt();

    if ( numKnownNetworks == 0 ) {
        ui.advanced_frame->setEnabled( false );
        ui.netSelector->setEnabled( false );
        ui.newButton->setFocus();
    }
    disconnect( ui.netSelector, 0, this, 0 );
    foreach( QString key, keys ) {
        if ( !key.startsWith( "WirelessNetworks" ) )
            continue;
        int idx = key.mid(17 /*strlen("WirelessNetworks/")*/, key.indexOf(QChar('/'), 17)-17).toInt();
        if ( idx <= numKnownNetworks ) {
            //copy all values into changedSettings where we keep track of changes
            changedSettings.insert( key, prop.value( key ) );
            if ( key.endsWith( "ESSID" ) ) {
                QString text = prop.value( key ).toString();
                if ( text.isEmpty() )
                    text = tr("Unnamed network");
                ui.netSelector->addItem( text );
            }
        }
    }
    lastIndex = 0;
    ui.netSelector->setCurrentIndex( 0 );
    readConfig();
    connect( ui.netSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(newNetSelected(int)) );
}

void WirelessPage::newNetSelected( int index )
{
    if ( index < 0 || index >= ui.netSelector->count() )
        return;
    saveConfig( );
    //lastIndex contains the index of the previously unselected entry
    //after calling saveConfig we have to update lastIndex
    lastIndex = index;
    readConfig( );
}

/*!
  \internal

  Loads details of new networks into the UI elements.
  */
void WirelessPage::readConfig( )
{
    const int num = ui.netSelector->currentIndex() + 1;
    const QString s = QString("WirelessNetworks/%1/").arg(num);

    QString mode = changedSettings.value(s+"WirelessMode").toString();
    ui.channel->setValue( changedSettings.value(s+"CHANNEL").toInt() );
    if ( mode == "Ad-hoc" ) {
        ui.mode->setCurrentIndex( 0 );
    } else if ( mode == "Master" ) {
        ui.mode->setCurrentIndex( 2 );
    } else {
        ui.mode->setCurrentIndex( 1 ); //Managed
        ui.channel->setEnabled( false );
        ui.channel->setValue( 0 );
    }

    ui.essid->setText( changedSettings.value(s+"ESSID").toString() );
    ui.ap->setText( changedSettings.value(s+"AccessPoint").toString() );
    ui.nickname->setText( changedSettings.value(s+"Nickname").toString() );
    int index = ui.bitrate->findText( changedSettings.value(s+"BitRate").toString() );
    if ( index <= 0 )
        ui.bitrate->setCurrentIndex( 0 ); //automatic bitrate detection
    else
        ui.bitrate->setCurrentIndex( index );
}

QtopiaNetworkProperties WirelessPage::properties()
{
    saveConfig();
    //has a new entry been added?
    const int size = ui.netSelector->count();
    QList<QString> allKeys = changedSettings.keys();

    QtopiaNetworkProperties result;
    //delete all networks with empty essid
    int newSize = 0;
    for ( int i = 1; i <= size; i++ ) {
        QString entryKey = "WirelessNetworks/"+QString::number( i );
        QString essid = changedSettings.value(entryKey+"/ESSID").toString();
        if ( !essid.isEmpty() ) {
            newSize++;
            QString newEntryKey = "WirelessNetworks/"+QString::number( newSize );
            foreach( QString k, allKeys )
                if ( k.startsWith( entryKey ) ) {
                    QString key = k.mid(k.lastIndexOf( QChar('/') )+1 );
                    result.insert( newEntryKey+QChar('/')+key, changedSettings.value(k).toString());
                }
        }
    }

    //add size attribute for QSettings::beginReadArray()
    result.insert("WirelessNetworks/size", newSize );

    //add remaining keys which are not part of a particular net
    result.insert("WirelessNetworks/Timeout", changedSettings.value("WirelessNetworks/Timeout", 0));
    result.insert("WirelessNetworks/AutoConnect", changedSettings.value("WirelessNetworks/AutoConnect", false));
    return result;
}


/*!
  \internal

  Saves the selected network options.
  */
void WirelessPage::saveConfig()
{
    if ( lastIndex < 0 || lastIndex >= ui.netSelector->count() )
        return;

    const QString s = QString("WirelessNetworks/%1/").arg(lastIndex+1);

    switch( ui.mode->currentIndex() ) {
        case 0:
            changedSettings.insert(s+"WirelessMode", "Ad-hoc");
            break;
        case 2:
            changedSettings.insert(s+"WirelessMode", "Master");
            break;
        default:
            changedSettings.insert(s+"WirelessMode", "Managed");
    }

    changedSettings.insert(s+"ESSID", ui.essid->text());
    changedSettings.insert(s+"AccessPoint", ui.ap->text());
    changedSettings.insert(s+"Nickname", ui.nickname->text());
    changedSettings.insert(s+"CHANNEL", ui.channel->value());

    if ( ui.bitrate->currentIndex() == 0 )
        changedSettings.insert(s+"BitRate", "0");
    else
        changedSettings.insert(s+"BitRate", ui.bitrate->currentText());

    //save create/save connection Uuid
    if ( !changedSettings.contains( s+"Uuid" ) ) {
        QUuid uid = QUuid::createUuid();
        changedSettings.insert( s+"Uuid", uid.toString() );
    }
}



void WirelessPage::changeChannelMode( int index )
{
    if ( index >= 0 ) {
        ui.channel->setEnabled( index != 1 );//Managed mode ->no channel selection
        if ( index == 1 )
            ui.channel->setValue( 0 );
    }
}

void WirelessPage::setProperties( const QtopiaNetworkProperties& cfg )
{
    initNetSelector( cfg );
}

void WirelessPage::removeWLAN()
{
    const int oldCount = ui.netSelector->count();
    const int idx = ui.netSelector->currentIndex()+1;
    const QString badKey = QString("WirelessNetworks/%1/").arg( idx );
    const QtopiaNetworkProperties save = properties();
    QtopiaNetworkProperties newProps;
    const QList<QString> keys = save.keys();
    foreach ( QString key, keys ) {
        const int index = key.mid(17, key.indexOf(QChar('/'), 17)-17).toInt();
        if ( index < idx )
            newProps.insert( key, save.value( key ) );
        else if ( index == idx )
            continue;
        else
            newProps.insert( QString("WirelessNetworks/%1").arg(index-1)+key.mid(key.lastIndexOf(QChar('/'))), save.value( key ) );

    }
    newProps.insert("WirelessNetworks/size", oldCount-1);
    initNetSelector( newProps );
}

void WirelessPage::addWLAN()
{
    ui.netSelector->addItem( tr("<New Network>") );
    ui.netSelector->setCurrentIndex( ui.netSelector->count()-1 );
    ui.advanced_frame->setEnabled( true );
    ui.netSelector->setEnabled( true );
    ui.netSelector->setFocus();
}
#endif
