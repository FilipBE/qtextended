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

#ifndef QABSTRACTSERVERINTERFACE_H
#define QABSTRACTSERVERINTERFACE_H

#include <QWidget>

class QAbstractServerInterface : public QWidget
{
Q_OBJECT
public:
    QAbstractServerInterface(QWidget *parent = 0, Qt::WFlags flags = 0);
protected:
    void closeEvent(QCloseEvent *e);
};

#endif
