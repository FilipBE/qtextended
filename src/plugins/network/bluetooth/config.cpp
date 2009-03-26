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

#include "config.h"
#include "configui.h"

#include <QFile>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QScrollArea>
#include <QStackedWidget>

#include <qsoftmenubar.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qbluetoothnamespace.h>
#include <accountconfig.h>

#include <unistd.h>
#include <time.h>

using namespace QBluetooth;

class BluetoothUI : public QDialog
{
    Q_OBJECT
public:
    BluetoothUI( BluetoothConfig* config, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~BluetoothUI();

    enum UIPage {
        Account, Dialing, Advanced
    };

public slots:
    void accept();

private:
    void init();
    void createPeerId();
    int writeSystemFiles();


private slots:
    void optionSelected( QListWidgetItem* item );
    void updateUserHint( QListWidgetItem* cur, QListWidgetItem* prev );

private:
    BluetoothConfig* config;
    QStackedWidget* stack;
    QListWidget* options;
    QLabel* userHint;

    DialingBTPage* dialPage;
    AdvancedBTPage* advPage;
    AccountPage* accPage;
};

//BluetoothUI
BluetoothUI::BluetoothUI( BluetoothConfig* bconfig, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), config( bconfig )
{
    init();
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
    setObjectName("bluetooth-dun");
}

BluetoothUI::~BluetoothUI()
{
}

void BluetoothUI::init()
{
    QVBoxLayout* vBox = new QVBoxLayout( this );
    vBox->setMargin( 0 );
    vBox->setSpacing( 0 );

    QtopiaNetworkProperties knownProp = config->getProperties();

    QString title = knownProp.value("Info/Name").toString();
    if (!title.isEmpty())
        setWindowTitle( title );

    stack = new QStackedWidget( this );

    QWidget* page = new QWidget();
    QVBoxLayout *vb = new QVBoxLayout(page);
    vBox->setMargin( 0 );
    vBox->setSpacing( 0 );
    options = new QListWidget( page );
    options->setSpacing( 1 );
    options->setAlternatingRowColors( true );
    options->setSelectionBehavior( QAbstractItemView::SelectRows );

    QListWidgetItem* item = new QListWidgetItem(tr("Account"), options, Account );
    item->setTextAlignment( Qt::AlignHCenter);
    item->setIcon( QIcon(":icon/netsetup/account") );

    item = new QListWidgetItem( tr("Dial parameter" ), options,  Dialing );
    item->setTextAlignment( Qt::AlignHCenter);
    item->setIcon( QIcon(":icon/netsetup/server") );

    item = new QListWidgetItem( tr("Advanced", "advanced settings"), options, Advanced );
    item->setTextAlignment( Qt::AlignHCenter);
    item->setIcon(QIcon(":icon/netsetup/proxies"));

    vb->addWidget( options );

    QHBoxLayout* hBox = new QHBoxLayout();
    userHint = new QLabel( page);
    userHint->setWordWrap( true );
    userHint->setMargin( 2 );
    hBox->addWidget( userHint );

    QSpacerItem* spacer = new QSpacerItem( 1, 60,
            QSizePolicy::Minimum, QSizePolicy::Expanding );
    hBox->addItem( spacer );

    vb->addLayout( hBox );

    connect( options, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateUserHint(QListWidgetItem*,QListWidgetItem*)));
    options->setCurrentRow( 0 );

    stack->addWidget( page );

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    accPage = new AccountPage( QtopiaNetwork::BluetoothDUN, knownProp );
    scroll->setWidget( accPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    dialPage = new DialingBTPage( knownProp );
    scroll->setWidget( dialPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    advPage = new AdvancedBTPage( knownProp );
    scroll->setWidget( advPage );
    stack->addWidget( scroll );

    stack->setCurrentIndex( 0 );
    vBox->addWidget( stack );
    connect(options, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(optionSelected(QListWidgetItem*)));
}

void BluetoothUI::optionSelected( QListWidgetItem* item )
{
    if ( item ) {
        switch( item->type() )
        {
            case Account:
                stack->setCurrentIndex( 1 );
                break;
            case Dialing:
                stack->setCurrentIndex( 2 );
                break;
            case Advanced:
                stack->setCurrentIndex( 3 );
                break;
            default:
                break;
        }
    }
}

void BluetoothUI::updateUserHint( QListWidgetItem* cur, QListWidgetItem* /*prev*/ )
{
    if ( !cur )
        return;

    QString desc;
    switch( cur->type() ) {
        case Account:
            desc = tr("General account information");
            break;
        case Dialing:
            desc = tr("General dial-up parameter.");
            break;
        case Advanced:
            desc = tr("Advanced parameter");
            break;
        default:
            break;
    }

    userHint->setText( desc );
}

void BluetoothUI::accept()
{
    if ( stack->currentIndex() == 0 ) {
        QtopiaNetworkProperties props = accPage->properties();
        config->writeProperties( props );
        props = dialPage->properties();
        config->writeProperties( props );
        props = advPage->properties();
        config->writeProperties( props );

        createPeerId();
        int retCode = writeSystemFiles();
        if ( retCode == 0 ) 
            QDialog::accept();
        else if ( retCode > 0 ) 
            stack->setCurrentIndex( retCode );
    } else {
        stack->setCurrentIndex( 0 );
    }

}

void BluetoothUI::createPeerId( )
{
    //create peer id if it doesn't already exist
    QString peerId = config->property( "Serial/PeerID" ).toString();
    if (peerId.isEmpty()) {
        //create a uniquish id
        peerId = config->property("Info/Type").toString();
        peerId.replace( QRegExp("[^A-Za-z]"), "" );
        peerId += QString::number(time(0)^(getpid()<<3));
        qLog(Network) << "Creating new peer ID: " << peerId;
        QtopiaNetworkProperties p;
        p.insert("Serial/PeerID", peerId);
        config->writeProperties( p );
    }
}

/*
  Return values:
    -1 -> general error not connected to specific configuration
    0  -> no error
    >0 -> returned number gives indication of widget that could fix the problem
  */
int BluetoothUI::writeSystemFiles()
{
    const QtopiaNetworkProperties prop = config->getProperties();

    //chat scripts
    QByteArray a;
    a = "ABORT \"NO CARRIER\"\n" // No tr
        "ABORT \"NO DIALTONE\"\n" // No tr
        "ABORT \"BUSY\"\n" // No tr
        "\"\" "; // No tr

    QString dial = prop.value( QLatin1String("Serial/ExtraDialString") ).toString();
    if ( !dial.isEmpty() ) {
        a += QLatin1String("AT");
        a += dial;
        a += QLatin1String("\nOK ");
    }

    dial = prop.value( QLatin1String("Serial/DialString") ).toString();
    if ( dial.isEmpty() ) {
        QMessageBox::warning(this, tr("Error"), "<qt>"+tr("Missing dial number")+"</qt>",
                QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        qLog(Network) << QLatin1String("Missing dial number");
        return 2; //index of dial parameter widget
    } else {
        a += QLatin1String("ATD") + dial + QLatin1String("\n");
        a += QLatin1String("CONNECT \"\"");
    }

    const QString path = Qtopia::applicationFileName("Network", "chat");
    const QString peerName = prop.value(QLatin1String("Serial/PeerID")).toString();
    QDir chatDir( path );
    chatDir.mkdir( path );
    QByteArray connectFile = path.toLatin1()+"/connect-" + peerName.toLatin1();

    QFile fc( connectFile );
    if ( fc.open(QFile::WriteOnly | QFile::Truncate) ) {
        fc.write( a );
        fc.close();
    } else {
        qLog(Network) << "Cannot write connect chat file";
        return -1;
    }

    QStringList params;

    QString tmp("connect '/usr/sbin/chat -s -v -f %1/connect-%2'");
    tmp = tmp.arg( path ).arg( peerName );
    params << tmp;
    // Most GRPS modems do not respond to LCP echo's
    params << "lcp-echo-failure 0";   // No tr
    params << "lcp-echo-interval 0"; // No tr
    // Compression does not make sense on a GPRS link
    params << "novj"; // No tr
    params <<"nobsdcomp"; // No tr
    params <<"novjccomp"; // No tr
    params <<"nopcomp"; // No tr
    params <<"noaccomp"; // No tr

    params << "crtscts";

    // Accept peers idea of our local address
    params << "ipcp-accept-local";
    params << "noipdefault";
    // Use modem control lines, otherwise change the following "modem" to "local"
    params << "modem";

    tmp = prop.value( QLatin1String("Properties/UserName"), QString() ).toString();
    if ( !tmp.isEmpty() ) {
        params << ("user " +  Qtopia::stringQuote(tmp));
    }

    tmp = prop.value( QLatin1String("Properties/Password"), QString() ).toString();
    if ( !tmp.isEmpty() ) {
        params << ("password " + Qtopia::stringQuote( tmp ));
    }

    const int idleTime = prop.value("Serial/Timeout").toInt();
    if ( idleTime )
        params << "idle " + QString::number(idleTime) ;

    params << QLatin1String("usepeerdns");
    params << QLatin1String("defaultroute");
    params << QLatin1String("connect-delay 1");
    params << ( "remotename " + peerName );

    //install peer file
    const QString peerFileName = Qtopia::tempDir() + peerName;
    QFile tmpPeer( peerFileName );
    if ( tmpPeer.open(QFile::WriteOnly | QFile::Truncate) ) {
        QTextStream s(&tmpPeer);
        for (int i = 0; i<params.count(); i++)
            s << params[i] << endl;
        tmpPeer.close();
    } else {
        fc.remove(); //delete connect chat
        return -1; //cannot write peer file
    }

    QStringList args;
    args << "install";
    args << "peer";
    args << peerFileName;

    qLog(Network) << "Saving peers file " << peerName;
    //installing the peer shouldn't take to long -> hence we can block on its execution
    QProcess::execute(Qtopia::qtopiaDir()+"bin/btdun-network", args);
    QFile::remove( peerFileName );

    return 0;
}

//BluetoothConfig
BluetoothConfig::BluetoothConfig( const QString& confFile )
    : currentConfig( confFile ), cfg( confFile, QSettings::IniFormat )
{
}

BluetoothConfig::~BluetoothConfig()
{
    //TODO
}

QStringList BluetoothConfig::types() const
{
    QStringList uis;
    uis << QObject::tr("Properties");
    return uis;
}

QVariant BluetoothConfig::property( const QString& key ) const
{
    cfg.sync();
    QVariant result;
    result = cfg.value(key);
    return result;
}

QDialog* BluetoothConfig::configure( QWidget* parent, const QString& type )
{
    if ( type.isEmpty()  || type == QObject::tr("Properties") )
        return new BluetoothUI( this, parent );
    return 0;
}

QtopiaNetworkProperties BluetoothConfig::getProperties() const
{
    QtopiaNetworkProperties prop;
    cfg.sync();

    QStringList keys = cfg.allKeys();
    foreach( QString k, keys ) {
        QVariant v = cfg.value( k );
        if ( v.isValid() )
            prop.insert( k, v );
    }
    return prop;
}

void BluetoothConfig::writeProperties( const QtopiaNetworkProperties& properties )
{
    QMapIterator<QString,QVariant> iter(properties);

    while( iter.hasNext() ) {
        iter.next();
        cfg.setValue( iter.key(), iter.value() );
    }
}

#include "config.moc"
