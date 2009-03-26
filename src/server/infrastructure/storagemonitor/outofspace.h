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

#ifndef OUTOFSPACE_H
#define OUTOFSPACE_H

#include "ui_outofspace.h"

class OutOfSpace : public QDialog, public Ui_OutOfSpace
{
    Q_OBJECT

public:
    enum DialogCode {
        CleanupNow = 1,
        HourDelay = 2,
        DayDelay = 3,
        WeekDelay = 4,
        Never = 5
    };

    OutOfSpace(const QString &msg= QString(), QWidget *parent = 0, Qt::WindowFlags f = 0);

public slots:
    void accept();

private slots:
    void updateContext(bool checked);
};

#endif
