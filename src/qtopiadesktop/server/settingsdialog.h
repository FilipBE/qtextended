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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <qdplugindefs.h>

#include <QDialog>

class PluginChooser;

class QStackedWidget;
class QFrame;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog( QWidget *parent = 0 );
    virtual ~SettingsDialog();

    void loadGeometry();

signals:
    void applySettings();
    void revertSettings();
    void recheckSettings();

public slots:
    void reject();

private slots:
    void showPlugin( QDAppPlugin *plugin );
    void accept();
    void apply();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );

    PluginChooser *pluginChooser;
    QStackedWidget *stack;
};

#endif
