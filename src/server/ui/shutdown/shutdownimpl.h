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
#ifndef SHUTDOWNIMPL_H
#define SHUTDOWNIMPL_H

#include "ui_shutdown.h"
#include <qdialog.h>

#include "qtopiaserverapplication.h"

class QTimer;
class QAbstractButton;

class ShutdownImpl : public QDialog, Ui::ShutdownDlg
{
    Q_OBJECT
public:
    ShutdownImpl( QWidget* parent = 0, Qt::WFlags fl = 0 );

signals:
    void shutdown( QtopiaServerApplication::ShutdownType );

private slots:
//    void cancelClicked();
    void timeout();
public slots:
    void rebootClicked();
    void quitClicked();
    void restartClicked();
    void shutdownClicked();

private:
    void initiateAction();

    QTimer *timer;
    int progress;
    QtopiaServerApplication::ShutdownType operation;
};

class ShutdownDialogTask : public QObject
{
    Q_OBJECT
public:
    ShutdownDialogTask( QObject* parent = 0 );
private slots:
    void shutdownRequested();
};



#endif

