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

#ifndef QUSBSERIALGADGET_H
#define QUSBSERIALGADGET_H

#include <qtopiaglobal.h>

#include "qusbgadget.h"

class QTOPIA_EXPORT QUsbSerialGadget : public QUsbGadget
{
    Q_OBJECT

public:
    QUsbSerialGadget(const QString &group = QString(), QObject *parent = 0,
                     QAbstractIpcInterface::Mode mode = Client);

    bool supportsCdcAcm() const;
    bool cdcAcm() const;

    QByteArray tty() const;

public slots:
    virtual void setCdcAcm(bool acmMode);
};

class QUsbSerialGadgetProviderPrivate;
class QTOPIA_EXPORT QUsbSerialGadgetProvider : public QUsbSerialGadget
{
    Q_OBJECT

public:
    QUsbSerialGadgetProvider(const QString &group = QString(), QObject *parent = 0);
    ~QUsbSerialGadgetProvider();

public slots:
    void setVendorId(const quint16 id);
    void setProductId(const quint16 id);
    void setVendor(const QByteArray &vendor);
    void setProduct(const QByteArray &product);

    void saveConfig();

    void activate();
    void deactivate();

    void setCdcAcm(bool acmMode);

private:
    QUsbSerialGadgetProviderPrivate *d;

    void initFromConfig();
    void initFromSystem();

private slots:
    void loadModule();
    void abort();
};

#endif
