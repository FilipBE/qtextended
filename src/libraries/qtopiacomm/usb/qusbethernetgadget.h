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

#ifndef QUSBETHERNETGADGET_H
#define QUSBETHERNETGADGET_H

#include <qtopiaglobal.h>

#include "qusbgadget.h"

class QTOPIA_EXPORT QUsbEthernetGadget : public QUsbGadget
{
    Q_OBJECT

public:
    QUsbEthernetGadget(const QString &group = QString(), QObject *parent = 0,
                       QAbstractIpcInterface::Mode mode = Client);

    QByteArray remoteMac() const;
    QByteArray localMac() const;
    QByteArray interface() const;

public slots:
    virtual void setRemoteMac(const QByteArray &mac);
    virtual void setLocalMac(const QByteArray &mac);
};

class QUsbEthernetGadgetProviderPrivate;
class QTOPIA_EXPORT QUsbEthernetGadgetProvider : public QUsbEthernetGadget
{
    Q_OBJECT

public:
    QUsbEthernetGadgetProvider(const QString &group = QString(), QObject *parent = 0);
    ~QUsbEthernetGadgetProvider();

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
    QUsbEthernetGadgetProviderPrivate *d;

    void initFromConfig();
    void initFromSystem();
    QByteArray interfaceName() const;

private slots:
    void loadModule();
    void abort();
};

#endif
