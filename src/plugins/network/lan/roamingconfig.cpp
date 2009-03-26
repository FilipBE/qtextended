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

#include "roamingconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QKeyEvent>
#include <QMultiHash>
#include "roamingmonitor.h" //includes wireless.h

RoamingPage::RoamingPage( const QtopiaNetworkProperties& cfg, QWidget* parent, Qt::WFlags fl )
    : QWidget( parent, fl ), currentSelection( 0 )
{
#if WIRELESS_EXT > 13
    ui.setupUi( this );
    init ( cfg );

    connect( ui.autoConnect, SIGNAL(stateChanged(int)), this, SLOT(reconnectToggled(int)) );
    connect( ui.knownNetworks, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(listActivated(QListWidgetItem*)) );
    ui.knownNetworks->installEventFilter(this);
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT

}

RoamingPage::~RoamingPage()
{
}

QtopiaNetworkProperties RoamingPage::properties()
{
    QtopiaNetworkProperties results;
#if WIRELESS_EXT > 13
    saveConfig();

    results.insert( "WirelessNetworks/size", props.take( QString("size") ) );
    results.insert( "WirelessNetworks/Timeout", props.take( QString("Timeout") ) );
    results.insert( "WirelessNetworks/AutoConnect", props.take( QString("AutoConnect") ) );

    QHash<int,int> oldToNew;
    int old;
    QList<QVariant> essidList = props.values("ESSID");
    for(int i = 0; i< ui.knownNetworks->count(); ++i) {
        old = essidList.indexOf( ui.knownNetworks->item( i )->text() );
        oldToNew.insert( old, i+1 );
    }

    const QList<QString> keys = props.keys();
    const QString prefix( "WirelessNetworks/%1/" );
    foreach( QString k , keys ) {
        QList<QVariant> entries = props.values( k );
        for( int i=0; i<entries.count(); ++i)
            results.insert( prefix.arg(oldToNew[i])+k, entries[i]);
    }
#endif // WIRELESS_EXT
    return results;
}

void RoamingPage::setProperties( const QtopiaNetworkProperties& cfg )
{
#if WIRELESS_EXT > 13
    init( cfg );
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT
}

void RoamingPage::init( const QtopiaNetworkProperties& cfg )
{
#if WIRELESS_EXT > 13
    props.clear();
    ui.knownNetworks->clear();

    const QList<QString> keys = cfg.keys();
    QString normalizedKey;
    foreach( QString k, keys ) {
        normalizedKey = k.mid(k.lastIndexOf(QChar('/'))+1);
        props.insert( normalizedKey, cfg.value( k ) );
        if ( normalizedKey == QLatin1String("ESSID") ) {
            QListWidgetItem* item = new QListWidgetItem( cfg.value( k ).toString(), ui.knownNetworks );
            Q_UNUSED(item)
        }
    }
    readConfig();
#else
    Q_UNUSED( cfg )
#endif // WIRELESS_EXT
}

void RoamingPage::readConfig()
{
#if WIRELESS_EXT > 13
    const int tout = props.value("Timeout", 10).toInt();
    ui.timeout->setValue( tout );
    ui.autoConnect->setChecked( props.value("AutoConnect", false).toBool() );
    reconnectToggled( ui.autoConnect->checkState() );
#endif // WIRELESS_EXT
}

void RoamingPage::saveConfig()
{
#if WIRELESS_EXT > 13
    props.replace( "Timeout", ui.timeout->value() );
    props.replace( "AutoConnect", ui.autoConnect->isChecked() );
#endif // WIRELESS_EXT
}

void RoamingPage::reconnectToggled( int newState )
{
#if WIRELESS_EXT > 13
    ui.timeout->setEnabled( newState == Qt::Checked );
    ui.timeoutLabel->setEnabled( newState == Qt::Checked );
#else
    Q_UNUSED( newState )
#endif // WIRELESS_EXT
}

void RoamingPage::listActivated(QListWidgetItem* item)
{
#if WIRELESS_EXT > 13
    if ( !item )
        return;
    if ( !currentSelection ) {
        ui.header->setText( tr("Moving %1", "%1=essid").arg(item->text()) );
        QFont f = item->font();
        f.setBold( true );
        item->setFont( f );
        item->setText( item->text() );
        currentSelection = item;
    }else if ( currentSelection ) {
        ui.header->setText( tr("Order of selection") );
        QFont f = currentSelection->font();
        f.setBold( false );
        currentSelection->setFont( f );
        if ( item != currentSelection) {
            int oldRow = ui.knownNetworks->row(currentSelection);
            int newRow = ui.knownNetworks->row(item);
            if (oldRow>newRow) {
                ui.knownNetworks->takeItem(oldRow);
                ui.knownNetworks->insertItem(newRow+1, currentSelection);
                ui.knownNetworks->takeItem(newRow);
                ui.knownNetworks->insertItem(oldRow, item);
            } else {
                ui.knownNetworks->takeItem(oldRow);
                ui.knownNetworks->insertItem(newRow, currentSelection);
                ui.knownNetworks->takeItem(newRow-1);
                ui.knownNetworks->insertItem(oldRow, item);
            }
            ui.knownNetworks->setCurrentRow(newRow);
        }
        currentSelection = 0;
    }
#else
    Q_UNUSED( item );
#endif //WIRELESS_EXT
}

bool RoamingPage::eventFilter( QObject* watched, QEvent* event )
{
#if WIRELESS_EXT > 13
    if ( watched == ui.knownNetworks &&
            0 != currentSelection )
    {
        if ( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease ) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);

            if ( event->type() == QEvent::KeyRelease &&  //ignore releases if key is one we watch out for
                    (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key()==Qt::Key_Back) )
                return true;

            int row = ui.knownNetworks->currentRow();
            if ( ke->key() == Qt::Key_Up ) {
                if ( row > 0 ) //top row cannot move further up
                {
                    ui.knownNetworks->takeItem( row );
                    ui.knownNetworks->insertItem( row-1, currentSelection );
                    ui.knownNetworks->setCurrentRow( row-1 );
                }
                return true;
            } else if ( ke->key() == Qt::Key_Down ) {
                if ( row < ui.knownNetworks->count()-1 ) { //bottom row cannot move further down
                    ui.knownNetworks->takeItem( row );
                    ui.knownNetworks->insertItem( row+1, currentSelection );
                    ui.knownNetworks->setCurrentRow( row+1 );
                }
                return true;
            } else if ( ke->key() == Qt::Key_Back ) {
                return true; //ignore back for as long as we have a selection
            }
        }
    }
#else
    Q_UNUSED(watched);
    Q_UNUSED(event);
#endif
    return false;
}
#endif // NO_WIRELESS_LAN
