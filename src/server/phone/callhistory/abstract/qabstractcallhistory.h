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

#ifndef QABSTRACTCALLHISTORY_H
#define QABSTRACTCALLHISTORY_H

#include <QWidget>
#include <QUniqueId>

class QAbstractCallHistory : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("SingletonServerWidget", "true");
    friend class CallHistoryService;
public:
    QAbstractCallHistory( QWidget *parent = 0, Qt::WFlags fl = 0  ) : QWidget(parent, fl){};

    virtual void reset()           = 0;
    virtual void showMissedCalls() = 0;
    virtual void refresh()         = 0;
    virtual void setFilter(const QString &f) = 0;
signals:
    void requestedDial(const QString& number, const QUniqueId& uid);
    void viewedMissedCalls();
};
#endif
