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

#ifndef GREENPHONEETHERNETGADGET_H
#define GREENPHONEETHERNETGADGET_H

//! [1]

#include <QUsbEthernetGadget>

class QUsbManager;

class GreenphoneEthernetGadgetProvider : public QUsbEthernetGadget
{
    Q_OBJECT

public:
    GreenphoneEthernetGadgetProvider(const QString &group = QString(), QObject *parent = 0);
    ~GreenphoneEthernetGadgetProvider();

public slots:
    void setVendorId(const quint16 id);
    void setProductId(const quint16 id);
    void setVendor(const QByteArray &vendor);
    void setProduct(const QByteArray &product);

    void saveConfig();

    void activate();
    void deactivate();

    void setRemoteMac(const QByteArray &mac);
    void setLocalMac(const QByteArray &mac);

private:
    QUsbManager *m_manager;

    QByteArray m_defaultLocalMac;
    QByteArray m_defaultRemoteMac;

    void initFromConfig();

private slots:
    void loadModule();
    void abort();
};

//! [1]

#endif
