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

#ifndef ARCHIVEVIEWER_H
#define ARCHIVEVIEWER_H

#include "launcherview.h"
#include <qtopiaabstractservice.h>
#include <QStack>
#include <QKeyEvent>
#include <QPointer>
#include "qabstractmessagebox.h"

class ArchiveViewer : public LauncherView
{
    Q_OBJECT
public:
    ArchiveViewer( QWidget* parent = 0, Qt::WFlags fl = 0 );

public slots:
    void setDocument( const QString &document );

private slots:
    void executeContent( const QContent &content );
    void showProperties();

protected:
    void hideEvent( QHideEvent *event );
    void showEvent( QShowEvent *event );

    void keyPressEvent( QKeyEvent *event );

private:
    QPointer<QAbstractMessageBox> warningBox;
    QAction *propertiesAction;
    QDialog *propDlg;
    QContent propLnk;

    QStack< QContentFilter > filterStack;
};

#endif
