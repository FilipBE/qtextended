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

#ifndef MEMORY_H
#define MEMORY_H

#include <QWidget>

class GraphData;
class Graph;
class GraphLegend;
class QLabel;

class MemoryInfo : public QWidget
{
    Q_OBJECT
public:
    MemoryInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MemoryInfo();

protected:
    void timerEvent(QTimerEvent *);
private slots:
    void updateData();
    void init();

private:
    QLabel *totalMem;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};

#endif
