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

#include "mmseditaccount.h"
#include <private/accountconfiguration_p.h>

#include <qtopiaapplication.h>
#include <qtopiaservices.h>
#include <QWapAccount>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QMailAccount>

MmsEditAccount::MmsEditAccount(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("mms-account");
    setupUi(this);
    connect(networkBtn, SIGNAL(clicked()), this, SLOT(configureNetworks()));
    QtopiaIpcAdaptor* netChannel = new QtopiaIpcAdaptor("QPE/NetworkState", this);
    QtopiaIpcAdaptor::connect(netChannel, MESSAGE(wapChanged()),
            this, SLOT(updateNetwork()));
}

void MmsEditAccount::populateNetwork()
{
    // Find available configs.
    QString path = Qtopia::applicationFileName("Network", "wap");
    QDir configDir(path);
    configDir.mkdir(path);

    QStringList files = configDir.entryList( QStringList("*.conf") );
    QStringList configList;
    foreach( QString item, files ) {
        configList.append( configDir.filePath( item ) );
    }

    // Get default
    QSettings cfg("Trolltech", "Network");
    cfg.beginGroup("WAP");
    QString defaultWap = cfg.value("DefaultAccount").toString();
    cfg.endGroup();
    int defaultConfig = -1;

    // Add to combo
    networkCombo->clear();
    foreach( const QString &netConfig, configList ) {
        QWapAccount acc( netConfig );
        networkCombo->addItem(QIcon(":icon/netsetup/wap"), acc.name(), netConfig);
        if ( netConfig == defaultWap ) {
            defaultConfig = networkCombo->count()-1;
        }
        if ( netConfig == config->networkConfig() ) {
            networkCombo->setCurrentIndex(networkCombo->count()-1);
        }
    }

    if (networkCombo->currentIndex() == -1 && defaultConfig >= 0)
        networkCombo->setCurrentIndex(defaultConfig);

    if (!networkCombo->count()) {
        networkCombo->addItem(tr("<None configured>", "No network profiles have been configured"));
        networkCombo->setCurrentIndex(0);
    }
}

void MmsEditAccount::setAccount(QMailAccount *in, AccountConfiguration *conf)
{
    account = in;
    config = conf;
    populateNetwork();
    autoRetrieve->setChecked(config->isAutoDownload());
}

void MmsEditAccount::accept()
{
    int currItem = networkCombo->currentIndex();
    if (currItem >= 0 && networkCombo->itemData(currItem).isValid()) {
        config->setNetworkConfig(networkCombo->itemData(currItem).toString());
    } else {
        config->setNetworkConfig(QString());
    }
    config->setAutoDownload(autoRetrieve->isChecked());
    QDialog::accept();
}

void MmsEditAccount::configureNetworks()
{
    QtopiaServiceRequest serv("NetworkSetup", "configureWap()");
    serv.send();
}

void MmsEditAccount::updateNetwork()
{
    populateNetwork();
}

