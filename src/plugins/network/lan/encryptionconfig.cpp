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

#include "encryptionconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QButtonGroup>
#include <QDocumentSelectorDialog>
#include <QMessageBox>
#include <QPainter>
#include <QtopiaApplication>
#include <QSet>

#include <hexkeyvalidator.h>

#include <qsoftmenubar.h>

QIcon crossOutIcon(const QString& icon)
{
    QPixmap pixmap( icon );
    QPainter p;
    QPen pen( Qt::red );
    pen.setWidth( 2 );
    p.begin( &pixmap );
    p.setPen( pen );
    p.drawLine( 0, 0, pixmap.width(), pixmap.height() );
    p.drawLine( 0, pixmap.height(), 0, pixmap.width() );
    p.end();

    return QIcon( pixmap );
}


WirelessEncryptionPage::WirelessEncryptionPage( const QtopiaNetworkProperties& cfg,
        QWidget* parent, Qt::WFlags flags )
    : QWidget( parent, flags ), lastIndex( 0 )
{
    ui.setupUi( this );

    QButtonGroup* grp = new QButtonGroup( this );
    grp->addButton( ui.key1_check );
    grp->addButton( ui.key2_check );
    grp->addButton( ui.key3_check );
    grp->addButton( ui.key4_check );

    HexKeyValidator* wepValidator = new HexKeyValidator( this );
    ui.key1->setValidator( wepValidator );
    ui.key2->setValidator( wepValidator );
    ui.key3->setValidator( wepValidator );
    ui.key4->setValidator( wepValidator );

    ui.passphrase->setEchoMode( QLineEdit::PasswordEchoOnEdit );
    ui.ident_password->setEchoMode( QLineEdit::PasswordEchoOnEdit );

    ui.authentication->addItem( tr("Any") );
    ui.authentication->addItem( tr("MD5", "EAP authentication method") ); 
    ui.authentication->addItem( tr("GTC", "EAP authentication method") ); 
    ui.authentication->addItem( tr("MSCHAPv2", "EAP authentication method") ); 

    connect( ui.client_cert_selector, SIGNAL(clicked()), this, SLOT(fileSelected()) );
    connect( ui.server_cert_selector, SIGNAL(clicked()), this, SLOT(fileSelected()) );
    connect( ui.client_key_selector, SIGNAL(clicked()), this, SLOT(fileSelected()) );

    init( cfg );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
    selectEncryptAlgorithm( ui.encrypt->currentIndex() );
    connect( ui.netSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(newNetSelected(int)) );
    connect( ui.encrypt, SIGNAL(currentIndexChanged(int)), this, SLOT(selectEncryptAlgorithm(int)) );
    connect( ui.encrypt_type, SIGNAL(currentIndexChanged(int)), this, SLOT(selectEncryptType(int)) );
    connect( ui.passphrase, SIGNAL(editingFinished()), this, SLOT(checkPassword()) );
    connect( ui.WPAtype, SIGNAL(currentIndexChanged(int)), this, SLOT(wpaEnterpriseChanged(int)) );
}

WirelessEncryptionPage::~WirelessEncryptionPage()
{
}

QtopiaNetworkProperties WirelessEncryptionPage::properties()
{
    if ( !isEnabled() ) {
        QtopiaNetworkProperties p;
        p.insert("WirelessNetworks/size", 0);
        return p;
    }

    saveConfig();
    return props;
}

void WirelessEncryptionPage::init( const QtopiaNetworkProperties& cfg )
{
    props.clear();
    lastIndex = 0;
    ui.netSelector->clear();
    const QList<QString> keys = cfg.keys();
    const int numWLANs = cfg.value( QLatin1String("WirelessNetworks/size"), 0 ).toInt();
    const bool hasWLANs = numWLANs > 0;
    if ( hasWLANs ) {
        setEnabled( true );
    } else {
        setEnabled( false );
        ui.netSelector->addItem("<No WLAN defined>");
        props.clear();
        readConfig();
        return;
    }
    int idx = 0;
    QString normalizedKey;
    foreach( QString key, keys ) {
        if ( !key.startsWith("WirelessNetworks")  )
            continue;
        idx = key.mid(17, key.indexOf(QChar('/'), 17)-17).toInt();
        if ( idx <= numWLANs ) {
            //copy all values into props where we keep track of changes
            props.insert( key, cfg.value( key ) );
            if ( key.endsWith( "ESSID" ) ) {
                QString text = cfg.value( key ).toString();
                if ( text.isEmpty() )
                    text = tr("Unnamed network");
                ui.netSelector->addItem( text );
            }
        }
    }

    readConfig( );
}

void WirelessEncryptionPage::readConfig( )
{
    documents.clear();
    const QString s = QString("WirelessNetworks/%1/").arg( ui.netSelector->currentIndex() +1 );
    QString enc = props.value(s+"Encryption").toString();
    if ( enc == "none" )
        ui.encrypt->setCurrentIndex( 0 );
    else if ( enc == "shared" )
        ui.encrypt->setCurrentIndex( 2 );
    else if ( enc == "WPA-PSK" )
        ui.encrypt->setCurrentIndex( 3 );
    else if ( enc == "WPA-EAP" )
        ui.encrypt->setCurrentIndex( 4 );
    else
        ui.encrypt->setCurrentIndex( 1 ); //Open authentication

    QString key = props.value(s+"SelectedKey").toString();
    if ( !key.isEmpty() && key.at(key.length()-1).isDigit() ) {
        switch( key.at(key.length()-1).digitValue() ) {
            case 0:
                ui.key1_check->setChecked( true );
                break;
            case 1:
                ui.key2_check->setChecked( true );
                break;
            case 2:
                ui.key3_check->setChecked( true );
                break;
            case 3:
                ui.key4_check->setChecked( true );
                break;
            default:
                break;
        }
        ui.encrypt_type->setCurrentIndex( 1 );
    } else {
        ui.encrypt_type->setCurrentIndex( 0 ); // use passphrase
    }

    if ( props.value(s+"KeyLength").toInt() != 64 )
        ui.bit128->setChecked( true );
    else
        ui.bit64->setChecked( true );

    ui.key1->setText( props.value(s+"WirelessKey_1").toString() );
    ui.key2->setText( props.value(s+"WirelessKey_2").toString() );
    ui.key3->setText( props.value(s+"WirelessKey_3").toString() );
    ui.key4->setText( props.value(s+"WirelessKey_4").toString() );
    ui.passphrase->setText( props.value(s+"PRIV_GENSTR").toString());

    const QString psk_encrypt = props.value(s+"PSKAlgorithm").toString();
    ui.wpa_psk_encrypt->setCurrentIndex( psk_encrypt == QLatin1String("AES") ? 1 : 0 );

    const QString wpa_eap = props.value(s+"WPAEnterprise", QLatin1String("TLS")).toString();
    int wpa_eap_idx = 0; //defaults to TLS
    if ( wpa_eap == "TTLS" )
        wpa_eap_idx = 1;
    else if ( wpa_eap == "PEAP" )
        wpa_eap_idx = 2;
    ui.WPAtype->setCurrentIndex( wpa_eap_idx );
    wpaEnterpriseChanged( wpa_eap_idx );

    ui.identity->setText( props.value(s+"EAPIdentity").toString() );
    ui.anon_ident->setText( props.value(s+"EAPAnonIdentity").toString() );
    ui.ident_password->setText( props.value(s+"EAPIdentityPassword").toString());

    QContent doc = QContent(props.value(s+"EAPClientCert").toString());
    documents[ui.client_cert_selector] = doc.isValid() ? doc : QContent();
    ui.client_cert_selector->setIcon( !doc.isValid() ? 
            QIcon(":icon/padlock") : crossOutIcon(QLatin1String(":icon/padlock")) );
    ui.client_cert->setText( doc.name() );

    doc = QContent(props.value(s+"EAPServerCert").toString() );
    documents[ui.server_cert_selector] = doc.isValid() ? doc : QContent();
    ui.server_cert_selector->setIcon( !doc.isValid() ? 
            QIcon(":icon/padlock") : crossOutIcon(QLatin1String(":icon/padlock")) );
    ui.server_cert->setText( doc.name() );

    doc = QContent(props.value(s+"EAPClientKey").toString() );
    documents[ui.client_key_selector] = doc.isValid() ? doc : QContent();
    ui.client_key_selector->setIcon( !doc.isValid() ? 
            QIcon(":icon/padlock") : crossOutIcon(QLatin1String(":icon/padlock")) );
    ui.client_key->setText( doc.name() );

    ui.client_key_pw->setText( props.value(s+"EAPClientKeyPassword").toString() );
    
    const QString wpa_auth = props.value(s+"EAPAuthentication", QLatin1String("Any")).toString();
    int wpa_auth_idx = 0;
    if ( wpa_auth == QLatin1String("MD5" ) )
        wpa_auth_idx = 1;
    else if ( wpa_auth == QLatin1String( "GTC" ) )
        wpa_auth_idx = 2;
    else if ( wpa_auth == QLatin1String( "MSCHAPV2" ) )
        wpa_auth_idx = 3;
    ui.authentication->setCurrentIndex( wpa_auth_idx );
}

/*!
  \internal

  Save options of current WLAN
*/
void WirelessEncryptionPage::saveConfig()
{
    if ( lastIndex < 0 || lastIndex >= ui.netSelector->count() )
        return;

    const QString s = QString("WirelessNetworks/%1/").arg(lastIndex+1);
    switch( ui.encrypt->currentIndex() ) {
        case 0:
            props.insert( s+"Encryption", "none" );
            break;
        case 2:
            props.insert( s+"Encryption", "shared" );
            break;
        case 3:
            props.insert( s+"Encryption", "WPA-PSK" );
            break;
        case 4:
            props.insert( s+"Encryption", "WPA-EAP" );
            break;
        case 1:
        default:
            props.insert( s+"Encryption", "open" );
            break;
    }

    if ( ui.encrypt_type->currentIndex() == 0)
        props.insert(s+"SelectedKey", "PP");
    else {
        if (ui.key2_check->isChecked())
            props.insert(s+"SelectedKey", "K1");
        else if (ui.key3_check->isChecked())
            props.insert(s+"SelectedKey", "K2");
        else if (ui.key4_check->isChecked())
            props.insert(s+"SelectedKey", "K3");
        else
            props.insert(s+"SelectedKey", "K0");
    }

    props.insert(s+"KeyLength", ui.bit64->isChecked() ? 64 : 128 );

    props.insert(s+"WirelessKey_1", ui.key1->text());
    props.insert(s+"WirelessKey_2", ui.key2->text());
    props.insert(s+"WirelessKey_3", ui.key3->text());
    props.insert(s+"WirelessKey_4", ui.key4->text());
    props.insert(s+"PRIV_GENSTR", ui.passphrase->text());
    if ( ui.wpa_psk_encrypt->currentIndex() == 0 )
        props.insert(s+"PSKAlgorithm", "TKIP" );
    else
        props.insert(s+"PSKAlgorithm", "AES");

    //WPA-EAP parameter
    QString wpa_eap;
    switch( ui.WPAtype->currentIndex() ) {
        case 0:
            wpa_eap = "TLS";
            break;
        case 1:
            wpa_eap = "TTLS";
            break;
        case 2:
            wpa_eap = "PEAP";
            break;
    }
    props.insert(s+"WPAEnterprise", wpa_eap);
    props.insert(s+"EAPIdentity", ui.identity->text());
    props.insert(s+"EAPAnonIdentity", ui.anon_ident->text());
    props.insert(s+"EAPIdentityPassword", ui.ident_password->text());

    QContent doc = documents[ui.client_cert_selector];
    if (doc.isValid()) {
        doc.setCategories( doc.categories() << QLatin1String("Certificate") );
        doc.commit();
    }
    props.insert(s+"EAPClientCert", doc.fileName());

    doc = documents[ui.server_cert_selector];
    if (doc.isValid()) {
        doc.setCategories( doc.categories() << QLatin1String("Certificate") );
        doc.commit();
    }
    props.insert(s+"EAPServerCert", doc.fileName());

    doc = documents[ui.client_key_selector];
    if (doc.isValid()) {
        doc.setCategories( doc.categories() << QLatin1String("Security Key") );
        doc.commit();
    }
    props.insert(s+"EAPClientKey", doc.fileName());

    props.insert(s+"EAPClientKeyPassword", ui.client_key_pw->text());
    QString wpa_auth = QLatin1String("Any");
    switch( ui.authentication->currentIndex() )
    {
        case 1:
            wpa_auth = QLatin1String("MD5");
            break;
        case 2:
            wpa_auth = QLatin1String("GTC");
            break;
        case 3:
            wpa_auth = QLatin1String("MSCHAPV2");
            break;
        default:
            break;
    }
    props.insert(s+"EAPAuthentication", wpa_auth);
}

void WirelessEncryptionPage::newNetSelected( int newIdx )
{
    if ( newIdx < 0 || newIdx >= ui.netSelector->count() )
        return;

    saveConfig();
    lastIndex = newIdx;
    readConfig();
}

void WirelessEncryptionPage::selectEncryptAlgorithm( int index )
{
    switch( index ) {
        case 0: //None
            ui.pskPwLabel->setVisible( false );
            ui.encrypt_type->setVisible( false );
            ui.passphrase->setVisible( false );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( false );
            ui.wpa_psk_encrypt->setVisible( false );
            break;
        case 1: //Open
        case 2: //Shared key
        default:
            {
                ui.pskPwLabel->setVisible( false );
                ui.wpa_psk_encrypt->setVisible( false );
                ui.encrypt_type->setVisible( true );
                ui.encrypt_type->setEnabled( true );
                const bool multikey = ui.encrypt_type->currentIndex();
                ui.multkeys->setVisible( multikey );
                ui.passphrase->setVisible( !multikey );
                ui.EAPBox->setVisible( false );
            }
            break;
        case 3: // WPA-PSK
            ui.encrypt_type->setVisible( false );
            ui.pskPwLabel->setVisible( true );
            ui.wpa_psk_encrypt->setVisible( true );
            ui.passphrase->setVisible( true );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( false );
            ui.encrypt_type->setCurrentIndex( 0 ); //passphrase only
            break;
        case 4: // WPA-EAP
            ui.pskPwLabel->setVisible( false );
            ui.wpa_psk_encrypt->setVisible( false );
            ui.encrypt_type->setVisible( false );
            ui.passphrase->setVisible( false );
            ui.multkeys->setVisible( false );
            ui.EAPBox->setVisible( true );
            break;
    }
}

void WirelessEncryptionPage::selectEncryptType( int index )
{
    const bool multikey = index == 1;
    ui.multkeys->setVisible( multikey );
    ui.passphrase->setVisible( !multikey );
}

void WirelessEncryptionPage::setProperties( const QtopiaNetworkProperties& cfg )
{
    init( cfg );
}

void WirelessEncryptionPage::checkPassword()
{
    if ( ui.encrypt->currentIndex() == 3 ) { //WPA-PSK
        const QString pw = ui.passphrase->text();
        if ( pw.length() < 8 ) {
            QMessageBox::critical( this, tr("WPA-PSK error"), tr("<qt>Password must be 8 characters or longer.</qt>"),
                     QMessageBox::Ok, QMessageBox::NoButton );
            //ui.passphrase->setEditFocus( true );
        }
    }
}

void WirelessEncryptionPage::wpaEnterpriseChanged(int index)
{
    const bool useTLS = (index == 0);
    ui.ident_password->setVisible( !useTLS );
    ui.ident_passwordLabel->setVisible( !useTLS );
    ui.clientKeyPwLabel->setVisible( useTLS );
    ui.client_key_pw->setVisible( useTLS );

    ui.client_cert->setVisible( useTLS );
    ui.clientCertLabel->setVisible( useTLS );
    ui.client_cert_selector->setVisible( useTLS );
    ui.client_key->setVisible( useTLS );
    ui.clientKeyLabel->setVisible( useTLS );
    ui.client_key_selector->setVisible( useTLS );
    ui.authentication->setVisible( !useTLS );
    ui.authLabel->setVisible( !useTLS );
    ui.anonIdentityLabel->setVisible( !useTLS );
    ui.anon_ident->setVisible( !useTLS );

}

void WirelessEncryptionPage::fileSelected()
{
    QToolButton* btn = qobject_cast<QToolButton*>(sender());
    if ( !btn )
        return;

    QContent doc = documents[btn];
    if ( doc.isValid() ) {
        documents.insert( btn, QContent() );
        btn->setIcon( QIcon(":icon/padlock") );        
        if (btn == ui.client_cert_selector) {
            ui.client_cert->setText( "" );
        } else if (btn == ui.server_cert_selector) {
            ui.server_cert->setText( "" );
        } else if (btn == ui.client_key_selector) {
            ui.client_key->setText( "" );
        }
        return;
    }

    QDocumentSelectorDialog dlg;
    dlg.setModal( true );
    dlg.setFilter( QContentFilter( QContent::Document ) );
    dlg.setDefaultCategories( QStringList() << QLatin1String("Certificate") 
            << QLatin1String("Security Key") );
    if ( QtopiaApplication::execDialog( &dlg ) == QDialog::Accepted ) {
        QContent content = dlg.selectedDocument();
        if ( !content.isValid() ) {
            btn->setIcon( QIcon(":icon/padlock") );        
            return;
        }
        
    
        if (btn == ui.client_cert_selector) {
            ui.client_cert->setText( content.name() );
        } else if (btn == ui.server_cert_selector) {
            ui.server_cert->setText( content.name() );
        } else if (btn == ui.client_key_selector) {
            ui.client_key->setText( content.name() );
        } else {
            return;
        }
        btn->setIcon(crossOutIcon( QLatin1String(":icon/padlock") ) );  
        documents[btn]=content;
    }

}
#endif // NO_WIRELESS_LAN
