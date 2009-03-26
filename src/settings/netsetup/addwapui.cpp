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

#include "addwapui.h"

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QRegExp>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>

#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtopianetworkinterface.h>
#include <qsoftmenubar.h>

#include "ui_gatewaybase.h"
#include "ui_mmsbase.h"
#include "ui_browserbase.h"

enum PageEnum {
   Account, WAPGateway, MMS, Browsing
} Page;


class WapAccountPage : public QWidget
{
Q_OBJECT
public:
    WapAccountPage( QWapAccount* acc, QWidget* parent = 0, Qt::WFlags fl = 0)
        : QWidget( parent, fl ), wapAccount( acc )
    {
        Q_ASSERT(wapAccount);
        QVBoxLayout* vb = new QVBoxLayout( this );
        vb->setMargin( 5 );
        vb->setSpacing( 4 );

        QLabel * account_label = new QLabel( tr("Name:"), this );
        vb->addWidget( account_label );

        account = new QLineEdit( this );
        vb->addWidget( account );
        account_label->setBuddy( account );

        QLabel * dataAccount_label = new QLabel(tr("Use data account:"), this);
        vb->addWidget(dataAccount_label);

        dataAccount = new QComboBox( this );
        vb->addWidget( dataAccount );
        dataAccount_label->setBuddy( dataAccount );

        QSpacerItem* spacer = new QSpacerItem( 10, 100,
                QSizePolicy::Minimum, QSizePolicy::Expanding );
        vb->addItem( spacer );

        if ( wapAccount->name().isEmpty() ) {
            wapAccount->setName( tr("WAP account", "name of default wap account") );
            account->setText( tr("WAP account", "name of default wap account") );
        } else {
            account->setText( wapAccount->name() );
        }

        dataInterfaces = QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::GPRS );

        int found = -1;
        for( int i = 0; i<dataInterfaces.count(); i++ ) {
            QSettings dataCfg( dataInterfaces[i], QSettings::IniFormat );
            dataAccount->addItem( dataCfg.value("Info/Name").toString() );
            if (dataInterfaces[i] == wapAccount->dataInterface())
                found = i;
        }

        if ( dataAccount->count() == 0 ) {
            dataAccount->setEnabled( false );
            dataAccount->addItem( tr("<No GPRS account>") );
            dataAccount->setCurrentIndex( 0 );
        } else {
            dataAccount->setCurrentIndex( found );
        }

        QtopiaApplication::setInputMethodHint( account, QtopiaApplication::Words );
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
        setObjectName("wap-account");

        connect( account, SIGNAL(editingFinished()), this, SLOT(nameChanged()) );
        connect( dataAccount, SIGNAL(currentIndexChanged(int)), this, SLOT(dataInterfaceChanged(int)) );
    }

    virtual ~WapAccountPage()
    {
        wapAccount = 0; //pointer owned by AddWapUI
    }

private slots:

    void nameChanged()
    {
        wapAccount->setName( account->text() );
    }

    void dataInterfaceChanged( int idx )
    {
        if (idx >= 0 && idx < dataAccount->count() && dataInterfaces.count() )
            wapAccount->setDataInterface( dataInterfaces[idx] );
        else
            wapAccount->setDataInterface( "" );
    }

private:
    QStringList dataInterfaces;
    QLineEdit* account;
    QComboBox* dataAccount;
    QWapAccount* wapAccount;
};

class GatewayPage: public QWidget
{
    Q_OBJECT
public:
    GatewayPage( QWapAccount* acc, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl ), wapAccount( acc )
    {
        Q_ASSERT(wapAccount);
        ui.setupUi( this );
        QtopiaApplication::setInputMethodHint( ui.gateway, "url" );
        QtopiaApplication::setInputMethodHint( ui.username, "text noautocapitalization" );
        // automatically sets the password hint modifier
        ui.password->setEchoMode( QLineEdit::PasswordEchoOnEdit );

        QUrl url = wapAccount->gateway();
        ui.gateway->setText( url.host() );
        ui.port->setValue( url.port() == 9200 ? 0 : url.port() );
        ui.username->setText( url.userName() );
        ui.password->setText( url.password() );
        ui.password->setEchoMode( QLineEdit::PasswordEchoOnEdit );

        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
        setObjectName("wap-gateway");

        connect( ui.gateway, SIGNAL(editingFinished()), this, SLOT(gatewayChanged()) );
        connect( ui.username, SIGNAL(editingFinished()), this, SLOT(gatewayChanged()) );
        connect( ui.password, SIGNAL(editingFinished()), this, SLOT(gatewayChanged()) );

        connect( ui.port, SIGNAL(valueChanged(int)), this, SLOT(gatewayChanged()) );
    };
private slots:
    void gatewayChanged()
    {
        QUrl url;
        url.setHost( ui.gateway->text() );
        url.setPort( ui.port->value() == 0 ? 9200 : ui.port->value() );
        url.setUserName( ui.username->text() );
        url.setPassword( ui.password->text() );
        wapAccount->setGateway( url );
    }

private:
    Ui::GatewayBase ui;
    QWapAccount* wapAccount;
};

struct ExpiryTime {
    int expiry;
    const char *desc;
};

static const ExpiryTime expiryTimes[] = {
    { 0, QT_TRANSLATE_NOOP("MMSPage", "Maximum") },
    { 1, QT_TRANSLATE_NOOP("MMSPage", "1 Hour") },
    { 2, QT_TRANSLATE_NOOP("MMSPage", "2 Hours") },
    { 6, QT_TRANSLATE_NOOP("MMSPage", "6 Hours") },
    { 12, QT_TRANSLATE_NOOP("MMSPage", "12 Hours") },
    { 24, QT_TRANSLATE_NOOP("MMSPage", "1 Day") },
    { 48, QT_TRANSLATE_NOOP("MMSPage", "2 Days") },
    { 72, QT_TRANSLATE_NOOP("MMSPage", "3 Days") },
    { 0, 0 }
};


class MMSPage: public QWidget
{
    Q_OBJECT
public:
    MMSPage( QWapAccount* acc, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl ), wapAccount( acc )
    {
        Q_ASSERT( wapAccount );
        ui.setupUi( this );
        QtopiaApplication::setInputMethodHint( ui.mms, QtopiaApplication::Text );

        QUrl url( wapAccount->mmsServer() );
        
        QString host( url.toString( QUrl::RemovePort ) );
        if ( host.startsWith( "http://" ) )
            host.remove(0, 7);
        if ( host.startsWith( "//" ) )
            host.remove(0, 2);
        ui.mms->setText( host );

        int port = url.port();
        ui.mmsPort->setValue( port == 80 ? 0 : port );

        int idx = 0;
        while (expiryTimes[idx].desc) {
            ui.expiry->addItem(qApp->translate("MMSPage", expiryTimes[idx].desc));
            idx++;
        }

        idx = 0;
        int exp = wapAccount->mmsExpiry();
        while (expiryTimes[idx].desc && expiryTimes[idx].expiry < exp)
            idx++;
        if (!expiryTimes[idx].desc)
            idx = 0;
        ui.expiry->setCurrentIndex(idx);

        switch( wapAccount->mmsSenderVisibility() ) {
            case QWapAccount::SenderHidden:
                ui.visibility->setCurrentIndex( 2 );
                break;
            case QWapAccount::SenderVisible:
                ui.visibility->setCurrentIndex( 1 );
                break;
            default:
                ui.visibility->setCurrentIndex( 0 );
                break;
        }

        ui.delivery->setCheckState( wapAccount->mmsDeliveryReport() ? Qt::Checked : Qt::Unchecked );

        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
        setObjectName("wap-mms");

        connect( ui.mms, SIGNAL(editingFinished()), this, SLOT(mmsServerChanged()) );
        connect( ui.mmsPort, SIGNAL(valueChanged(int)), this, SLOT(mmsServerChanged()) );
        connect( ui.expiry, SIGNAL(currentIndexChanged(int)), this, SLOT(expiryChanged(int)) );
        connect( ui.visibility, SIGNAL(currentIndexChanged(int)), this, SLOT(visibilityChanged(int)) );
        connect( ui.delivery, SIGNAL(stateChanged(int)), this, SLOT(deliveryChanged(int)) );
    };

private slots:

    void mmsServerChanged()
    {
        QUrl url;

        // Parse the input: 
        QRegExp elements( "(?:(\\w+)://)?"  // optional scheme info
                          "([^:/]+)"        // mandatory host component
                          "(?::(\\d+))?"    // optional port specifier
                          "(?:/(.+))?" );   // optional path information

        if ( elements.indexIn( ui.mms->text() ) != -1 ) {
            QString scheme( elements.cap(1) );
            QString host( elements.cap(2) );
            QString port( elements.cap(3) );
            QString path( elements.cap(4) );

            if ( !scheme.isEmpty() )
                url.setScheme( scheme );
            if ( !host.isEmpty() )
                url.setHost( host );
            if ( !port.isEmpty() )
                url.setPort( port.toInt() );
            if ( !path.isEmpty() )
                url.setPath( path );
        }

        // If the port is specified individually, override any existing port value
        int port( ui.mmsPort->value() );
        if ( port != 0 )
            url.setPort( port );

        wapAccount->setMmsServer( url );
    }

    void expiryChanged( int idx )
    {
        if ( idx < 0 )
            return;
        wapAccount->setMmsExpiry( expiryTimes[idx].expiry );
    }

    void visibilityChanged( int idx )
    {
        if ( idx < 0 )
            return;
        switch (idx) {
            case 0:
            default:
                wapAccount->setMmsSenderVisibility( QWapAccount::Default );
                break;
            case 1:
                wapAccount->setMmsSenderVisibility( QWapAccount::SenderVisible );
                break;
            case 2:
                wapAccount->setMmsSenderVisibility( QWapAccount::SenderHidden );
                break;
        }
    }

    void deliveryChanged( int state )
    {
        wapAccount->setMmsDeliveryReport( state == Qt::Checked ? true : false );
    }
private:
    QWapAccount* wapAccount;
    Ui::MMSBase ui;
};

class BrowserPage : public QWidget
{
    Q_OBJECT
public:
    BrowserPage( QString file, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl ), cfg( file, QSettings::IniFormat )
    {
        ui.setupUi( this );

        if ( cfg.value("Browser/ShowPictures").toString() != "y" )
            ui.showPics->setCheckState(Qt::Unchecked);
        else
            ui.showPics->setCheckState( Qt::Checked );

        btnGrp = new QButtonGroup( this );
        btnGrp->addButton( ui.confirm );
        btnGrp->addButton( ui.accept );
        btnGrp->addButton( ui.reject );
        QString cookies = cfg.value("Browser/Cookies").toString();
        if ( cookies == "Confirm" )
            ui.confirm->setChecked( true );
        else if ( cookies == "Accept" )
            ui.accept->setChecked( true );
        else
            ui.reject->setChecked( true );
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
        setObjectName("wap-browser");

        connect( ui.showPics, SIGNAL(stateChanged(int)), this, SLOT(picsChanged()) );
        connect( btnGrp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(cookiesChanged()) );
    }

private slots:

    void picsChanged()
    {
       if ( ui.showPics->checkState() == Qt::Checked )
           cfg.setValue("Browser/ShowPictures", "y");
       else
           cfg.setValue("Browser/ShowPictures", "n");
    }

    void cookiesChanged()
    {
        if ( ui.confirm->isChecked() )
            cfg.setValue( QLatin1String("Browser/Cookies"), "Confirm" );
        else if ( ui.accept->isChecked() )
            cfg.setValue( QLatin1String("Browser/Cookies"), "Accept" );
        else
            cfg.setValue( QLatin1String("Browser/Cookies"), "Reject" );
    }

private:
    QButtonGroup* btnGrp;
    QSettings cfg;
    Ui::BrowserBase ui;
};



AddWapUI::AddWapUI( const QString& file, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), acc( file )
{
    setModal( true );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
    setObjectName("wap-menu");

    init();
}

AddWapUI::~AddWapUI()
{
}

void AddWapUI::init()
{
    setWindowTitle( tr("WAP") );
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );

    stack = new QStackedWidget( this );

    QWidget* page = new QWidget();
    QVBoxLayout *vb = new QVBoxLayout(page);
    vb->setMargin( 2 );
    vb->setSpacing( 2 );
    options = new QListWidget( page );
    options->setSpacing( 1 );
    options->setAlternatingRowColors( true );
    options->setSelectionBehavior( QAbstractItemView::SelectRows );
    QListWidgetItem* item = new QListWidgetItem( QIcon(":icon/account"),
            tr("Account"), options, Account );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( QIcon(":icon/wap"),
            tr("WAP Gateway"), options,  WAPGateway );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( QIcon(":icon/mms"),
            tr("MMS"), options, MMS );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( QIcon(":icon/settings"), 
            tr("Misc"), options, Browsing );
    item->setTextAlignment( Qt::AlignHCenter);
    vb->addWidget( options );

    QHBoxLayout* hb = new QHBoxLayout();
    hint = new QLabel( page );
    hint->setWordWrap( true );
    hint->setMargin( 2 );
    hb->addWidget( hint );

    QSpacerItem* spacer = new QSpacerItem( 1, 60, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );
    hb->addItem( spacer );

    vb->addLayout( hb );

    connect( options, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateUserHint(QListWidgetItem*,QListWidgetItem*)));

    options->setCurrentRow( 0 );
    stack->addWidget( page );

    QScrollArea* scroll = 0;

    accountPage = new WapAccountPage( &acc );
    stack->addWidget( accountPage );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    gatewayPage = new GatewayPage( &acc );
    scroll->setWidget( gatewayPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    mmsPage = new MMSPage( &acc );
    scroll->setWidget( mmsPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    browserPage = new BrowserPage( acc.configuration() );
    scroll->setWidget( browserPage );
    stack->addWidget( scroll );

    stack->setCurrentIndex( 0 );

    mainLayout->addWidget( stack );
    connect(options, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(optionSelected(QListWidgetItem*)));
}

void AddWapUI::accept()
{
    if ( stack->currentIndex() == 0 ) {
        QDialog::accept();
    } else {
        stack->setCurrentIndex( 0 );
    }
}

void AddWapUI::optionSelected(QListWidgetItem* item)
{
    if (item) {
        switch( item->type() )
        {
            case Account:
                stack->setCurrentIndex( 1 );
                break;
            case MMS:
                stack->setCurrentIndex( 3 );
                break;
            case Browsing:
                stack->setCurrentIndex( 4 );
                break;
            case WAPGateway:
                stack->setCurrentIndex( 2 );
                break;
            default:
                break;
        }
    }
}


void AddWapUI::updateUserHint( QListWidgetItem* newItem, QListWidgetItem* /*prev*/ )
{
    if (!newItem)
        return;

    QString text;
    switch( newItem->type() ) {
        case Account:
            text = tr("General account information.");
            break;
        case MMS:
            text = tr("MMS server and message details.");
            break;
        case Browsing:
            text = tr("General browser settings");
            break;
        case WAPGateway:
            text = tr("WAP server login details.");
            break;
        default:
            break;
    }
    hint->setText( text );
}

#include "addwapui.moc"

