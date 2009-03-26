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

#ifndef QABSTRACTBROWSERSCREEN_H
#define QABSTRACTBROWSERSCREEN_H

#include <QWidget>

class QAbstractBrowserScreen : public QWidget
{
Q_OBJECT
public:
    QAbstractBrowserScreen(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}

    virtual QString currentView() const = 0;
    virtual bool viewAvailable(const QString &) const = 0;

public slots:
    virtual void resetToView(const QString &) = 0;
    virtual void moveToView(const QString &) = 0;

signals:
    void currentViewChanged(const QString &);
    void applicationLaunched(const QString &);
};

#endif
