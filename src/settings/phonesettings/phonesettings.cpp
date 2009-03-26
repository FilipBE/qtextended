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
#include "phonesettings.h"

#include <qtopiaapplication.h>
#include <qpassworddialog.h>
#include <qsoftmenubar.h>
#include <QSimInfo>
#include <qcategoryselector.h>
#include <qcontactview.h>
#include <qcbsmessage.h>
#include <qpinmanager.h>
#include <qtopiaipcenvelope.h>

#include <QCallVolume>

#include <QMessageBox>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QKeyEvent>
#include <QWaitWidget>
#include <QtopiaItemDelegate>
#include <QScrollArea>

const QString SERVICE_NAME = "modem";

int FloatingTextList::lastCharIndex = 0;

FloatingTextList::FloatingTextList( QWidget *parent, int w )
: QListWidget( parent ), availableWidth( w )
{
    timer = new QTimer( this );

    connect( this, SIGNAL(currentRowChanged(int)), this, SLOT(newCurrentRow(int)) );
    connect( timer, SIGNAL(timeout()), this, SLOT(floatText()) );

    setItemDelegate(new QtopiaItemDelegate);
    setFrameStyle(NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void FloatingTextList::showItemText()
{
    if ( timer->isActive() )
        timer->stop();

    QFontMetrics fm(font());
    QListWidgetItem *current;

    for ( int i = 0; i < count(); i++ ) {
        current = item( i );
        QString text = current->data( Qt::WhatsThisRole ).toString();
        int currentTextWidth = fm.width( text );
        if ( availableWidth < currentTextWidth ) {
            while ( availableWidth < currentTextWidth ) {
                text = text.left( text.length() - 1 );
                currentTextWidth = fm.width( text );
            }
        }
        current->setData( Qt::UserRole + 1, text.length() );
        current->setText( text );
    }
}

void FloatingTextList::newCurrentRow( int row )
{
    if ( timer->isActive() )
        timer->stop();

    if ( row < 0 )
        return;

    showItemText();

    QListWidgetItem *current = item( row );
    int complete = current->data( Qt::WhatsThisRole ).toString().length();
    int shown = current->data( Qt::UserRole + 1 ).toInt();
    if ( complete > shown )
        timer->singleShot( 1200, this, SLOT(floatText()) );
}

void FloatingTextList::floatText()
{
    QListWidgetItem *item = currentItem();
    if ( !item )
        return;
    QString completeText = item->data( Qt::WhatsThisRole ).toString();
    int allowedLength = item->data( Qt::UserRole + 1 ).toInt();

    if ( lastCharIndex == completeText.length() ) { // reached to the end
        lastCharIndex = allowedLength - 1;
        newCurrentRow( currentRow() );
        return;
    }

    QString shownText = item->text();
    QFontMetrics fm(font());

    if ( !timer->isActive() ) {
        lastCharIndex = allowedLength - 1;
        timer->start( 500 );
    }

    shownText = completeText.mid( lastCharIndex - allowedLength + 1, allowedLength );
    //check if the new string would fit the screen
    int currentTextWidth = fm.width( shownText );
    if ( availableWidth < currentTextWidth ) {
        while ( availableWidth < currentTextWidth ) { // if it doesn't fit
            shownText = shownText.mid( 1 ); // remove the first character
            currentTextWidth = fm.width( shownText );
        }
    }
    lastCharIndex++;
    item->setText( shownText );
    if ( lastCharIndex == completeText.length() ) {
        timer->stop();
        timer->singleShot( 800, this, SLOT(floatText()) );
    }
}

void FloatingTextList::keyPressEvent( QKeyEvent *e )
{
    if ( !hasEditFocus() ) {
        QListWidget::keyPressEvent( e );
        return;
    }

    int curRow = currentRow();
    if ( e->key() == Qt::Key_Up ) {
        if ( curRow == 0 )
            setCurrentRow( count() - 1 );
        else
            setCurrentRow( curRow - 1 );
    } else if ( e->key() == Qt::Key_Down ) {
        if ( curRow == count() - 1 )
            setCurrentRow( 0 );
        else
           setCurrentRow( curRow + 1 );
    } else if ( e->key() == Qt::Key_Back ) {
        setEditFocus( false );
    } else {
        QListWidget::keyPressEvent( e );
    }
}

//----------------------------------------------------------------------------

PhoneSettings::PhoneSettings( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), selectVoiceMail( false )
{
    //we are using libqtopiapim and hence need to load the translations
    QtopiaApplication::loadTranslations( "libqtopiapim" );
    init();

    connect( optionList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated(QListWidgetItem*)) );

    new VoiceMailService( this );
}

void PhoneSettings::init()
{
    setWindowTitle( tr( "Call Options" ) );

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setContentsMargins(0, 0, 0, 0);
    optionList = new QListWidget( this );
    optionList->setItemDelegate(new QtopiaItemDelegate);
    optionList->setSpacing(1);
    optionList->setFrameStyle(QFrame::NoFrame);
    vb->addWidget( optionList );
    QListWidgetItem *item;

    item = new QListWidgetItem( tr( "Call Barring" ), optionList);
    item->setIcon( QIcon( ":icon/phone/callbarring" ) );
    item->setData( Qt::UserRole, Barring );

    item = new QListWidgetItem( tr( "Call Waiting" ), optionList);
    item->setIcon( QIcon( ":icon/phone/callwaiting" ) );
    item->setData( Qt::UserRole, Waiting );

    item = new QListWidgetItem( tr( "Caller ID" ), optionList);
    item->setIcon( QIcon( ":icon/phone/callerid" ) );
    item->setData( Qt::UserRole, CallerId );

    item = new QListWidgetItem( tr( "Cell Broadcasting" ), optionList);
    item->setIcon( QIcon( ":icon/phone/channels" ) );
    item->setData( Qt::UserRole, Broadcast );

    item = new QListWidgetItem( tr( "Fixed Dialing" ), optionList);
    item->setIcon( QIcon( ":icon/phone/fixeddialing" ) );
    item->setData( Qt::UserRole, Fixed );

    if ( Qtopia::hasKey( Qt::Key_Flip ) ) {
        item = new QListWidgetItem( tr( "Flip Function" ), optionList);
        item->setIcon( QIcon( ":icon/phone/flip" ) );
        item->setData( Qt::UserRole, Flip );
    }

    item = new QListWidgetItem( tr( "Service Numbers" ), optionList);
    item->setIcon( QIcon( ":icon/phone/services" ) );
    item->setData( Qt::UserRole, Service );

    item = new QListWidgetItem( tr( "Call Volume" ), optionList);
    item->setIcon( QIcon( ":icon/sound" ) );
    item->setData( Qt::UserRole, Volume );

    QSoftMenuBar::menuFor( this );
    optionList->setCurrentRow( 0 );
}

void PhoneSettings::activate( int itemId )
{
    for ( int index = 0; index < optionList->count(); ++index ) {
        if ( optionList->item(index)->data( Qt::UserRole ).toInt() == itemId ) {
            itemActivated( optionList->item(index) );
            return;
        }
    }
}

void PhoneSettings::itemActivated( QListWidgetItem *item )
{
    // check is SIM card is ready
    QSimInfo simInfo( SERVICE_NAME, this );
    if ( simInfo.identity().isEmpty() ) {
        QMessageBox::warning( this, tr( "SIM Card Error" ), tr( "SIM card is not inserted or currently not ready." ) );
        return;
    }

    QDialog *dlg = 0;
    int index = item->data( Qt::UserRole ).toInt();
    if ( index == Barring ) {
        dlg = new CallBarring( this );
        dlg->showMaximized();
    } else if ( index == Waiting )
        dlg = new CallWaiting( this );
    else if ( index == CallerId )
        dlg = new CallerID( this );
    else if ( index == Broadcast ) {
        dlg = new CellBroadcasting( this );
        dlg->showMaximized();
    } else if ( index == Fixed )
        dlg = new FixedDialing( this );
    else if ( index == Flip )
        dlg = new FlipFunction( this );
    else if ( index == Service ) {
        ServiceNumbers *sdlg = new ServiceNumbers( this );
        if ( selectVoiceMail ) {
            sdlg->selectVoiceMail();
            selectVoiceMail = false;
        }
        dlg = sdlg;
    } else if ( index == Volume )
        dlg = new CallVolume( this );

    if ( dlg ) {
        dlg->setModal( true );
        QtopiaApplication::execDialog( dlg );
        delete dlg;
    }
}

//----------------------------------------------------------------------------

CallBarring::CallBarring( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), isLoading( true )
{
    init();

    connect( barOptions, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated(QListWidgetItem*)) );
    connect( client, SIGNAL(barringStatus(QCallBarring::BarringType,
                    QTelephony::CallClass)),
            this, SLOT(barringStatus(QCallBarring::BarringType,
                    QTelephony::CallClass)) );
    connect( client, SIGNAL(setBarringStatusResult(QTelephony::Result)),
            this, SLOT(setBarringStatusResult(QTelephony::Result)) );
    connect( client, SIGNAL(changeBarringPasswordResult(QTelephony::Result)),
            this, SLOT(changeBarringPasswordResult(QTelephony::Result)) );
    connect( client, SIGNAL(unlockResult(QTelephony::Result)),
            this, SLOT(unlockResult(QTelephony::Result)) );
}

void CallBarring::init()
{
    setObjectName( "barring" );
    setWindowTitle( tr( "Call Barring" ) );

    waitWidget = new QWaitWidget( this );
    waitWidget->setCancelEnabled( true );
    connect( waitWidget, SIGNAL(cancelled()), this, SLOT(reject()) );
    waitWidget->show();

    QVBoxLayout *vb = new QVBoxLayout(this);

    incoming = QIcon( ":icon/phone/incomingcall" );
    outgoing = QIcon( ":icon/phone/outgoingcall" );
    barred = QIcon( ":icon/reset" );
    int is = style()->pixelMetric(QStyle::PM_ListViewIconSize);
    QSize iconSize = incoming.actualSize( QSize( is, is ) );

    QDesktopWidget *desktop = QApplication::desktop();
    int listWidth = desktop->availableGeometry(desktop->screenNumber(this)).width()
                    - ( iconSize.width() * 3/2 );

    barOptions = new FloatingTextList( this, listWidth );
    barOptions->setEnabled( false );
    vb->addWidget( barOptions );
    vb->setMargin(0);
    QListWidgetItem *item;

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "All outgoing calls" ) );
    item->setData( Qt::UserRole, QCallBarring::OutgoingAll );
    item->setIcon( outgoing );

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "International outgoing calls" ) );
    item->setData( Qt::UserRole, QCallBarring::OutgoingInternational );
    item->setIcon( outgoing );

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "International calls except home" ) );
    item->setData( Qt::UserRole, QCallBarring::OutgoingInternationalExceptHome );
    item->setIcon( outgoing );

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "All incoming calls" ) );
    item->setData( Qt::UserRole, QCallBarring::IncomingAll );
    item->setIcon( incoming );

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "All incoming calls not stored in SIM" ) );
    item->setData( Qt::UserRole, QCallBarring::IncomingNonSIM );
    item->setIcon( incoming );

    item = new QListWidgetItem( QString(), barOptions );
    item->setData( Qt::WhatsThisRole, tr( "Incoming calls when roaming" ) );
    item->setData( Qt::UserRole, QCallBarring::IncomingWhenRoaming );
    item->setIcon( incoming );

    barOptions->setEnabled( false );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    unlock = contextMenu->addAction( tr( "Deactivate all" ), this, SLOT(unlockAll()) );
    pin = contextMenu->addAction( tr( "Change PIN2" ), this, SLOT(changePin()) );
    updateMenu();

    client = new QCallBarring( SERVICE_NAME, this );
}

void CallBarring::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    barOptions->showItemText();
    if ( isLoading )
        QTimer::singleShot( 200, this, SLOT(checkStatus()) );
}

void CallBarring::checkStatus()
{
    client->requestBarringStatus( QCallBarring::OutgoingAll );
    client->requestBarringStatus( QCallBarring::OutgoingInternational );
    client->requestBarringStatus( QCallBarring::OutgoingInternationalExceptHome );
    client->requestBarringStatus( QCallBarring::IncomingAll );
    client->requestBarringStatus( QCallBarring::IncomingNonSIM );
    client->requestBarringStatus( QCallBarring::IncomingWhenRoaming );
}

void CallBarring::barringStatus( QCallBarring::BarringType type, QTelephony::CallClass c )
{
    bool enable = c != QTelephony::CallClassNone;

    QListWidgetItem *item;
    for ( int i = 0; i < barOptions->count(); i++ ) {
        item = barOptions->item( i );
        QCallBarring::BarringType t = (QCallBarring::BarringType)item->data( Qt::UserRole ).toInt();
        if ( t == type ) {
            if ( enable ) {
                item->setIcon( barred );
            } else {
                if ( t < QCallBarring::IncomingAll )
                    item->setIcon( outgoing );
                else
                    item->setIcon( incoming );
            }
            if ( !isLoading ) {
                barOptions->setEnabled( true );
                barOptions->setFocus();
            }
            break;
        }
    }
    if ( isLoading && type == QCallBarring::IncomingWhenRoaming ) {
        barOptions->setEnabled( true );
        barOptions->setFocus();
        waitWidget->hide();
        isLoading = false;
        barOptions->setCurrentRow( 0 );
    }

    if ( !isLoading )
        waitWidget->hide();

    updateMenu();
}

void CallBarring::itemActivated( QListWidgetItem * item )
{
    bool enable = false;
    if ( item->icon().serialNumber() == incoming.serialNumber()
        || item->icon().serialNumber() == outgoing.serialNumber() )
        enable = true;

    QString msg;
    if ( enable )
        msg = tr( "<P>Activate: Enter your Call Barring password (PIN2)" );
    else
        msg = tr( "<P>Deactivate: Enter your Call Barring password (PIN2)" );

    QCallBarring::BarringType type = (QCallBarring::BarringType)item->data( Qt::UserRole ).toInt();
    QString pin2 = QPasswordDialog::getPassword( this, msg, QPasswordDialog::Pin );
    if ( pin2.isEmpty() )
        return;
    client->setBarringStatus( type, pin2, QTelephony::CallClassVoice, enable );
    client->requestBarringStatus( type );
    barOptions->setEnabled( false );
    waitWidget->show();
    updateMenu();
}

void CallBarring::setBarringStatusResult( QTelephony::Result result )
{
    if ( result == QTelephony::Error ) {
        QMessageBox::warning( this, tr( "Failed" ),
        tr( "<qt>Unable to bar calls(%1).</qt>", "%1 = condition string e.g. All incoming calls" )
        .arg( barOptions->currentItem()->data( Qt::WhatsThisRole ).toString() ), QMessageBox::Ok );
    } else if ( result == QTelephony::OperationNotAllowed ) {
        QMessageBox::warning( this, tr( "Not allowed" ),
        tr("<qt>Call barring is not allowed. Please contact your operator.</qt>"), QMessageBox::Ok );
    } else if ( result == QTelephony::IncorrectPassword ) {
        QMessageBox::warning( this, tr( "Incorrect Password" ), tr( "<qt>Please enter the correct password.</qt>" ), QMessageBox::Ok );
    }
}


void CallBarring::changeBarringPasswordResult( QTelephony::Result result )
{
    if ( result == QTelephony::Error )
        QMessageBox::warning( this, tr( "Failed" ),
        tr( "<qt>Failed to change password(%1)</qt>", "%1 = condition string e.g. All incoming calls" )
        .arg( barOptions->currentItem()->data( Qt::WhatsThisRole ).toString() ), QMessageBox::Ok );
    barOptions->setEnabled( true );
    updateMenu();
}

void CallBarring::unlockResult( QTelephony::Result result )
{
    if ( result == QTelephony::Error )
        QMessageBox::warning( this, tr( "Failed" ),
        tr( "<qt>Failed to unlock all barring" ), QMessageBox::Ok );
    barOptions->setEnabled( true );
    updateMenu();
}

void CallBarring::unlockAll()
{
    QString pin2 = QPasswordDialog::getPassword(
            this,
            tr( "<P>Deactivate all: Enter your Call Barring password (PIN2)" ),
            QPasswordDialog::Pin );
    if ( pin2.isEmpty() )
        return;
    client->unlockAll( pin2 );

    barOptions->setEnabled( false );
    updateMenu();

    // update icons
    QListWidgetItem *item;
    for ( int i = 0; i < barOptions->count(); i++ ) {
        item = barOptions->item( i );
        QCallBarring::BarringType t = (QCallBarring::BarringType)item->data( Qt::UserRole ).toInt();
        if ( t < QCallBarring::IncomingAll )
            item->setIcon( outgoing );
        else
            item->setIcon( incoming );
    }
}

void CallBarring::changePin()
{
    QListWidgetItem *item = barOptions->currentItem();

    QString pin2_old = QPasswordDialog::getPassword(
            this,
            tr( "<P>Enter your old PIN2 password (%1)", "%1 = condition string e.g. All incoming calls" )
            .arg( item->data( Qt::WhatsThisRole ).toString() ), QPasswordDialog::Pin, false );
    if ( pin2_old.isEmpty() )
        return;
    QString pin2_new = QPasswordDialog::getPassword(
            this,
            tr( "<P>Enter new PIN2 password (%1)", "%1 = condition string e.g. All incoming calls" )
            .arg( item->data( Qt::WhatsThisRole ).toString() ), QPasswordDialog::Pin );
    if ( pin2_new.isEmpty() )
        return;

    QCallBarring::BarringType type = (QCallBarring::BarringType)item->data( Qt::UserRole ).toInt();
    client->changeBarringPassword( type, pin2_old, pin2_new );

    barOptions->setEnabled( false );
    updateMenu();
}

void CallBarring::updateMenu()
{
    unlock->setVisible( barOptions->isEnabled() );
    unlock->setEnabled( barOptions->isEnabled() );
    pin->setVisible( barOptions->isEnabled() );
    pin->setEnabled( barOptions->isEnabled() );
}

//----------------------------------------------------------------------------

CallWaiting::CallWaiting( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), isLoading( true )
{
    init();

    connect( waitOptions, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated(QListWidgetItem*)) );
    connect( client, SIGNAL(callWaiting(QTelephony::CallClass)),
            this, SLOT(callWaiting(QTelephony::CallClass)) );
    connect( client, SIGNAL(setCallWaitingResult(QTelephony::Result)),
            this, SLOT(setCallWaitingResult(QTelephony::Result)) );
}

void CallWaiting::init()
{
    setObjectName( "waiting" );
    setWindowTitle( tr( "Call Waiting" ) );

    waitWidget = new QWaitWidget( this );
    waitWidget->setCancelEnabled( true );
    connect( waitWidget, SIGNAL(cancelled()), this, SLOT(reject()) );
    waitWidget->show();

    QVBoxLayout *vb = new QVBoxLayout(this);
    waitOptions = new QListWidget( this );
    waitOptions->setEnabled( false );
    waitOptions->setFrameStyle(QFrame::NoFrame);
    vb->addWidget( waitOptions );
    QListWidgetItem *item;

    item = new QListWidgetItem( tr( "Voice" ), waitOptions );
    item->setData( Qt::UserRole, QTelephony::CallClassVoice );
    item = new QListWidgetItem( tr( "Data" ), waitOptions );
    item->setData( Qt::UserRole, QTelephony::CallClassData );
    item = new QListWidgetItem( tr( "Fax" ), waitOptions );
    item->setData( Qt::UserRole, QTelephony::CallClassFax );

    waitOptions->setCurrentRow( 0 );

    client = new QCallSettings( SERVICE_NAME, this );
}

void CallWaiting::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    if ( isLoading )
        QTimer::singleShot( 200, this, SLOT(checkStatus()) );
}

void CallWaiting::checkStatus()
{
    client->requestCallWaiting();
}

void CallWaiting::itemActivated( QListWidgetItem * item )
{
    // check state is updated before signal QListWidget::itemActivated() emitted.
    // check state is not updated before signal QListWidget::itemClicked() emitted.
    // therefore we cannot rely on the current check state to update network.
    // items will cache info about whether active or not in QListWidgetItem::data( Qt::UserRole + 1 )
    bool enable = (Qt::CheckState)item->data( Qt::UserRole + 1 ).toInt() == Qt::Unchecked;
    QTelephony::CallClass c = (QTelephony::CallClass)item->data( Qt::UserRole ).toInt();
    client->setCallWaiting( enable, c );
    client->requestCallWaiting();
    waitOptions->setEnabled( false );
    waitWidget->show();
}

void CallWaiting::callWaiting( QTelephony::CallClass c )
{
    // check items if active
    waitOptions->item( 0 )->setCheckState( (c&QTelephony::CallClassVoice) ? Qt::Checked : Qt::Unchecked );
    waitOptions->item( 1 )->setCheckState( (c&QTelephony::CallClassData) ? Qt::Checked : Qt::Unchecked );
    waitOptions->item( 2 )->setCheckState( (c&QTelephony::CallClassFax) ? Qt::Checked : Qt::Unchecked );
    // cache info
    waitOptions->item( 0 )->setData( Qt::UserRole + 1, (int)(c&QTelephony::CallClassVoice) ? Qt::Checked : Qt::Unchecked );
    waitOptions->item( 1 )->setData( Qt::UserRole + 1, (int)(c&QTelephony::CallClassData) ? Qt::Checked : Qt::Unchecked );
    waitOptions->item( 2 )->setData( Qt::UserRole + 1, (int)(c&QTelephony::CallClassFax) ? Qt::Checked : Qt::Unchecked );
    waitOptions->setEnabled( true );
    waitOptions->setFocus();

    waitWidget->hide();
    isLoading = false;
}

void CallWaiting::setCallWaitingResult( QTelephony::Result r )
{
    if ( r == QTelephony::Error )
        QMessageBox::warning( this, tr( "Failed" ),
        tr( "<qt>Unable to make %1 calls wait.</qt>", "%1 = type of calls eg. Voice, Data" )
        .arg( waitOptions->currentItem()->text() ) );
}

//----------------------------------------------------------------------------

CallerID::CallerID( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();

    connect( toContact, SIGNAL(pressed()), this, SLOT(selectContactCategory()) );
    connect( client, SIGNAL(callerIdRestriction(QCallSettings::CallerIdRestriction,
                    QCallSettings::CallerIdRestrictionStatus)),
            this, SLOT(callerIdRestriction(QCallSettings::CallerIdRestriction,
                    QCallSettings::CallerIdRestrictionStatus)) );
}

void CallerID::init()
{
    setObjectName( "id" );
    setWindowTitle( tr( "Caller ID" ) );
    QSoftMenuBar::menuFor( this );
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin( 4 );

    toNone = new QRadioButton( tr( "To none" ), this );
    toAll = new QRadioButton( tr( "To all" ), this );
    opDefault = new QRadioButton( tr( "Operator default" ), this );
    toContact = new QRadioButton( tr( "To my Contacts" ), this );

    toNone->setEnabled( false );
    toAll->setEnabled( false );
    opDefault->setEnabled( false );
    toContact->setEnabled( false );

    vb->addWidget( toNone );
    vb->addWidget( toAll );
    vb->addWidget( opDefault );
    vb->addWidget( toContact );
    vb->addStretch( 1 );

    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallerId" );
    choice = cfg.value( "choice", 0 ).toInt();
    // choice 0 = Network default QCallSettings::Subscription
    // choice 1 = To none QCallSettings::Invoked
    // choice 2 = To al QCallSettings::Suppressed
    // choice 4 = To Contacts
    if ( choice == 4 ) {
        QCategoryFilter cat;
        cat.readConfig( cfg, "category" );
        QString newText;
        if (cat.acceptAll())
            newText = formatString( tr( "To all my contacts" ) );
        else
            newText = formatString( tr( "To my \"%1\" contacts", "%1=name of category e.g. personal" ).arg( cat.label() ) );
        toContact->setText( newText );
    }

    client = new QCallSettings( SERVICE_NAME, this );
    client->requestCallerIdRestriction();
}

void CallerID::accept()
{
    activate();
    QDialog::accept();
}

void CallerID::activate()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallerId" );
    int prevChoice = cfg.value( "choice" ).toInt();

    if ( toNone->isChecked() )
        choice = 1;
    else if ( toAll->isChecked() )
        choice = 2;
    else if ( opDefault->isChecked() )
        choice = 0;
    else if ( toContact->isChecked() )
        choice = 4;

    if ( prevChoice == choice )
        return;

    cfg.setValue( "choice", choice );

    QCallSettings::CallerIdRestriction r =
        choice == 4 ? QCallSettings::Invoked : (QCallSettings::CallerIdRestriction)choice;
    client->setCallerIdRestriction( r );
}

void CallerID::selectContactCategory()
{
    toContact->setChecked( true );

    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallerId" );

    QCategoryFilter cat;
    cat.readConfig(cfg, "category" );

    QCategoryDialog *catdlg = new QCategoryDialog( "Address Book", QCategoryDialog::Filter | QCategoryDialog::SingleSelection, this );
    catdlg->setModal( true );
    catdlg->selectFilter(cat);
    if ( QtopiaApplication::execDialog( catdlg ) ) {
        cat = catdlg->selectedFilter();
        cat.writeConfig( cfg, "category" );
        QString newText;
        if (cat.acceptAll())
            newText = formatString( tr( "To all my contacts" ) );
        else
            newText = formatString( tr( "To my \"%1\" contacts" ).arg( cat.label() ) );
        toContact->setText( newText );
    }
    delete catdlg;
}

void CallerID::callerIdRestriction( QCallSettings::CallerIdRestriction r,
            QCallSettings::CallerIdRestrictionStatus)
{
    if ( r == QCallSettings::Subscription )
        opDefault->setChecked( true );
    else if ( r == QCallSettings::Suppressed )
        toAll->setChecked( true );
    else {
        if ( (QCallSettings::CallerIdRestriction)choice == QCallSettings::Invoked )
            toNone->setChecked( true );
        else
            toContact->setChecked( true );
    }

    toNone->setEnabled( true );
    toAll->setEnabled( true );
    opDefault->setEnabled( true );
    toContact->setEnabled( true );

    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallerId" );
    int prevChoice = cfg.value( "choice" ).toInt();
    switch(prevChoice) {
        default:
        case 1:
            toNone->setFocus();
            break;
        case 2:
            toAll->setFocus();
            break;
        case 4:
            toContact->setFocus();
            break;
        case 0:
            opDefault->setFocus();
    }
}

QString CallerID::formatString( QString str )
{
    QDesktopWidget *desktop = QApplication::desktop();
    int availableWidth = desktop->availableGeometry(desktop->screenNumber(this)).width() - 40;
    QFontMetrics fm(font());

    // the string is not longer than the available width
    if ( fm.width( str ) < availableWidth )
        return str;

    // make the string multiple lines
    QString newStr;
    while ( true ) {
        int i = str.indexOf( " " );
        QString nextWord = str.left( i + 1 );
        if ( i != -1 && fm.width( newStr + nextWord ) < availableWidth ) {
            newStr += nextWord;
            str = str.right( str.length() - i - 1 );
        } else
            break;
    }

    // recursive call, in case two lines are not sufficient
    if ( fm.width( str ) > availableWidth )
        str = formatString( str );

    return newStr + '\n' + str;
}

//----------------------------------------------------------------------------

CellBroadcasting::CellBroadcasting( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();

    connect( channelList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated(QListWidgetItem*)) );
    connect( actionEdit, SIGNAL(triggered()), this, SLOT(edit()) );
    connect( actionAdd, SIGNAL(triggered()), this, SLOT(add()) );
    connect( actionRemove, SIGNAL(triggered()), this, SLOT(remove()) );
}

CellBroadcasting::~CellBroadcasting()
{
   for ( int i = 0; i < channels.count(); i++ )
        delete channels.at( i );
}

void CellBroadcasting::accept()
{
    activate();
    writeConfig();
    QDialog::accept();
}

void CellBroadcasting::init()
{
    setObjectName( "broadcast" );
    setWindowTitle( tr( "Cell Broadcasting" ) );

    QVBoxLayout *vb = new QVBoxLayout(this);
    channelList = new QListWidget( this );
    vb->addWidget( channelList );
    QListWidgetItem *item;

    client = new QCellBroadcast( SERVICE_NAME, this );
    readConfig();

    for ( int i = 0; i < channels.count(); i++ ) {
        item = new QListWidgetItem( QString::number( channels.at( i )->num ) + ":" + channels.at( i )->label, channelList );
        item->setCheckState( channels.at( i )->active ? Qt::Checked : Qt::Unchecked );
    }

    channelList->setCurrentRow( 0 );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    actionEdit = contextMenu->addAction( QIcon(":icon/edit"), tr( "Edit..." ) );
    actionAdd = contextMenu->addAction( QIcon(":icon/new"), tr( "Add..." ) );
    actionRemove = contextMenu->addAction( QIcon(":icon/trash"), tr( "Remove" ) );
}

void CellBroadcasting::activate()
{
    QList<int> c;
    for ( int i = 0; i < channels.count(); i++ ) {
        if ( channels.at( i )->active )
            c.append( channels.at( i )->num );
    }

    // always listen to channel 50 to cache location info
    // that way the info can be displayed immediately when turned on
    if ( !c.contains( 50 ) )
        c.append( 50 );

    client->setChannels( c );
}

void CellBroadcasting::readConfig()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    if ( cfg.value( "count" ).toInt() != 0 ) {
        int c = cfg.value( "count" ).toInt();
        for ( int i = 0; i < c; i++ ) {
            QString n = QString::number( i );
            Channel *c = new Channel;
            c->num = cfg.value( "num" + n ).toInt();
            c->label = cfg.value( "label" + n ).toString();
            c->active = cfg.value( "on" + n ).toBool();
            c->mode = (Mode)cfg.value( "mode" + n ).toInt();
            c->languages = cfg.value( "languages" + n ).toString().split( ',', QString::SkipEmptyParts );
            channels.append( c );
        }
    } else {
        // Built-in defaults
        // XXX should actually come from default settings
        Channel *c = new Channel;
        c->num = 50;
        c->label = tr( "Location" );
        c->active = false;
        c->mode = Background;
        c->languages.append( QString::number( QCBSMessage::English ) );
        channels.append( c );
        c = new Channel;
        c->num = 0;
        c->label = tr( "Index" );
        c->active = false;
        c->mode = Foreground;
        c->languages.append( QString::number( QCBSMessage::English ) );
        channels.append( c );
    }
}

void CellBroadcasting::writeConfig()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    cfg.remove("");
    int count = channels.count();
    cfg.setValue( "count", count );
    for ( int i = 0; i < count; i++ ) {
        QString n = QString::number( i );
        Channel *c = channels.at( i );
        cfg.setValue( "num" + n, c->num );
        cfg.setValue( "label" + n, c->label );
        cfg.setValue( "on" + n, c->active );
        cfg.setValue( "mode" + n, (int)c->mode );
        cfg.setValue( "languages" + n, c->languages.join( QString( ',' ) ) );
    }
}

bool CellBroadcasting::edit()
{
    int row = channelList->currentRow();
    Channel *c = channels.at( row );

    CellBroadcastEditDialog editDlg( this );
    editDlg.setModal( true );
    editDlg.setChannel( *c );

    if ( QtopiaApplication::execDialog( &editDlg ) ) {
        Channel modified = editDlg.channel();
        // check if the new channel aleady exists
        for ( int i = 0; i < channels.count() ; i++ ) {
            if ( i == channelList->currentRow() )
                continue;
            if ( channels.at( i )->num == modified.num ) {
                QMessageBox::warning( this, tr( "Failed" ),
                tr( "<qt>Channel %1 exists already.</qt>" ).arg( modified.num ) );
                return edit();
                break;
            }
        }
        c->num = modified.num;
        c->label = modified.label;
        c->mode = modified.mode;
        c->active = modified.active;
        c->languages = modified.languages;
        channelList->item( row )->setText( QString::number( c->num ) + ":" + c->label );
        return true;
    } else
        return false;
}

void CellBroadcasting::add()
{
    // find the next available channel
    int emptyChannel = 0;
    if ( channels.count() > 0 ) {
        QList<int> chList;
        for ( int i = 0; i < channels.count(); i++ )
            chList << channels.at( i )->num;
        qSort( chList );
        for ( int i = 0; i < chList.count() - 1; i++ ) {
            if ( chList.at( i ) + 1 == chList.at( i + 1 ) )
                continue;
            else {
                emptyChannel = chList.at( i ) + 1;
                break;
            }
        }
    }

    Channel *c = new Channel;
    c->num = emptyChannel;
    c->label = QString();
    c->mode = Foreground;
    c->active = true;
    c->languages.append( QString::number( QCBSMessage::English ) );
    channels.append( c );

    QListWidgetItem *item = new QListWidgetItem( QString::number( c->num ) + ":" + c->label, channelList );
    item->setCheckState( Qt::Checked );

    channelList->setCurrentRow( channelList->count() - 1 );

    if ( !edit() ) {
        delete item;
        channels.removeLast();
        c = 0;
    }
}

void CellBroadcasting::remove()
{
    int row = channelList->currentRow();
    int chnum = channels.at( row )->num;
    // cannot delete default channels
    if ( chnum == 50 ) {
        QMessageBox::warning( this, tr( "Failed" ),
        tr( "<qt>Cannot delete default channel %1.</qt>" ).arg( chnum ) );
        return;
    }
    delete channels.takeAt( row );
    delete channelList->takeItem ( row );
}

void CellBroadcasting::itemActivated( QListWidgetItem * item )
{
    int row = channelList->currentRow();
    bool enabled = channels.at( row )->active == true;
    item->setCheckState( !enabled ? Qt::Checked : Qt::Unchecked );
    channels.at( row )->active = !enabled;
}

//----------------------------------------------------------------------------

CellBroadcastEditDialog::CellBroadcastEditDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ), lstLang( 0 )
{
    setObjectName( "broadcast-edit" );

    editor = new Ui::ChannelEdit;
    QWidget *container = new QWidget(this);
    editor->setupUi( container );

    QScrollArea *sArea = new QScrollArea();
    sArea->setFocusPolicy(Qt::NoFocus);
    sArea->setFrameStyle(QFrame::NoFrame);
    sArea->setWidget( container );
    sArea->setWidgetResizable( true );
    sArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->addWidget( sArea );

    connect( editor->btnLang, SIGNAL(clicked()), this, SLOT(selectLanguages()) );
}

void CellBroadcastEditDialog::setChannel( CellBroadcasting::Channel c )
{
    ch.num = c.num;
    ch.label = c.label;
    ch.mode = c.mode;
    ch.active = c.active;
    ch.languages = c.languages;

    editor->num->setValue( ch.num );
    if ( !ch.label.isNull() )
        editor->title->setText( ch.label );
    switch ( ch.mode ) {
        case CellBroadcasting::Background:
            editor->homeScreen->setChecked( true );
            break;
        case CellBroadcasting::Foreground:
            editor->popup->setChecked( true );
            break;
    }

    if ( ch.num == 50 )
        editor->num->setEnabled( false );
}

CellBroadcasting::Channel CellBroadcastEditDialog::channel() const
{
    return ch;
}

void CellBroadcastEditDialog::selectLanguages()
{
    QDialog dlg( this );
    dlg.setModal( true );
    dlg.setWindowTitle( tr( "Select Languages" ) );
    QVBoxLayout layout( &dlg );
    layout.setMargin(0);
    lstLang = new QListWidget( &dlg );
    connect( lstLang, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated(QListWidgetItem*)) );
    setLanguages();
    layout.addWidget( lstLang );

    if ( QtopiaApplication::execDialog( &dlg ) ) {
        ch.languages.clear();
        for ( int i = 0; i < lstLang->count(); i++ ) {
            if ( lstLang->item( i )->checkState() == Qt::Checked )
                ch.languages.append(
                        QString::number( lstLang->item( i )->type() ) );
        }
        if ( ch.languages.count() == 0 )
            ch.languages.append( QString::number( QCBSMessage::English ) );
        delete lstLang;
    }
}

void CellBroadcastEditDialog::setLanguages()
{
    if ( !lstLang )
        return;

    //The type of these languages is determined by QCBSMessage::Language.
    //we are using QListWidgetItem::type() to keep track of the mappings
    QStringList langs;
    langs << tr( "German" );
    langs << tr( "English" );
    langs << tr( "Italian" );
    langs << tr( "French" );
    langs << tr( "Spanish" );
    langs << tr( "Dutch" );
    langs << tr( "Swedish" );
    langs << tr( "Danish" );
    langs << tr( "Portuguese" );
    langs << tr( "Finnish" );
    langs << tr( "Norwegian" );
    langs << tr( "Greek" );
    langs << tr( "Turkish" );

    for ( int i = 0 ; i< langs.count() ; i++ )
        void( new QListWidgetItem( langs[i], lstLang, i ) );
    lstLang->sortItems();

    for ( int i = 0; i < lstLang->count(); i++ ) {
        bool selected = ch.languages.contains( QString::number( lstLang->item( i )->type() ) );
        lstLang->item( i )->setCheckState( selected ? Qt::Checked : Qt::Unchecked );
        // cache selection info
        lstLang->item( i )->setData( Qt::UserRole + 1, selected ? Qt::Checked : Qt::Unchecked );
    }
}

void CellBroadcastEditDialog::itemActivated( QListWidgetItem *item )
{
    item->setCheckState( item->data( Qt::UserRole + 1 ) == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    item->setData( Qt::UserRole + 1, item->checkState() );
}

void CellBroadcastEditDialog::accept()
{
    ch.num = editor->num->value();
    ch.label = editor->title->text();
    if ( editor->homeScreen->isChecked() )
        ch.mode = CellBroadcasting::Background;
    else if ( editor->popup->isChecked() )
        ch.mode = CellBroadcasting::Foreground;
    QDialog::accept();
}

//----------------------------------------------------------------------------

FixedDialing::FixedDialing( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();

    connect( active, SIGNAL(toggled(bool)), this, SLOT(setFixedDialing(bool)) );
    connect( actionAdd, SIGNAL(triggered()), this, SLOT(add()) );
    connect( actionRemove, SIGNAL(triggered()), this, SLOT(remove()) );
}

void FixedDialing::init()
{
    setObjectName( "fixed" );
    setWindowTitle( tr( "Fixed Dialing" ) );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 2 );
    vb->setSpacing( 2 );

    active = new QCheckBox( tr( "Active" ), this );
    vb->addWidget( active );

    QLabel *lbl = new QLabel( tr( "Allowed Numbers" ), this );
    vb->addWidget( lbl );

    QDesktopWidget *desktop = QApplication::desktop();
    int listWidth = desktop->availableGeometry(desktop->screenNumber(this)).width() - 40;
    allowedNumbers = new FloatingTextList( this, listWidth );
    vb->addWidget( allowedNumbers );
    vb->setMargin(0);

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    actionAdd = contextMenu->addAction( QIcon(":icon/new"), tr( "Add..." ) );
    actionRemove = contextMenu->addAction( QIcon(":icon/trash"), tr( "Remove" ) );

    // read phonebook
    phonebook = new QPhoneBook( QString(), this );

    connect( phonebook, SIGNAL(setFixedDialingStateResult(QTelephony::Result)),
        this, SLOT(setFixedDialingStateResult(QTelephony::Result)) );
    connect( phonebook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
        this, SLOT(phoneBookEntries(QString,QList<QPhoneBookEntry>)) );
    connect( phonebook, SIGNAL(fixedDialingState(bool)),
        this, SLOT(fixedDialingState(bool)) );
    connect( phonebook, SIGNAL(limits(QString,QPhoneBookLimits)),
        this, SLOT(phonebookLimits(QString,QPhoneBookLimits)) );

    phonebook->requestFixedDialingState();
}

void FixedDialing::setFixedDialing( bool enabled )
{
    if ( enabled )
        pin2 = QPasswordDialog::getPassword( this, tr( "Enter your PIN2 password." ), QPasswordDialog::Pin );
    phonebook->flush( "FD" );
    phonebook->setFixedDialingState( enabled, pin2 );
    phonebook->clearPassword( "FD" );
}

void FixedDialing::fixedDialingState( bool enabled )
{
    active->setCheckState( enabled ? Qt::Checked : Qt::Unchecked );
    active->setFocus();

    if (pin2.isEmpty()) {
        // First time we got the state, so ask for the PIN2 value.
        pin2 = QPasswordDialog::getPassword( this, tr( "Enter your PIN2 password." ), QPasswordDialog::Pin );
        if ( pin2.isEmpty() )
            reject();
    }

    // Read the contents and limits of the FD phone book.
    phonebook->setPassword( "FD", pin2 );
    phonebook->getEntries( "FD" );
    phonebook->requestLimits( "FD" );
    //phonebook->clearPassword( "FD" );
}

void FixedDialing::phonebookLimits(const QString& store, const QPhoneBookLimits& value)
{
    if ( store != "FD" )
        return;

    // in 4.2.x, find out if fixed dialing is supported by reading limits.
    limit = value.lastIndex();
    if ( limit == 0 ) {
        active->setEnabled( false );
        actionAdd->setVisible( false );
        actionRemove->setVisible( false );
        QMessageBox::critical( this, tr( "Error" ), tr( "<qt>Fixed dialing is not supported.</qt>" ) );
    }
}

void FixedDialing::setFixedDialingStateResult( QTelephony::Result result )
{
    if ( result == QTelephony::OK )
        return;

    if ( result == QTelephony::Error )
        QMessageBox::critical( this, tr( "Error" ), tr("<qt>Attempt to change fixed dialing mode failed</qt>") );
    else if ( result == QTelephony::SimPuk2Required )
        QMessageBox::critical( this, tr( "Error" ), tr( "<qt>SIM PUK2 password is required.</qt>" ) );
    else if ( result == QTelephony::IncorrectPassword )
        QMessageBox::critical( this, tr( "Error" ), tr( "<qt>Password is invalid.</qt>" ) );
    else if ( result == QTelephony::OperationNotAllowed )
        QMessageBox::critical( this, tr( "Error" ), tr( "<qt>Fixed dialing is not supported.</qt>" ) );
    else
        QMessageBox::critical( this, tr( "Error" ), tr( "<qt>Unknown error occurred.</qt>" ) );
}

void FixedDialing::phoneBookEntries( const QString& pbook, const QList<QPhoneBookEntry>& entries )
{
    if ( pbook == "FD" ) {
        // remove the current item
        while ( allowedNumbers->count() > 0 )
            delete allowedNumbers->takeItem( 0 );
        QListWidgetItem *item;
        for ( QList<QPhoneBookEntry>::const_iterator it = entries.constBegin();
            it != entries.constEnd(); ++it)
        {
            item = new QListWidgetItem( QString(), allowedNumbers );
            item->setData( Qt::WhatsThisRole, QString("%1-%2").arg((*it).text()).arg((*it).number()) );
            item->setData( Qt::UserRole, (*it).index() );
        }
        allowedNumbers->showItemText();
    }
}

void FixedDialing::add()
{
    // check limit
    if ( qobject_cast<QListWidget*>(allowedNumbers)->count() == limit ) {
        QMessageBox::critical( this, tr( "Error" ),
                tr( "<qt>Cannot store more than %n phone number(s).</qt>",
                    "%n = number of phone numbers", limit ) );
        return;
    }

    QString name, number;
    QContactModel *contactModel = new QContactModel( this );
    QContactSelector *dlg = new QContactSelector( false, this );
    dlg->setModal( true );
    dlg->setModel( contactModel );
    QtopiaApplication::execDialog( dlg );
    if ( dlg->result() && dlg->contactSelected() ) {
        QContact contact = dlg->selectedContact();
        if ( !contact.firstName().isEmpty() )
            name = contact.firstName();
        if ( !contact.middleName().isEmpty() )
            name += " " + contact.middleName();
        if ( !contact.lastName().isEmpty() )
            name += " " + contact.lastName();
        QPhoneTypeSelector *typeSelector = new QPhoneTypeSelector( contact, QString(), this );
        typeSelector->setModal( true );
        QtopiaApplication::execDialog( typeSelector );
        if ( typeSelector->result() && !typeSelector->selectedNumber().isNull() )
            number = typeSelector->selectedNumber();
        delete typeSelector;
    }
    delete contactModel;
    delete dlg;

    if ( number.isEmpty() )
        return;

    QStringList list = number.split( QRegExp( "[*#,W!@]" ) );
    if ( list.count() != 1 ) {
        QMessageBox::warning( this, tr("Error"),
            tr( "<p>Number '%1' contains unacceptable characters for Fixed Dialing" )
            .arg( number ) );
        return;
    }

    int newIndex = qobject_cast<QListWidget*>(allowedNumbers)->count();
    QPhoneBookEntry entry;
    entry.setNumber( number );
    entry.setText( name );
    entry.setIndex( newIndex );
    phonebook->setPassword( "FD", pin2 );
    phonebook->add( entry, "FD" );
    phonebook->clearPassword( "FD" );

    QListWidgetItem *item = new QListWidgetItem( QString(), allowedNumbers );
    item->setData( Qt::WhatsThisRole, QString("%1-%2").arg(name).arg(number) );
    item->setData( Qt::UserRole, newIndex );
    allowedNumbers->setCurrentItem( item );
    allowedNumbers->showItemText();
}

void FixedDialing::remove()
{
    int row = allowedNumbers->currentRow();
    if ( row < 0 )
        return;

    int index = allowedNumbers->item( row )->data( Qt::UserRole ).toInt();
    phonebook->setPassword( "FD", pin2 );
    phonebook->remove( index, "FD" );
    phonebook->clearPassword( "FD" );

    delete allowedNumbers->takeItem( row );
}

//----------------------------------------------------------------------------

FlipFunction::FlipFunction( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();
}

FlipFunction::~FlipFunction()
{
    writeConfig();
}

void FlipFunction::init()
{
    setObjectName( "flip" );
    setWindowTitle( tr( "Flip Function" ) );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 2 );
    vb->setSpacing( 2 );

    answer = new QCheckBox( tr( "Answer on open"), this );
    vb->addWidget( answer );

    hangup = new QCheckBox( tr( "Hangup on close"), this );
    vb->addWidget( hangup );

    readConfig();
}

void FlipFunction::readConfig()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "FlipFunction" );
    answer->setChecked( cfg.value( "answer" ).toBool() );
    hangup->setChecked( cfg.value( "hangup" ).toBool() );
}

void FlipFunction::writeConfig()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "FlipFunction" );
    cfg.setValue( "answer", answer->isChecked() );
    cfg.setValue( "hangup", hangup->isChecked() );
}

//----------------------------------------------------------------------------

ServiceNumbers::ServiceNumbers( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), focusVoiceMail( false )
{
    init();
}

ServiceNumbers::~ServiceNumbers()
{
}

void ServiceNumbers::accept()
{
    if ( init_smsnum != serviceCenter->text() )
        serviceNumbers->setServiceNumber( QServiceNumbers::SmsServiceCenter, serviceCenter->text() );
    if ( init_voicenum != voiceMail->text() )
        serviceNumbers->setServiceNumber( QServiceNumbers::VoiceMail, voiceMail->text() );
    QDialog::accept();
}

void ServiceNumbers::init()
{
    setObjectName( "service" );
    setWindowTitle( tr( "Service Numbers" ) );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 2 );
    vb->setSpacing( 2 );

    QGroupBox *box1 = new QGroupBox( tr( "SMS Service Center" ), this );
    QVBoxLayout *layout1 = new QVBoxLayout( box1 );
    serviceCenter = new QLineEdit( box1 );
    QtopiaApplication::setInputMethodHint( serviceCenter, QtopiaApplication::PhoneNumber );
    layout1->addWidget( serviceCenter );
    layout1->setMargin( 4 );
    layout1->setSpacing( 2 );
    vb->addWidget( box1 );

    QGroupBox *box2 = new QGroupBox( tr( "Voice Mail" ), this );
    QVBoxLayout *layout2 = new QVBoxLayout( box2 );
    voiceMail = new QLineEdit( box2 );
    QtopiaApplication::setInputMethodHint( voiceMail, QtopiaApplication::PhoneNumber );
    layout2->addWidget( voiceMail );
    layout2->setMargin( 4 );
    layout2->setSpacing( 2 );
    vb->addWidget( box2 );

    serviceNumbers = new QServiceNumbers( QString(), this );
    connect( serviceNumbers, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
            this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );
    serviceNumbers->requestServiceNumber( QServiceNumbers::SmsServiceCenter );
    serviceNumbers->requestServiceNumber( QServiceNumbers::VoiceMail );
}

void ServiceNumbers::serviceNumber( QServiceNumbers::NumberId id, const QString& number )
{
    if ( id == QServiceNumbers::SmsServiceCenter ) {
        init_smsnum = number;
        serviceCenter->setText( init_smsnum );
    } else if ( id == QServiceNumbers::VoiceMail ) {
        init_voicenum = number;
        voiceMail->setText( init_voicenum );
    }
    if ( focusVoiceMail )
        voiceMail->setFocus();
    else
        serviceCenter->setFocus();
}

//----------------------------------------------------------------------------

CallVolume::CallVolume( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
      m_changeSpeakerVolume(true),
      m_changeMicrophoneVolume(true)
{
    init();

    connect(speakerVolume, SIGNAL(valueChanged(int)),
            this, SLOT(speakerSliderChanged(int)));
    connect(microphoneVolume, SIGNAL(valueChanged(int)),
            this , SLOT(microphoneSliderChanged(int)));

    connect(callVolume, SIGNAL(speakerVolumeChanged(int)),
            this, SLOT(speakerVolumeChanged(int)));
    connect(callVolume, SIGNAL(microphoneVolumeChanged(int)),
            this, SLOT(microphoneVolumeChanged(int)));
}

CallVolume::~CallVolume()
{
}

void CallVolume::accept()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallVolume" );

    cfg.setValue( "SpeakerVolume", callVolume->speakerVolume() );
    cfg.setValue( "MicrophoneVolume", callVolume->microphoneVolume() );

    QDialog::accept();
}

void CallVolume::reject()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CallVolume" );

    callVolume->setSpeakerVolume(m_oldSpeakerVolume);
    cfg.setValue( "SpeakerVolume", m_oldSpeakerVolume );

    callVolume->setMicrophoneVolume(m_oldMicrophoneVolume);
    cfg.setValue( "MicrophoneVolume", m_oldMicrophoneVolume );

    QDialog::reject();
}

void CallVolume::speakerSliderChanged(int volume)
{
    if (m_changeSpeakerVolume)
        callVolume->setSpeakerVolume(volume);
}

void CallVolume::speakerVolumeChanged(int volume)
{
    m_changeSpeakerVolume = false;
    speakerVolume->setValue(volume);
    m_changeSpeakerVolume = true;
}

void CallVolume::microphoneSliderChanged(int volume)
{
    if (m_changeMicrophoneVolume)
        callVolume->setMicrophoneVolume(volume);
}

void CallVolume::microphoneVolumeChanged(int volume)
{
    m_changeMicrophoneVolume = false;
    microphoneVolume->setValue(volume);
    m_changeMicrophoneVolume = true;
}

void CallVolume::init()
{
    setObjectName( "volume" );
    setWindowTitle( tr( "Call Volume" ) );

    callVolume = new QCallVolume( QString(), this );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 2 );
    vb->setSpacing( 2 );

    QGroupBox *box1 = new QGroupBox( tr( "Speaker Volume" ), this );
    QVBoxLayout *layout1 = new QVBoxLayout( box1 );
    speakerVolume = new QSlider( Qt::Horizontal, box1 );
    speakerVolume->setRange(callVolume->minimumSpeakerVolume(), callVolume->maximumSpeakerVolume());
    m_oldSpeakerVolume = callVolume->speakerVolume();
    speakerVolume->setValue(m_oldSpeakerVolume);
    layout1->addWidget( speakerVolume );
    vb->addWidget( box1 );

    QGroupBox *box2 = new QGroupBox( tr( "Microphone Volume" ), this );
    QVBoxLayout *layout2 = new QVBoxLayout( box2 );
    microphoneVolume = new QSlider( Qt::Horizontal, box2 );
    microphoneVolume->setRange(callVolume->minimumMicrophoneVolume(),
                               callVolume->maximumMicrophoneVolume());
    m_oldMicrophoneVolume = callVolume->microphoneVolume();
    microphoneVolume->setValue(m_oldMicrophoneVolume);
    layout2->addWidget( microphoneVolume );
    vb->addWidget( box2 );
}

/*!
    \service VoiceMailService VoiceMail
    \inpublicgroup QtCellModule
    \brief Provides the voice mail configuration service.

    The \i VoiceMail service allows to set the voice mail number.
*/
/*! \internal */
VoiceMailService::~VoiceMailService()
{
}

/*!
    Set voice mail number.

    This slot corresponds to the QCop service message
    \c{VoiceMail::setVoiceMail()}.
*/
void VoiceMailService::setVoiceMail()
{
    parent->raise();
    parent->selectVoiceMail = true;
    parent->activate( PhoneSettings::Service );
    QtopiaApplication::instance()->showMainWidget();
}
