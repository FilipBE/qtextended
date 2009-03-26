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

#ifndef USBGADGETTASK_H
#define USBGADGETTASK_H

#include <QObject>
#include <QList>
#include <QUniqueId>

class QUsbManager;
class QUsbGadget;
class QFileSystem;
class QDialog;
class TaskManagerEntry;
class QDSAction;

class UsbGadgetTask : public QObject
{
    Q_OBJECT

public:
    UsbGadgetTask(QObject *parent = 0);

private:
    void loadProviders();

    void activateEthernet();
    void activateStorage();

private slots:
    void cableConnectedChanged(bool connected);
    void appMessage(const QString &msg, const QByteArray &data);

    void showDialog();
    void deleteDialog();

    void activateGadget();

    void activateStorageGadget();
    void ethernetActivated();
    void ethernetDeactivated();
    void storageDeactivated();

    void qdsResponse(const QUniqueId &);
    void qdsError(const QUniqueId &, const QString &);

private:
    QUsbManager *m_manager;
    QUsbGadget *m_gadget;
    QList<QFileSystem *> m_fileSystems;
    QDialog *m_dialog;
    TaskManagerEntry *m_taskEntry;
    int m_tries;
    QDSAction *m_action;
};

#endif
