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

#ifndef QUSBGADGET_H
#define QUSBGADGET_H

#include <qtopiaglobal.h>

#include <QAbstractIpcInterface>

class QTOPIA_EXPORT QUsbGadget : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QUsbGadget(const QString &interfaceName, const QString &group = QString(),
               QObject *parent = 0, QAbstractIpcInterface::Mode mode = Client);

    bool supportsVendorId() const;
    quint16 vendorId() const;

    bool supportsProductId() const;
    quint16 productId() const;

    bool supportsVendor() const;
    QByteArray vendor() const;

    bool supportsProduct() const;
    QByteArray product() const;

    bool active() const;

    QByteArray gadget() const;

public slots:
    virtual void setVendorId(const quint16 id);
    virtual void setProductId(const quint16 id);
    virtual void setVendor(const QByteArray &vendor);
    virtual void setProduct(const QByteArray &product);

    virtual void saveConfig();

    virtual void activate();
    virtual void deactivate();

signals:
    void activated();
    void deactivated();

    void activateFailed();
    void deactivateFailed();
};

#endif
