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

#include "callforwarding.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qcontactview.h>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QListWidget>
#include <QStringList>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QList>
#include <QRadioButton>
#include <QKeyEvent>
#include <QTimer>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qwaitwidget.h>

static QString voiceMailNumber = QString();

class CallForwardingDigitValidator : public QValidator
{
public:
    CallForwardingDigitValidator( bool plus, QWidget *parent );

    virtual State validate( QString &, int & ) const;

private:
    bool allowPlus;
};

CallForwardingDigitValidator::CallForwardingDigitValidator( bool plus, QWidget *parent )
    : QValidator( parent ), allowPlus( plus )
{
}

QValidator::State CallForwardingDigitValidator::validate( QString &input, int & ) const
{
    QString valid( "0123456789" );
    if ( allowPlus )
        valid += '+';

    QString fixed;
    for ( int i = 0 ; i < input.length() ; i++ ) {
        if ( valid.contains( input[i] ) )
            fixed += input[i];
    }

    input = fixed;

    return QValidator::Acceptable;
}

//----------------------------------------------------

ContactSelectDialog::ContactSelectDialog( QWidget *parent )
    : QDialog( parent )
{
    init();

    connect( contactList, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
    connect( contactList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectedContact(QListWidgetItem*)));
    connect( actionDeleteAll, SIGNAL(triggered()), this, SLOT(deleteAll()));
    connect( actionDeleteNumber, SIGNAL(triggered()), this, SLOT(deleteNumber()));
}

void ContactSelectDialog::init()
{
    setObjectName("contact");

    QtopiaApplication::setMenuLike( this, true );
    setWindowTitle( tr( "Select contact" ) );
    showMaximized();

    actionDeleteNumber = new QAction( QIcon( ":icon/trash" ), tr( "Remove number" ), this );
    actionDeleteAll = new QAction( QIcon( ":icon/trash" ), tr( "Remove all" ), this );

    contextMenu = QSoftMenuBar::menuFor( this );
    contextMenu->addAction( actionDeleteNumber );
    contextMenu->addAction( actionDeleteAll );

    QSettings setting( "Trolltech", "Phone" );
    setting.beginGroup( "UsedNumbers" );
    contacts = setting.value( "Numbers" ).toString().split( ',', QString::SkipEmptyParts );

    contactList = new QListWidget( this );

    if ( contacts.count() != 0 ) {
        actionDeleteAll->setEnabled( true );
        QStringList::Iterator it;
        for (it = contacts.begin(); it != contacts.end(); ++it)
            new QListWidgetItem( (*it), contactList );
    }

    if ( !voiceMailNumber.isEmpty() )
        voiceMail = new QListWidgetItem( tr( "To voice mail..." ), contactList );
    findContact = new QListWidgetItem( tr( "Find contact..." ), contactList );
    typeNumber = new QListWidgetItem( tr( "Type number..." ), contactList );

    QHBoxLayout *hLayout = new QHBoxLayout( this );
    hLayout->addWidget( contactList );

    contactList->setCurrentRow( 0 );
    itemSelectionChanged();
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
}

void ContactSelectDialog::selectedContact( QListWidgetItem *item )
{
    if ( item == voiceMail ) {
        selectedNumber = voiceMailNumber;
        accept();
    } else if ( item == findContact )
        numberFromContacts();
    else if ( item == typeNumber )
        numberFromInputLine();
    else {
        selectedNumber = item->text();
        accept();
    }
}

void ContactSelectDialog::numberFromContacts()
{
    QContactModel *contactModel = new QContactModel( this );
    QContactSelector *dlg = new QContactSelector( false, this );
    dlg->setModal( true );
    dlg->setModel( contactModel );
    dlg->showMaximized();
    QtopiaApplication::execDialog( dlg );
    if ( dlg->result() && dlg->contactSelected() ) {
        QContact contact = dlg->selectedContact();

        QPhoneTypeSelector *typeSelector = new QPhoneTypeSelector( contact, QString(), this );
        typeSelector->setModal( true );
        QtopiaApplication::execDialog( typeSelector );
        if ( typeSelector->result() && !typeSelector->selectedNumber().isNull() )
            addNumber( typeSelector->selectedNumber() );
        delete typeSelector;
    }
    delete contactModel;
    delete dlg;
}

void ContactSelectDialog::numberFromInputLine()
{
    QDialog *dlg = new QDialog( this );
    dlg->setObjectName("type");
    dlg->setWindowTitle( tr( "Enter number" ) );
    dlg->setModal( true );
    QVBoxLayout *vLayout = new QVBoxLayout( dlg );
    QLineEdit *line = new QLineEdit( dlg );
    QtopiaApplication::setInputMethodHint( line, QtopiaApplication::PhoneNumber );
    line->setMaxLength(15);
    line->setValidator( new CallForwardingDigitValidator( true, this ) );
    vLayout->addWidget( line );
    QtopiaApplication::execDialog( dlg );
    if ( dlg->result() && line->text().length() > 0 )
        addNumber( line->text() );
    delete dlg;
}

QString ContactSelectDialog::number() const
{
    return selectedNumber;
}

void ContactSelectDialog::addNumber( const QString& newNumber )
{
    if ( !contacts.contains( newNumber ) ) {
        contacts.push_front( newNumber );
        contactList->insertItem( 0, new QListWidgetItem( newNumber ) );
        contactList->setCurrentRow( 0 );
    }
    saveSettings();
}

void ContactSelectDialog::deleteAll()
{
    contacts.clear();
    contactList->clear();
    findContact = new QListWidgetItem( tr( "Find contact..." ), contactList );
    typeNumber = new QListWidgetItem( tr( "Type number..." ), contactList );
    saveSettings();
    contactList->setCurrentRow( 0 );
    itemSelectionChanged();
}

void ContactSelectDialog::deleteNumber()
{
    int i = contactList->currentRow();
    if ( i != -1 ) {
        contacts.removeAt( i );
        delete contactList->takeItem( i );
    }
    saveSettings();
    itemSelectionChanged();
}

void ContactSelectDialog::saveSettings()
{
    QSettings setting( "Trolltech", "Phone" );
    setting.beginGroup( "UsedNumbers" );
    setting.setValue( "Numbers", contacts.join(QString(',')) );
}

void ContactSelectDialog::itemSelectionChanged()
{
    QListWidgetItem *item = contactList->currentItem();
    bool enable = item != findContact && item != typeNumber;
    actionDeleteNumber->setEnabled( enable );
    actionDeleteNumber->setVisible( enable );
    enable = contactList->count() > 3;
    actionDeleteAll->setEnabled( enable );
    actionDeleteAll->setVisible( enable );
}

//----------------------------------------------------------------

CallForwardItem::CallForwardItem( QTelephony::CallClass c, QCallForwarding::Reason r, QWidget *parent )
    : QCheckBox( parent ), classX( c ), reason( r ), forwardNumber( QString() )
, statusUpdate( false ), abort( false ), currentStatus( false ), newStatus( false )
{
    init();
    if ( reason == QCallForwarding::Unconditional )
        connect( this, SIGNAL(toggled(bool)), parent, SLOT(alwaysChecked(bool)));

    connect( this, SIGNAL(toggled(bool)), this, SLOT(checked(bool)) );
    connect( this, SIGNAL(sendRequest(QCallForwarding::Reason,QString)),
        parent, SLOT(receiveRequest(QCallForwarding::Reason,QString)) );
}

void CallForwardItem::init()
{
    setText( false );
    readSettings();
}

QString CallForwardItem::conditionName()
{
    switch ( reason ) {
    case QCallForwarding::Unconditional:
        return tr( "Always" );
    case QCallForwarding::MobileBusy:
        return tr( "When busy" );
    case QCallForwarding::NoReply:
        return tr( "When unanswered" );
    case QCallForwarding::NotReachable:
        return tr( "When unavailable" );
    default:
        return QString();
    }
}

void CallForwardItem::checked( bool on )
{
    if ( statusUpdate )
        return;

    QString number;
    if ( on ) { // attempt to activate
        if ( !abort ) { // normal operation
            number = selectContact();
            if ( !number.isNull() && number != "0" ) {
                newNumber = number;
                newStatus = true;
                emit sendRequest( reason, newNumber );
            } else { // no number selected
                abort = true;
                setChecked( !on ); // go back to previous status, i.e. inactive
            }
        } else { // deactivating operation cancelled
            abort = false;
        }
    } else { // attempt to deactivate
        if ( !abort ) { // normal operation
            int result = selectOperationType();
            if ( result == 1 ) { // deactivate
                newStatus = false;
                emit sendRequest( reason, "0" );
            } else if ( result == 0 ) { // use other number
                ContactSelectDialog *dlg = new ContactSelectDialog( this );
                dlg->setModal( true );
                QtopiaApplication::execDialog( dlg );
                if ( dlg->result() )
                    number = dlg->number();
                delete dlg;
                if ( !number.isNull() && number != "0" ) {
                    newNumber = number;
                    newStatus = true;
                    emit sendRequest( reason, newNumber );
                }
                abort = true;
                setChecked( !on );
            } else { // do nothing
                abort = true;
                setChecked( !on ); // go back to previous status, i.e. active
            }
        } else { // activating operation cancelled
            abort = false;
        }
    }
}

void CallForwardItem::setForwardingResult( QTelephony::Result result )
{
    statusUpdate = true;

    if ( result == QTelephony::OK ) {
        currentStatus = newStatus;
        // if activation update number, if not, keep previous number.
        if ( currentStatus )
            forwardNumber = newNumber;
    }
    // initialize
    newNumber = QString();
    newStatus = false;

    // update ui
    setText( currentStatus, forwardNumber );
    setChecked( currentStatus );

    statusUpdate = false;
}

void CallForwardItem::setText( bool enabled, const QString &number )
{
    QString txt = conditionName();

    if ( enabled )
        txt += "\n" + tr( "forward to %1", "When busy forward to %1<phone number>" ).arg( number );

    QCheckBox::setText( txt );
}

void CallForwardItem::deactivate()
{
    statusUpdate = true;
    setChecked( false );
    setText( false );
    statusUpdate = false;
}

int CallForwardItem::selectOperationType()
{
    QDialog *dlg = new QDialog( this );
    QSoftMenuBar::setLabel( dlg, Qt::Key_Back, QSoftMenuBar::Next );
    dlg->setObjectName( "set" );
    dlg->setWindowTitle( tr( "Please select" ) );
    dlg->setModal( true );
    QVBoxLayout *vLayout = new QVBoxLayout( dlg );
    vLayout->setMargin( 4 );
    QRadioButton *deactivate = new QRadioButton( tr( "Deactivate" ) );
    deactivate->setChecked( true );
    QRadioButton *useOtherNumber = new QRadioButton( tr( "Use other number" ) );
    vLayout->addWidget( deactivate );
    vLayout->addWidget( useOtherNumber );
    QtopiaApplication::execDialog( dlg );

    int r = -1;
    if ( dlg->result() && deactivate->isChecked() )
        r = 1;
    else if ( dlg->result() && useOtherNumber->isChecked() )
        r = 0;
    else if ( !dlg->result() )
        r = -1;
    delete dlg;
    return r;
}

QString CallForwardItem::selectContact()
{
    QString number;
    bool openConatactList = false;
    if ( forwardNumber.isEmpty() || forwardNumber == "0" )
        openConatactList = true;
    else {
        if ( usePreviousNumber( openConatactList ) )
            number = forwardNumber;
    }

    if ( openConatactList ) {
        ContactSelectDialog *dlg = new ContactSelectDialog( this );
        QtopiaApplication::setMenuLike( dlg, true );
        dlg->setModal( true );
        if ( QtopiaApplication::execDialog( dlg ) )
            number = dlg->number();
        delete dlg;
    }
    return number;
}

bool CallForwardItem::usePreviousNumber( bool &openConatactList )
{
    QDialog *dlg = new QDialog( this );
    QSoftMenuBar::setLabel( dlg, Qt::Key_Back, QSoftMenuBar::Next );
    dlg->setObjectName("set");
    dlg->setWindowTitle( tr( "Previously used number" ) );
    dlg->setModal( true );
    QVBoxLayout *vLayout = new QVBoxLayout( dlg );
    vLayout->setMargin( 4 );
    QRadioButton *usePrevNumber = new QRadioButton( tr( "Would you like to use\n%1?", "%1 == phone number" ).arg( forwardNumber ) );
    usePrevNumber->setChecked( true );
    QRadioButton *useOtherNumber = new QRadioButton( tr( "Other number" ) );
    vLayout->addWidget( usePrevNumber );
    vLayout->addWidget( useOtherNumber );
    QtopiaApplication::execDialog( dlg );

    bool r = false;
    if ( dlg->result() && usePrevNumber->isChecked() )
        r = true;
    else if ( dlg->result() && useOtherNumber->isChecked() )
        openConatactList = true;
    delete dlg;
    return r;
}

void CallForwardItem::setStatus( const bool enabled, const QString number )
{
    statusUpdate = true;
    forwardNumber = number;
    currentStatus = enabled;
    setChecked( currentStatus );
    setText( currentStatus, number );
    statusUpdate = false;
}

QString CallForwardItem::status() const
{
    QString status;
    if ( isChecked() )
        status = QString::number( classX ) + "," + QString::number( reason ) + "," + forwardNumber + ",";
    return status;
}

void CallForwardItem::keyPressEvent(QKeyEvent *ke)
{
    int key = ke->key();
    switch ( key ) {
        case Qt::Key_Select:
            setChecked( !isChecked() );
            break;
        case Qt::Key_Back:
        case Qt::Key_Up:
        case Qt::Key_Down:
            ke->ignore();
            break;
        default:
            QCheckBox::keyPressEvent( ke );
            break;
    }
}

void CallForwardItem::readSettings()
{
    statusUpdate = true;

    QSettings setting( "Trolltech", "Phone" );
    setting.beginGroup( QString::number( classX ) + "-" + QString::number( reason ) );
    forwardNumber = setting.value( "Number", 0 ).toString();
    currentStatus = setting.value( "Active" ).toBool();
    setChecked( currentStatus );
    setText( currentStatus, forwardNumber );
    statusUpdate = false;
}
//-------------------------------------------------------

CallClassTab::CallClassTab( QTelephony::CallClass c, QWidget *mainWidget, QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f ), classX( c )
{
    init();

    connect( this, SIGNAL(sendRequest(QCallForwarding::Reason,QString,QTelephony::CallClass)),
        mainWidget, SLOT(receiveRequest(QCallForwarding::Reason,QString,QTelephony::CallClass)));

}

void CallClassTab::init()
{
    QVBoxLayout *vLayout = new QVBoxLayout( this );
    vLayout->setMargin( 2 );
    vLayout->setSpacing( 0 );
    vLayout->setAlignment( Qt::AlignTop );

    alwaysItem = new CallForwardItem( classX, QCallForwarding::Unconditional, this );
    busyItem = new CallForwardItem( classX, QCallForwarding::MobileBusy, this );
    unansweredItem = new CallForwardItem( classX, QCallForwarding::NoReply, this );
    unavailableItem = new CallForwardItem( classX, QCallForwarding::NotReachable, this );

    vLayout->addWidget( alwaysItem );
    vLayout->addWidget( busyItem );
    vLayout->addWidget( unansweredItem );
    vLayout->addWidget( unavailableItem );

    alwaysChecked( alwaysItem->isChecked() );
}

QString CallClassTab::typeName()
{
    QString type;
    if( classX == QTelephony::CallClassVoice )
        type = tr( "Voice calls" );
    else if( classX == QTelephony::CallClassData )
        type = tr( "Data calls" );
    else if( classX == QTelephony::CallClassFax )
        type = tr( "FAX calls" );
    else if( classX == QTelephony::CallClassSMS )
        type = tr( "SMS" );
    return type;
}

void CallClassTab::alwaysChecked( bool on )
{
    busyItem->setEnabled( !on );
    unansweredItem->setEnabled( !on );
    unavailableItem->setEnabled( !on );
}

void CallClassTab::deactivateAll()
{
    alwaysItem->deactivate();
    busyItem->deactivate();
    unansweredItem->deactivate();
    unavailableItem->deactivate();
}

void CallClassTab::receiveRequest( const QCallForwarding::Reason reason, const QString& number )
{
    emit sendRequest( reason, number, classX );
}

void CallClassTab::setForwardingResult( QCallForwarding::Reason reason, QTelephony::Result result )
{
    if ( reason == QCallForwarding::Unconditional )
        alwaysItem->setForwardingResult( result );
    else if ( reason == QCallForwarding::MobileBusy )
        busyItem->setForwardingResult( result );
    else if ( reason == QCallForwarding::NoReply )
        unansweredItem->setForwardingResult( result );
    else if ( reason == QCallForwarding::NotReachable )
        unavailableItem->setForwardingResult( result );
}

QTelephony::CallClass CallClassTab::callClassX()
{
    return classX;
}

void CallClassTab::setStatus( QCallForwarding::Reason reason, const QCallForwarding::Status status )
{
    if ( reason == QCallForwarding::Unconditional )
        alwaysItem->setStatus( true, status.number );
    else if ( reason == QCallForwarding::MobileBusy )
        busyItem->setStatus( true, status.number );
    else if ( reason == QCallForwarding::NoReply )
        unansweredItem->setStatus( true, status.number );
    else if ( reason == QCallForwarding::NotReachable )
        unavailableItem->setStatus( true, status.number );
}

QString CallClassTab::status() const
{
    QString status;
    status += alwaysItem->status();
    status += busyItem->status();
    status += unansweredItem->status();
    status += unavailableItem->status();
    return status;
}

void CallClassTab::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    alwaysItem->setFocus();
}

void CallClassTab::readSettings()
{
    alwaysItem->readSettings();
    busyItem->readSettings();
    unansweredItem->readSettings();
    unavailableItem->readSettings();
}

//--------------------------------------------------------------------

CallForwarding::CallForwarding( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ), isAutoActivation( false ), isStatusView( false ), isLoaded( false )
{
    init();
    showMaximized();

    connect( actionDeactivateAll, SIGNAL(triggered()), this, SLOT(deactivateAll()) );
    connect( actionCheckStatus, SIGNAL(triggered()), this, SLOT(checkStatus()) );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pushSettingStatus()) );

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(receive(QString,QByteArray)) );

    connect( client, SIGNAL(forwardingStatus(QCallForwarding::Reason,
                    const QList<QCallForwarding::Status>&)),
            this, SLOT(forwardingStatus(QCallForwarding::Reason,
                    const QList<QCallForwarding::Status>&)) );
    connect( client, SIGNAL(setForwardingResult(QCallForwarding::Reason,QTelephony::Result)),
            this, SLOT(setForwardingResult(QCallForwarding::Reason,QTelephony::Result)) );
}

void CallForwarding::init()
{
    tabWidget = new QTabWidget( this );

    voiceTab = new CallClassTab( QTelephony::CallClassVoice, this  );
    dataTab = new CallClassTab( QTelephony::CallClassData, this );
    faxTab = new CallClassTab( QTelephony::CallClassFax, this );
    smsTab = new CallClassTab( QTelephony::CallClassSMS, this );

    tabWidget->addTab( voiceTab, voiceTab->typeName() );
    tabWidget->addTab( dataTab, dataTab->typeName() );
    tabWidget->addTab( faxTab, faxTab->typeName() );
    tabWidget->addTab( smsTab, smsTab->typeName() );

    delete layout();
    QVBoxLayout *vLayout = new QVBoxLayout( this );
    vLayout->setMargin( 0 );
    vLayout->setSpacing( 0 );
    vLayout->addWidget( tabWidget );

    actionDeactivateAll = new QAction( tr( "Deactivate all" ), this );
    actionCheckStatus = new QAction( tr( "Check status", "ask the network for the current forwarding status") , this );
    actionCapture = new QAction( QIcon( ":icon/Note" ), tr( "Add to current profile" ), this );
    contextMenu = QSoftMenuBar::menuFor( this );
    contextMenu->addAction( actionDeactivateAll );
    contextMenu->addAction( actionCheckStatus );
    contextMenu->addAction( actionCapture );
    client = new QCallForwarding( "modem", this );

    setWindowTitle( tr( "Forwarding" ) );

    splash = new QWaitWidget( this );
    splash->setCancelEnabled( true );
    connect( splash, SIGNAL(cancelled()), this, SLOT(reject()) );

    reqType = NoRequest;

    QTimer::singleShot( 500, this, SLOT(checkStatus()) );
}

void CallForwarding::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds( data );
    if ( msg == "Settings::setStatus(bool,QString)" ) {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        isStatusView = true;
        QSoftMenuBar::removeMenuFrom( this, contextMenu );
        delete contextMenu;
        QString details;
        ds >> isFromActiveProfile;
        ds >> details;
        setStatus( details );
        isLoaded = true;
        show();
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        QString details;
        ds >> details;
        isAutoActivation = true;
        activate( details );
        hide();
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        pullSettingStatus();
        hide();
    } else if ( msg == "Settings::activateDefault()" ) {
        isAutoActivation = true;
        deactivateAll();
        hide();
    }
}

void CallForwarding::deactivateAll()
{
    QCallForwarding::Status status;
    status.cls = QTelephony::CallClassDefault;
    status.number = QString();
    status.time = 0;
    client->setForwarding( QCallForwarding::All, status, false );

    int cx, qt;
    QString number;
    QSettings setting( "Trolltech", "Phone" );

    for ( cx = QTelephony::CallClassVoice ; cx <= QTelephony::CallClassFax ; cx++ ) {
        for ( qt = QCallForwarding::Unconditional ; qt <= QCallForwarding::NotReachable ; qt++ ) {
            setting.beginGroup( QString::number( cx ) + "-" + QString::number( qt ) );
            setting.setValue( "Active", false );
            setting.endGroup();
        }
    }

    // need to update home screen
    client->requestForwardingStatus( QCallForwarding::Unconditional );

    voiceTab->deactivateAll();
    dataTab->deactivateAll();
    faxTab->deactivateAll();
    smsTab->deactivateAll();
}

void CallForwarding::receiveRequest( QCallForwarding::Reason reason, QString number, QTelephony::CallClass c )
{
    currentItem.reason = reason;
    currentItem.enabled = number == "0" ? false : true;
    currentItem.classx = c;
    currentItem.number = number;

    reqType = currentItem.enabled ? Activation : Deactivation;

    if ( isStatusView ) {
        // update status at the item level
        qobject_cast<CallClassTab *>(tabWidget->currentWidget())->setForwardingResult(reason, QTelephony::OK);
        if ( isFromActiveProfile ) // save to activate on exit
            forwardItemList.append(currentItem);
    } else { // for normal operation, activate immediately
        QCallForwarding::Status status;
        status.cls = c;
        status.number = number;
        status.time = 20;
        client->setForwarding( reason, status, currentItem.enabled );

        client->requestForwardingStatus( reason );
        if ( !isAutoActivation )
            splash->show();
    }
}

void CallForwarding::forwardingStatus( QCallForwarding::Reason reason, const QList<QCallForwarding::Status> &status )
{
    QSettings setting( "Trolltech", "Phone" );

    if ( reason == QCallForwarding::NotReachable )
        splash->hide();

    switch( reqType ) {
    case Status:
        for ( int i = 0; i < status.count(); i++ ) {
            switch ( status.at( i ).cls ) {
            case QTelephony::CallClassVoice:
                voiceTab->setStatus( reason, status.at( i ) );
                break;
            case QTelephony::CallClassData:
                dataTab->setStatus( reason, status.at( i ) );
                break;
            case QTelephony::CallClassFax:
                faxTab->setStatus( reason, status.at( i ) );
                break;
            case QTelephony::CallClassSMS:
                smsTab->setStatus( reason, status.at( i ) );
                break;
            default:
                break;
            }
            setting.beginGroup( QString::number( status.at( i ).cls )
                    + "-" + QString::number( reason ) );
            setting.setValue( "Active", true );
            setting.setValue( "Number", status.at( i ).number );
            setting.endGroup();
        }
        break;
    default:
        break;
    }
}


void CallForwarding::setForwardingResult( QCallForwarding::Reason reason, QTelephony::Result result )
{
    if ( splash->isVisible() )
        splash->hide();
    // if error ocurred notify requester and do not write config
    if ( result == QTelephony::Error ) {
        QMessageBox::information( this, tr("Forwarding failed"),
                            tr("<qt>The operation is not allowed. Please consult your network operator.</qt>") );
        qobject_cast<CallClassTab *>(tabWidget->currentWidget())->setForwardingResult(reason, result);
        return;
    } else if ( result == QTelephony::OperationNotAllowed ) {
        QMessageBox::information( this, tr("Forwarding failed"),
                            tr("<qt>Please check if the number is correct.</qt>") );
        qobject_cast<CallClassTab *>(tabWidget->currentWidget())->setForwardingResult(reason, result);
        return;
    }

    // update status at the item level
    qobject_cast<CallClassTab *>(tabWidget->currentWidget())->setForwardingResult(reason, result);

    // if successful, update config
    QSettings setting( "Trolltech", "Phone" );
    setting.beginGroup( QString::number( currentItem.classx ) + "-" + QString::number( reason ) );

    switch ( reqType ) {
    case Activation:
        setting.setValue( "Active", true );
        setting.setValue( "Number", currentItem.number );
       break;
    case Deactivation:
        setting.setValue( "Active", false );
        break;
    default:
        break;
    }
}

void CallForwarding::checkStatus()
{
    if ( isStatusView )
        return;

    reqType = Status;
    client->requestForwardingStatus( QCallForwarding::Unconditional );
    client->requestForwardingStatus( QCallForwarding::MobileBusy );
    client->requestForwardingStatus( QCallForwarding::NoReply );
    client->requestForwardingStatus( QCallForwarding::NotReachable );

    QServiceNumbers *serviceNum = new QServiceNumbers( "modem", this );
    connect( serviceNum, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
            this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );
    serviceNum->requestServiceNumber( QServiceNumbers::VoiceMail );

    isLoaded = true;
    show();
    splash->show();
}

void CallForwarding::pushSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pushSettingStatus(QString,QString,QString)" );
    e << QString( "callforwarding" ) << QString( windowTitle() ) << status();
    e.send();
}

void CallForwarding::pullSettingStatus()
{
    voiceTab->readSettings();
    dataTab->readSettings();
    faxTab->readSettings();
    smsTab->readSettings();

    QtopiaServiceRequest e( "SettingsManager", "pullSettingStatus(QString,QString,QString)" );
    e << QString( "callforwarding" ) << QString( windowTitle() ) << status();
    e.send();
}

QString CallForwarding::status()
{
    QString status;
    status += voiceTab->status();
    status += dataTab->status();
    status += faxTab->status();
    status += smsTab->status();
    return status;
}

void CallForwarding::setStatus( const QString details )
{
    QStringList s = details.split( ',' );
    int c = s.count();
    QCallForwarding::Reason reason;
    QString number;

    for ( int i = 0; i < c - 1; ) {
        QCallForwarding::Status status;
        status.cls = (QTelephony::CallClass)s.value( i++ ).toInt();
        reason = (QCallForwarding::Reason)s.value( i++ ).toInt();
        status.number = s.value( i++ );
        status.time = 20;
        if ( status.cls == QTelephony::CallClassVoice )
            voiceTab->setStatus( reason, status );
        else if ( status.cls == QTelephony::CallClassData )
            dataTab->setStatus( reason, status );
        else if ( status.cls == QTelephony::CallClassSMS )
            smsTab->setStatus( reason, status );
        else if ( status.cls == QTelephony::CallClassFax )
            faxTab->setStatus( reason, status );
    }
}

void CallForwarding::activate( const QString details )
{
    deactivateAll();

    if ( details == QString() )
        return;

    int cx, reason;
    QString number;
    QStringList s = details.split( ',' );
    int c = s.count();

    for ( int i = 0; i < c - 1; ) {
        cx = s.value( i++ ).toInt();
        reason = s.value( i++ ).toInt();
        number = s.value( i++ );
        receiveRequest( (QCallForwarding::Reason)reason, number, (QTelephony::CallClass)cx );
    }
}

void CallForwarding::accept()
{
    if ( isStatusView ) {
        if ( isFromActiveProfile ) { // activate on exit
            ForwardItem curItem;
            QSettings setting( "Trolltech", "Phone" );
            for ( int i = 0; i < forwardItemList.size(); i++ ) {
                curItem = forwardItemList.at( i );

                // save setting file
                setting.beginGroup( QString::number(curItem.classx) + "-" + QString::number(curItem.reason) );
                setting.setValue( "Active", curItem.enabled );
                setting.setValue( "Number", curItem.number );
                setting.endGroup();

                // modify network
                QCallForwarding::Status status;
                status.cls = curItem.classx;
                status.number = curItem.number;
                status.time = 20;
                client->setForwarding( curItem.reason, status, curItem.enabled );
            }
            // query to update homescreen
            client->requestForwardingStatus( QCallForwarding::Unconditional );
        }
        // pass details to profile
        pushSettingStatus();
    } else {
        // if the current profile has forwarding item
        // update the details with the new setting
        QSettings cfg( "Trolltech", "PhoneProfile" );
        cfg.beginGroup( "Profiles" );
        QString activeProfile = cfg.value( "Selected", 1 ).toString();
        cfg.endGroup();
        cfg.beginGroup( "Profile " + activeProfile );
        QString settings = cfg.value( "SettingList" ).toString();
        if ( settings.contains( "callforwarding" ) )
            pushSettingStatus();
    }
    QDialog::accept();
    close();
}

void CallForwarding::serviceNumber(QServiceNumbers::NumberId,const QString& value)
{
    voiceMailNumber = value;
}
