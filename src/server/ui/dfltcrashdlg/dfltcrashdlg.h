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

#ifndef DFLTCRASHDLG_H
#define DFLTCRASHDLG_H

#include <QObject>
#include <QStringList>
#include "applicationlauncher.h"

class QAbstractMessageBox;
class DefaultCrashDialogTask : public QObject
{
    Q_OBJECT
public:
    DefaultCrashDialogTask( QObject* parent = 0 );
    ~DefaultCrashDialogTask();
private slots:
    void applicationTerminated(const QString &name,
                               ApplicationTypeLauncher::TerminationReason,
                               bool filtered);
private:
    QAbstractMessageBox* ata;
    QStringList ata_list;
};

#endif
