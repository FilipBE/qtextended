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

#ifndef FIXBDADDR_H
#define FIXBDADDR_H

#include <QDialog>

class FixBdaddr_Private;

class FixBdaddr : public QDialog
{
    Q_OBJECT

public:
    FixBdaddr(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~FixBdaddr();

protected slots:
    void accept();
    void configValue(const QString &key, const QString &value);

private:
    FixBdaddr_Private *m_data;
};

#endif
