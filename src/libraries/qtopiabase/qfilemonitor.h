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

#ifndef QFILEMONITOR_H
#define QFILEMONITOR_H

#include "qtopiailglobal.h"
#include <QObject>

class QFileMonitorPrivate;
class QTOPIAIL_EXPORT QFileMonitor : public QObject
{
Q_OBJECT
public:
    enum Strategy { Auto, DNotify, INotify, Poll, None };

    explicit QFileMonitor(QObject * = 0);
    explicit QFileMonitor(const QString &, Strategy = Auto, QObject * = 0);
    virtual ~QFileMonitor();

    bool isValid() const;
    QString fileName() const;
    Strategy strategy() const;

signals:
    void fileChanged(const QString &);

private:
    friend class QFileMonitorPrivate;
    QFileMonitorPrivate * d;
};

#endif
