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

#ifndef CONTENTSETLAUNCHERVIEW_H
#define CONTENTSETLAUNCHERVIEW_H

#include "launcherview.h"
#include <qtopiaabstractservice.h>

class QAction;

class ContentSetLauncherView : public LauncherView
{
    Q_OBJECT
public:
    ContentSetLauncherView( QWidget* parent = 0, Qt::WFlags fl = 0 );

private slots:
    void showContentSet();

    void showProperties();

private:
    QAction *propertiesAction;
    QDialog *propDlg;
    QContent propLnk;
};

class ContentSetViewService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    virtual ~ContentSetViewService();

    static ContentSetViewService *instance();

    QContentSet contentSet() const;
    QString title() const;

public slots:
    void showContentSet( const QContentSet &set );
    void showContentSet( const QString &title, const QContentSet &set );

signals:
    void showContentSet();

private:
    ContentSetViewService();

    QContentSet m_contentSet;
    QString m_title;
};

#endif
