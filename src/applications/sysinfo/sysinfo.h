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

#ifndef SYSINFO_H
#define SYSINFO_H

#include <QWidget>
#include <qtopiaabstractservice.h>

#include "cleanupwizard.h"


class QTabWidget;
class QScrollArea;
class CleanupWizardService;

class SystemInfo : public QWidget
{
    Q_OBJECT
public:
    SystemInfo( QWidget *parent = 0, Qt::WFlags f = 0 );

public slots:
    void startCleanupWizard();
    void delayedInit();
    
private:

    QTabWidget *tab;

    CleanupWizard * wizard;

    CleanupWizardService *service;

    QScrollArea *wrapWithScrollArea(QWidget *);
};

class CleanupWizardService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SystemInfo;
private:
    CleanupWizardService( SystemInfo *parent )
        : QtopiaAbstractService( "CleanupWizard", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CleanupWizardService();

public slots:
    void showCleanupWizard();
private:
    SystemInfo *parent;
};

#endif
