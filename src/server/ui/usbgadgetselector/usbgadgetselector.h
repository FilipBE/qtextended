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

#ifndef USBGADGETSELECTOR_H
#define USBGADGETSELECTOR_H

#include <QDialog>

class QUsbManager;
class QListWidget;
class QListWidgetItem;
class QAction;
class QLabel;

class UsbGadgetSelector : public QDialog
{
    Q_OBJECT

    public:
        UsbGadgetSelector(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    public slots:
        void accept();

    private slots:
        void cableConnectedChanged(bool connected);
        void itemActivated(QListWidgetItem *item);

    private:
        void loadServices();

    private:
        QLabel *m_label;
        QListWidget *m_services;
        QAction *m_rememberChoice;
        QListWidgetItem *m_selectedItem;

        QUsbManager *m_manager;

        QString m_service;
        QString m_application;
};

#endif
