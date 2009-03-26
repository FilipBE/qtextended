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

#include "accountconfig.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#ifdef QTOPIA_BLUETOOTH
#include <QPushButton>
#include <QBluetoothLocalDevice>
#include <QBluetoothRemoteDevice>
#include <QBluetoothRemoteDeviceDialog>
#endif

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>


/*!
  \class AccountPage
    \inpublicgroup QtBaseModule
  \brief The AccountPage class provides the user interface for the network account configuration.

  \internal

  The AccountPage widget is exclusively used by the Qt Extended network plug-ins. It 
  allows the user to edit the general account details such as:

  \list
    \o user name
    \o password
    \o APN
    \o dialup number
    \o Autostart behavior
  \endlist

  Note that not all of these details are always enabled. The type of the network plug-in 
  determines which subwidgets are relevant for a particular network interface type.

  This is not a public class.
  */

class AccountPagePrivate
{
public:
    void _q_selectBluetoothDevice()
    {
#ifdef QTOPIA_BLUETOOTH
        QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice( 0 );
        if ( !addr.isValid() )
            return;
        remotePartner = addr;
        QFont f = QApplication::font();
        f.setItalic( ! (btDevice->isUp().value()) );
        btPartner->setFont( f );
        btPartner->setText( btDeviceName( addr) );
#endif
    }

    void _q_BluetoothStateChanged()
    {
#ifdef QTOPIA_BLUETOOTH
        if ( btDevice->isUp().value() ) {
            chooseBtPartner->setEnabled( true );
            if ( remotePartner.isValid() )
                btPartner->setText( btDeviceName(remotePartner) );
        }  else {
            chooseBtPartner->setEnabled( false);
            btPartner->setText( AccountPage::tr("<Bluetooth disabled>") );
        }
        QFont f = QApplication::font();
        f.setItalic( ! (btDevice->isUp().value()) );
        btPartner->setFont( f );

#endif
    }
#ifdef QTOPIA_BLUETOOTH
    QString btDeviceName( const QBluetoothAddress& addr )
    {
        QBluetoothRemoteDevice remote( addr );
        QBluetoothLocalDevice localDev;
        QString alias = localDev.remoteAlias( addr ).value();
        if ( !alias.isEmpty() ) {
            return alias;
        }
        localDev.updateRemoteDevice( remote );
        if ( remote.name().isEmpty() )
           return addr.toString();
        else
           return remote.name();
    }

    QLabel* btPartner;
    QBluetoothAddress remotePartner;
    QPushButton* chooseBtPartner;
    QBluetoothLocalDevice* btDevice;
#endif
    AccountPage* parent;
};


AccountPage::AccountPage(
        QtopiaNetwork::Type type, const QtopiaNetworkProperties& cfg,
        QWidget* parent, Qt::WFlags flags )
    : QWidget( parent, flags | Qt::Window ), accountType( type )
{
    d = new AccountPagePrivate;
    d->parent = this;
    init();
    readConfig( cfg );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );

    //this is required for help lookup
    if ( accountType & QtopiaNetwork::Dialup )
        setObjectName(QLatin1String("dialup-account"));
    else if ( accountType & QtopiaNetwork::GPRS )
        setObjectName(QLatin1String("gprs-account"));
    else if ( accountType & QtopiaNetwork::BluetoothDUN )
        setObjectName(QLatin1String("bluetooth-account"));
    else
        setObjectName( QLatin1String("lan-account") );
}

AccountPage::~AccountPage()
{
    delete d;
}

QtopiaNetworkProperties AccountPage::properties()
{
    QtopiaNetworkProperties props;

    //prevent duplication of interface names
    QStringList allInterfaces = QtopiaNetwork::availableNetworkConfigs( accountType );
    QStringList allNames;
    foreach( QString iface, allInterfaces ) {
        QTranslatableSettings cfg( iface, QSettings::IniFormat );
        allNames << cfg.value(QLatin1String("Info/Name")).toString();
    }
    QString n = name->text();
    bool initialName = true;
    int idx = 1;
    int count = allNames.count( n );
    //find next available name
    //note: allNames always contains this iface too. we must ensure that we don't trigger a false positive.
    while( (count > 1 && initialName) || ( count>=1 && !initialName ) ) {
        n = name->text()+QString::number(idx);
        idx++;
        initialName = false;
        count = allNames.count( n );
    }

    props.insert( QLatin1String("Info/Name"), n );
    if ( accountType & ~QtopiaNetwork::BluetoothDUN )
        props.insert( QLatin1String("Properties/Autostart"), startup->currentIndex() ? "y" : "n" );

    if ( accountType & (QtopiaNetwork::Dialup | QtopiaNetwork::GPRS | QtopiaNetwork::BluetoothDUN) ) {
        props.insert( QLatin1String("Properties/UserName"), user->text());
        props.insert( QLatin1String("Properties/Password"), password->text());

        if ( accountType & QtopiaNetwork::GPRS )
            props.insert( QLatin1String("Serial/APN"), dialup->text() );
        else if ( accountType & QtopiaNetwork::Dialup )
            props.insert( QLatin1String("Serial/Phone"), dialup->text() );
    }

#ifdef QTOPIA_BLUETOOTH
    if ( accountType & QtopiaNetwork::BluetoothDUN )
        props.insert( QLatin1String("Serial/PartnerDevice"), d->remotePartner.isValid() ? d->remotePartner.toString() : QLatin1String("") );
#endif

    return props;
}

void AccountPage::init()
{
    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setMargin( 5 );
    vb->setSpacing( 4 );

    QLabel *name_label = new QLabel( tr("Account name:"), this );
    vb->addWidget( name_label );
    name = new QLineEdit( this );
    vb->addWidget( name );
    name_label->setBuddy( name );

    startup_label = new QLabel( tr("Startup mode:"), this );
    vb->addWidget( startup_label );
    startup = new QComboBox( this );
    startup->addItems( QStringList() << tr("When needed") << tr("Always online") );
    vb->addWidget( startup );
    startup_label->setBuddy( startup );

    dialup_label = new QLabel( this );
    vb->addWidget( dialup_label );
    dialup = new QLineEdit( this );
    vb->addWidget( dialup );
    dialup_label->setBuddy( dialup );

    user_label = new QLabel( tr("Username:"), this );
    vb->addWidget( user_label );
    user = new QLineEdit( this );
    QtopiaApplication::setInputMethodHint( user, "text noautocapitalization");
    vb->addWidget( user );
    user_label->setBuddy( user );

    password_label = new QLabel( tr("Password:"), this );
    vb->addWidget( password_label );
    password = new QLineEdit( this );
    password->setEchoMode( QLineEdit::PasswordEchoOnEdit );
    vb->addWidget( password );
    password_label->setBuddy( password );

#ifdef QTOPIA_BLUETOOTH
    if ( accountType & QtopiaNetwork::BluetoothDUN ) {
        QLabel* bt_label = new QLabel( tr("Bluetooth partner:"), this );
        vb->addWidget( bt_label );
        d->btPartner = new QLabel( this );
        QHBoxLayout* hb = new QHBoxLayout;
        hb->setMargin( 0 );
        hb->setSpacing( 4 );
        vb->addLayout( hb );
        hb->addWidget( d->btPartner );
        hb->addStretch( 1 );
        d->chooseBtPartner = new QPushButton( QIcon(":icon/settings"), "", this );
        connect( d->chooseBtPartner, SIGNAL(clicked(bool)), this, SLOT(_q_selectBluetoothDevice()) );
        hb->addWidget( d->chooseBtPartner );

        d->btDevice = new QBluetoothLocalDevice();
        connect( d->btDevice, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
                 this, SLOT(_q_BluetoothStateChanged()) );
    }
#endif

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
            QSizePolicy::Minimum, QSizePolicy::Expanding );
    vb->addItem( spacer );
}

void AccountPage::readConfig( const QtopiaNetworkProperties& prop)
{
    if ( accountType & ( QtopiaNetwork::Dialup | QtopiaNetwork::GPRS | QtopiaNetwork::BluetoothDUN ) ) {
        user->setText( prop.value(QLatin1String("Properties/UserName")).toString() );
        password->setText( prop.value(QLatin1String("Properties/Password")).toString() );

        if ( accountType & QtopiaNetwork::GPRS ) {
            dialup_label->setText( tr("APN:", "GPRS access point") );
            dialup->setText( prop.value(QLatin1String("Serial/APN")).toString());
            QtopiaApplication::setInputMethodHint( dialup, "text noautocapitalization" );
        } else if ( accountType & QtopiaNetwork::BluetoothDUN ) {
#ifdef QTOPIA_BLUETOOTH
            d->remotePartner = QBluetoothAddress( prop.value(QLatin1String("Serial/PartnerDevice")).toString() );
            if ( d->remotePartner.isValid() )
                d->btPartner->setText( d->btDeviceName( d->remotePartner ) );
            d->_q_BluetoothStateChanged();
#endif
            dialup_label->hide();
            dialup->hide();
        } else {
            dialup_label->setText( tr("Dialup number:") );
            QtopiaApplication::setInputMethodHint( dialup, QtopiaApplication::Number );
            dialup->setText( prop.value(QLatin1String("Serial/Phone")).toString() );
        }
    } else {
        dialup_label->hide();
        dialup->hide();
        user_label->hide();
        user->hide();
        password_label->hide();
        password->hide();
    }
    name->setText( prop.value(QLatin1String("Info/Name")).toString() );
    if ( accountType & ~QtopiaNetwork::BluetoothDUN ) {
        startup->setCurrentIndex( prop.value(QLatin1String("Properties/Autostart")).toString() == "y" );
    } else {
        startup_label->hide();
        startup->hide();
    }
}

#include "moc_accountconfig.cpp"
