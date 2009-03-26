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

#ifndef QDELAYEDSCROLLAREA_H
#define QDELAYEDSCROLLAREA_H

#include <QScrollArea>

class QDelayedScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    QDelayedScrollArea(int index, QWidget *parent = 0);
    QDelayedScrollArea(QWidget *parent = 0);
    ~QDelayedScrollArea();

    void setIndex(int index) { i = index; }
    int index() const { return i; }

signals:
    void aboutToShow(int);

protected:
    void showEvent(QShowEvent *event);

private:
    int i;
};

#endif
