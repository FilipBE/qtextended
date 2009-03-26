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

#ifndef QUSBMANAGER_H
#define QUSBMANAGER_H

#include <qtopiaglobal.h>

#include <QAbstractIpcInterfaceGroupManager>
#include <QList>

class QUsbGadget;
class QUsbManagerPrivate;

class QTOPIA_EXPORT QUsbManager : public QAbstractIpcInterfaceGroupManager
{
    Q_OBJECT

public:
    QUsbManager(QObject *parent = 0);
    ~QUsbManager();

    QList<QUsbGadget *> activeGadgets();
    void deactivateGadgets();
    bool canActivate(const QByteArray &gadget);

    bool cableConnected() const;

private:
    QUsbManagerPrivate *d;

private slots:
    void gadgetDeactivated();
    void gadgetActivated();
    void cableConnectedChanged();

signals:
    void deactivateCompleted();
    void deactivateAborted();

    void cableConnectedChanged(bool connected);
};

#endif
