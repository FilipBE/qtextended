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

#ifndef ADDWAPUI_H
#define ADDWAPUI_H

#include <QDialog>
#include <QWapAccount>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class WapAccountPage;
class GatewayPage;
class MMSPage;
class BrowserPage;

class AddWapUI : public QDialog
{
    Q_OBJECT
public:
    AddWapUI( const QString& file = QString(), QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AddWapUI();

public slots:
    void accept();

private slots:
    void optionSelected(QListWidgetItem* item);
    void updateUserHint(QListWidgetItem* cur, QListWidgetItem* prev );

private:
    void init();
    //void writeConfig( const QtopiaNetworkProperties prop );

private:
    QListWidget* options;
    QStackedWidget* stack;
    QLabel* hint;
    QWapAccount acc;
    QString configFile;
    WapAccountPage* accountPage;
    GatewayPage* gatewayPage;
    MMSPage* mmsPage;
    BrowserPage* browserPage;
};

#endif
