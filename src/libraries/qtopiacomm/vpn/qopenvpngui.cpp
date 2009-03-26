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

#include "qopenvpngui_p.h"

#ifndef QTOPIA_NO_OPENVPN

#include <QFontMetrics>
#include <QPainter>
#include <QSettings>
#include <QDebug>

#include <qdocumentselector.h>
#include <qtopiaapplication.h>
#include <ipvalidator.h>

#include <qsslcertificate.h>
#include <qsslkey.h>
/*!
  \internal
  \class GeneralOpenVPNPage

  */
GeneralOpenVPNPage::GeneralOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "GeneralOpenVPNPage" );
    ui.setupUi( this );

    connect( ui.remote, SIGNAL(editingFinished()), this, SLOT(forceRemoteName()) );
}

GeneralOpenVPNPage::~GeneralOpenVPNPage()
{
}

void GeneralOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    ui.name->setText( cfg.value("Info/Name").toString() );
    ui.description->insertPlainText( cfg.value("Info/Description").toString() );

    cfg.beginGroup( QLatin1String("Properties") );
    ui.remote->setText( cfg.value( QLatin1String("Remote") ).toString() );

    ui.ping->setValue( cfg.value( "Ping", 10 ).toInt() );
    ui.pingRestart->setValue( cfg.value( "PingRestart", 60 ).toInt() );
}

void GeneralOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.setValue( "Info/Name", ui.name->text() );
    cfg.setValue( "Info/Description", ui.description->toPlainText() );

    cfg.beginGroup( "Properties" );
    cfg.setValue( "Remote", ui.remote->text() );
    cfg.setValue( "Ping", ui.ping->value() );
    cfg.setValue( "PingRestart", ui.pingRestart->value() );
}

void GeneralOpenVPNPage::forceRemoteName()
{
    if ( ui.remote->text().isEmpty() ) {
        QMessageBox::warning( this, tr("Remote peer"),
               tr("<qt>You must enter the address of the VPN peer.</qt>"), QMessageBox::Ok, QMessageBox::NoButton );

        ui.remote->setEditFocus( true );
    }
}

/*!
  \internal
  \class CertificateOpenVPNPage

  */

CertificateOpenVPNPage::CertificateOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "CertificateOpenVPNPage" );
    ui.setupUi( this );
    connect( ui.authentication, SIGNAL(currentIndexChanged(int)),
            this, SLOT(authenticationChanged(int)) );
    connect( ui.secretKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.certKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.privKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.caKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
    connect( ui.authKey, SIGNAL(clicked()), this, SLOT(selectFile()) );
}

CertificateOpenVPNPage::~CertificateOpenVPNPage()
{
}

static QString certDescription( const QContent& c )
{
#ifndef QT_NO_OPENSSL
    QFile file( c.fileName() );
    bool res = file.open(QIODevice::ReadOnly);
    if ( !res ) 
        return CertificateOpenVPNPage::tr("Unknown certificate");
        //return (c.name() + QLatin1String("<br>") + CertificateOpenVPNPage::tr("Unknown certificate"));

    QSslCertificate cert( &file );
    if ( cert.isNull() )
        return CertificateOpenVPNPage::tr("Unknown certificate");
        //return (c.name() + QLatin1String("<br>") + CertificateOpenVPNPage::tr("Unknown certificate"));
    QString result = CertificateOpenVPNPage::tr("(%1, %2)", "e.g. %1=company %2=location");
    result = result.arg( cert.issuerInfo( QSslCertificate::Organization )).
        arg( cert.issuerInfo( QSslCertificate::LocalityName ));
    return result;
#else
    if ( c.isValid() )
        return c.name();
    else
        return CertificateOpenVPNPage::tr("Invalid");
#endif //QT_NO_OPENSSL
}

static QString keyDescription( const QContent& c )
{
#ifndef QT_NO_OPENSSL
    QFile file( c.fileName() );
    bool res = file.open(QIODevice::ReadOnly);
    if ( !res )
        return CertificateOpenVPNPage::tr("Unknown key");
            
    struct ParameterLookup {
        QSsl::KeyAlgorithm alg;
        QSsl::EncodingFormat enc;
        QSsl::KeyType type;
    }; 

    static const int paramCombinations = 8;
    static const ParameterLookup l[paramCombinations] = {
        {QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey},
        {QSsl::Dsa, QSsl::Pem, QSsl::PrivateKey},
        {QSsl::Rsa, QSsl::Der, QSsl::PrivateKey},
        {QSsl::Dsa, QSsl::Der, QSsl::PrivateKey},
        {QSsl::Rsa, QSsl::Pem, QSsl::PublicKey},
        {QSsl::Dsa, QSsl::Pem, QSsl::PublicKey},
        {QSsl::Rsa, QSsl::Der, QSsl::PublicKey},
        {QSsl::Dsa, QSsl::Der, QSsl::PublicKey},
    }; 
   
    const ParameterLookup *iter = l;
    QByteArray data = file.readAll();
    QSslKey match;
    for ( int i = 0; i<paramCombinations; i++ ) {
        QSslKey k( data, iter->alg, iter->enc, iter->type );
        if ( !k.isNull() ) {
            match = k;
            break;
        }
    }

    if ( match.isNull() )
        return CertificateOpenVPNPage::tr( "Unknown key" );
    
    QString result( CertificateOpenVPNPage::tr("%1, %2 bit", "e.g.: RSA, 1024 bit") );
    result = result.arg( match.algorithm() == QSsl::Rsa ? 
            CertificateOpenVPNPage::tr("RSA") : CertificateOpenVPNPage::tr("DSA") );
    result = result.arg( match.length() ) ;
    return result;
#else
    if ( c.isValid() )
        return c.name();
    else
        return CertificateOpenVPNPage::tr("Invalid");
#endif
}

void CertificateOpenVPNPage::init()
{
    toDocument.clear();
    QSettings cfg(config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties") );
    QString temp = cfg.value( QLatin1String("Authentication"), QLatin1String("TLS") ).toString();

    if ( temp == QLatin1String("None") ) {
        ui.authentication->setCurrentIndex( 0 );
    } else if ( temp == QLatin1String("Static") ) {
        ui.authentication->setCurrentIndex( 1 );
    } else { //SSL/TLS
        ui.authentication->setCurrentIndex( 2 );
    }
    authenticationChanged( ui.authentication->currentIndex() );

    static const short numButtons = 5;
    static const QString keys[numButtons] = { "Secret", "Certificate", "PrivKey", "CA", "TLSauth" };
    QToolButton *const buttons[numButtons] = { ui.secretKey, ui.certKey, ui.privKey, ui.caKey, ui.authKey };
    QLabel *const labels[numButtons] = {ui.secretLabel, ui.certLabel, ui.privLabel, ui.caLabel, ui.authLabel };

    QIcon normal( ":icon/padlock");
    QPixmap crossed( ":icon/padlock" );

    QPainter p;
    QPen pen( Qt::red );
    pen.setWidth( 2 );
    p.begin( &crossed );
    p.setPen( pen );
    p.drawLine( 0, 0, crossed.width(), crossed.height() ); 
    p.drawLine( crossed.width(), 0, 0, crossed.height() ); 
    p.end();
    
    for (int i = 0; i< numButtons; ++i ) {
        temp = cfg.value( keys[i] ).toString();
       
        QContent c( temp );
        toDocument.insert( buttons[i], c );
        if ( !c.isValid( true ) ) {
            buttons[i]->setIcon( normal );
            labels[i]->setText( tr("<None>", "no certificate/key selected") );
        } else {
            if ( buttons[i] == ui.certKey || buttons[i] == ui.caKey )
                labels[i]->setText( certDescription( c ) );
            else
                labels[i]->setText( keyDescription( c ) );
            buttons[i]->setIcon( crossed );
        }
    }
}

void CertificateOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    switch( ui.authentication->currentIndex() ) {
        case 0: //none
            cfg.setValue( QLatin1String("Authentication"), QLatin1String("None") );
            break;
        case 1: //static key
            {
                cfg.setValue( QLatin1String("Authentication"), QLatin1String("Static") );
                QContent doc = toDocument[ui.secretKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("Secret"), doc.fileName() );
            }
            break;
        default:
        case 2: //SSL/TLS
            {
                cfg.setValue( QLatin1String("Authentication"), QLatin1String("TLS") );
                QContent doc = toDocument[ui.certKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories()<< QLatin1String("Certificate") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("Certificate"), doc.fileName() );

                doc = toDocument[ui.privKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("PrivKey"), doc.fileName() );

                doc = toDocument[ui.caKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Certificate") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("CA"), doc.fileName() );

                doc = toDocument[ui.authKey];
                if ( doc.isValid() ) {
                    doc.setCategories( doc.categories() << QLatin1String("Security key") );
                    doc.commit();
                }
                cfg.setValue( QLatin1String("TLSauth"), doc.fileName() );
            }
                break;
    }
}

void CertificateOpenVPNPage::authenticationChanged( int idx )
{
    ui.certFrame->setVisible( idx == 2 );
    ui.secretFrame->setVisible( idx == 1 );
}


void CertificateOpenVPNPage::selectFile()
{
    QToolButton* b = qobject_cast<QToolButton*>(sender());
    if ( !b )
        return;

    //create icons
    QIcon normal( ":icon/padlock");
    QPixmap crossed( ":icon/padlock" );

    QPainter p;
    QPen pen( Qt::red );
    pen.setWidth( 2 );
    p.begin( &crossed );
    p.setPen( pen );
    p.drawLine( 0, 0, crossed.width(), crossed.height() ); 
    p.drawLine( crossed.width(), 0, 0, crossed.height() ); 
    p.end();
 
    QMap<QToolButton*,QLabel*> btnToLabel;
    btnToLabel.insert( ui.secretKey, ui.secretLabel );
    btnToLabel.insert( ui.certKey, ui.certLabel );
    btnToLabel.insert( ui.privKey, ui.privLabel );
    btnToLabel.insert( ui.caKey, ui.caLabel );
    btnToLabel.insert( ui.authKey, ui.authLabel );

    QLabel* label = btnToLabel[b];
 
    QContent doc = toDocument[b];
    if ( doc.isValid() ) {
        //unselect
        toDocument.insert( b, QContent() );
        b->setIcon( normal );
        label->setText( tr("<None>","no certificate/key selected") );
        return;
    }
    
    QDocumentSelectorDialog dlg;
    dlg.setModal( true );

    QContentFilter filter;

    bool certificate = false;
    bool secKey = false;

    if ( b == ui.certKey || b == ui.caKey )
        certificate = true;
    else
        secKey = true;

    dlg.setFilter( QContentFilter( QContent::Document ) );
    dlg.setDefaultCategories( QStringList() << "Certificate" << "Security Key" );


    if ( QtopiaApplication::execDialog( &dlg ) == QDialog::Accepted ) {
        QContent doc = dlg.selectedDocument();

        if ( !doc.isValid() ) {
            b->setIcon( normal );
            btnToLabel[b]->setText( tr("<None>","no certificate/key selected") );
            return;
        }

        if ( b == ui.caKey || b == ui.certKey )
            label->setText( certDescription( doc ) );
        else
            label->setText( keyDescription( doc ) );
        b->setIcon( crossed );
        toDocument.insert( b, doc );
    }
}

/*!
  \internal
  \class OptionsOpenVPNPage

  */

OptionsOpenVPNPage::OptionsOpenVPNPage( QWidget* parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "OptionsOpenVPNPage" );
    ui.setupUi( this );

    connect( ui.configFile, SIGNAL(clicked()), this, SLOT(selectConfigScript()) );
}

OptionsOpenVPNPage::~OptionsOpenVPNPage()
{
}

void OptionsOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    ui.LZO->setCheckState( cfg.value(QLatin1String("LZO"), false).toBool() ? Qt::Checked : Qt::Unchecked );
    ui.pull->setCheckState( cfg.value(QLatin1String("Pull"), false).toBool() ? Qt::Checked : Qt::Unchecked );

    ui.exitNotify->setEnabled(
        cfg.value( QLatin1String("Protocol"), "udp" ).toString() == QLatin1String("udp") );

    ui.exitNotify->setValue( cfg.value(QLatin1String("ExitNotification"), 2).toInt() );
    //ui.verbosity->setValue( cfg.value(QLatin1String("Verbosity"), 4).toInt() );
    //ui.mute->setValue( cfg.value(QLatin1String("Mute"), 10).toInt() );

    QString file = cfg.value(QLatin1String("ConfigScript")).toString();
    if ( !file.isEmpty() )
        configScript = QContent( file );
}

void OptionsOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    cfg.setValue(QLatin1String("LZO"), ui.LZO->checkState() == Qt::Checked );
    cfg.setValue(QLatin1String("Pull"), ui.pull->checkState() == Qt::Checked );

    cfg.setValue( QLatin1String("ExitNotification"), ui.exitNotify->value() );
    //cfg.setValue( QLatin1String("Verbosity"), ui.verbosity->value() );
    //cfg.setValue( QLatin1String("Mute"), ui.mute->value() );

    cfg.setValue( QLatin1String("ConfigScript"), configScript.isValid() ? configScript.fileName(): QString() );
}

void OptionsOpenVPNPage::selectConfigScript()
{
    QDocumentSelectorDialog dlg;
    dlg.setModal( true );

    QContentFilter filter;

    if ( QtopiaApplication::execDialog( &dlg ) == QDialog::Accepted ) {
        QContent doc = dlg.selectedDocument();

        if ( !doc.isValid() ) {
            ui.configFile->setText( tr("Select...") );
            configScript = QContent();
            return;
        }

        configScript = doc;
        QFont f = ui.configFile->font();
        QFontMetrics fm( ui.configFile->font() );
        ui.configFile->setText( fm.elidedText(doc.name(), Qt::ElideRight, ui.configFile->size().width() ) );
    } else {
        //TODO currently there is no way of unselecting a document
        //for now the user has to open the document selector and must press Cancel
        //in order to unselect the document
        ui.configFile->setText(tr("Select..."));
        configScript = QContent();
    }
}

/*!
  \internal
  \class DeviceOpenVPNPage

  */

DeviceOpenVPNPage::DeviceOpenVPNPage( QWidget * parent )
    : VPNConfigWidget( parent )
{
    setObjectName( "OptionsOpenVPNPage" );
    ui.setupUi( this );

    IPValidator* val = new IPValidator( this );
    ui.localIP->setValidator( val );
    ui.remoteIP->setValidator( val );
    QtopiaApplication::setInputMethodHint( ui.localIP, QLatin1String("netmask") );
    QtopiaApplication::setInputMethodHint( ui.remoteIP, QLatin1String("netmask") );

    ui.device->addItem( QLatin1String("tap") ); //no tr
    ui.device->addItem( QLatin1String("tun") ); //no tr
    connect( ui.device, SIGNAL(currentIndexChanged(int)), this, SLOT(resetRemoteLabel(int)) );
}

DeviceOpenVPNPage::~DeviceOpenVPNPage()
{
}

void DeviceOpenVPNPage::init()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties") );

    QString temp = cfg.value("Device", "tap" ).toString();
    ui.device->setCurrentIndex( temp == "tap" ? 0: 1 );
    resetRemoteLabel( ui.device->currentIndex() );
    temp = cfg.value("Protocol", "udp" ).toString();
    if ( temp == QLatin1String("tcp-client") )
        ui.protocol->setCurrentIndex( 1 );
    else if ( temp == QLatin1String("tcp-server") )
        ui.protocol->setCurrentIndex( 2 );
    else
        ui.protocol->setCurrentIndex( 0 );

    ui.port->setValue( cfg.value( "Port", 1194 ).toInt() );
    ui.localIP->setText( cfg.value( QLatin1String("LocalIP") ).toString() );
    ui.remoteIP->setText( cfg.value( QLatin1String("RemoteIP") ).toString() );
}

void DeviceOpenVPNPage::resetRemoteLabel( int newDevType )
{
    //see man openvpn for different types of --ifconfig parameter when using tun/tap device
    switch( newDevType ) {
        case 0: //tap device
            ui.remoteIPLabel->setText( tr("Subnet mask:") );
            break;
        case 1: //tun device
            ui.remoteIPLabel->setText( tr("Remote IP:") );
            break;
        default:
            qWarning() << "DeviceOpenVPNPage::resetRemoteLabel: Unknown device type";
   }
}

void DeviceOpenVPNPage::save()
{
    QSettings cfg( config, QSettings::IniFormat );
    cfg.beginGroup( QLatin1String("Properties" ) );

    int idx = ui.device->currentIndex();
    cfg.setValue( QLatin1String("Device"), idx == 0 ? QLatin1String("tap") : QLatin1String("tun") );
    idx = ui.protocol->currentIndex();
    QString prot = "udp";
    if ( idx == 1 )
        prot = "tcp-client";
    else if ( idx == 2 )
        prot = "tcp-server";
    cfg.setValue( "Protocol", prot );

    cfg.setValue( "Port", ui.port->value() );
    cfg.setValue( QLatin1String("LocalIP"), ui.localIP->text() );
    cfg.setValue( QLatin1String("RemoteIP"), ui.remoteIP->text() );

}
#endif //QTOPIA_NO_OPENVPN
