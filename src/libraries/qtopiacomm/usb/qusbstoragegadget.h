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

#ifndef QUSBSTORAGEGADGET_H
#define QUSBSTORAGEGADGET_H

#include <qtopiaglobal.h>

#include "qusbgadget.h"

class QTOPIA_EXPORT QUsbStorageGadget : public QUsbGadget
{
    Q_OBJECT

public:
    QUsbStorageGadget(const QString &group = QString(), QObject *parent = 0,
                      QAbstractIpcInterface::Mode mode = Client);

    QStringList backingStore() const;

public slots:
    virtual void setBackingStore(const QStringList &paths);
    virtual void addBackingStore(const QString &path);
    virtual void removeBackingStore(const QString &path);
};

class QUsbStorageGadgetProviderPrivate;
class QTOPIA_EXPORT QUsbStorageGadgetProvider : public QUsbStorageGadget
{
    Q_OBJECT

public:
    QUsbStorageGadgetProvider(const QString &group = QString(), QObject *parent = 0);
    ~QUsbStorageGadgetProvider();

public slots:
    void setVendorId(const quint16 id);
    void setProductId(const quint16 id);
    void setVendor(const QByteArray &vendor);
    void setProduct(const QByteArray &product);

    void saveConfig();

    void activate();
    void deactivate();

    void setBackingStore(const QStringList &paths);
    void addBackingStore(const QString &path);
    void removeBackingStore(const QString &path);

private:
    QUsbStorageGadgetProviderPrivate *d;

    void initFromConfig();
    void initFromSystem();

private slots:
    void loadModule();
    void abort();
};

#endif
