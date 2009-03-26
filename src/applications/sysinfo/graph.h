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

#ifndef GRAPH_H
#define GRAPH_H

#include <QFrame>
#include <QVector>
#include <QStringList>

class GraphData
{
public:
    void clear();
    void addItem( const QString &name, int value );

    const QString &name( int i ) const { return names[i]; }
    int value( int i ) const { return values[i]; }
    int count() const { return values.size(); }

private:
    QStringList names;
    QVector<int> values;
};

class Graph : public QFrame
{
    Q_OBJECT
public:
    Graph( QWidget *parent = 0, Qt::WFlags f = 0 );

    void setData( const GraphData *p ) { data = p; }

protected:
    virtual void paintEvent(QPaintEvent *pe);
    virtual void drawContents( QPainter *p ) = 0;
    const GraphData *data;
};

class BarGraph : public Graph
{
    Q_OBJECT
public:
    BarGraph( QWidget *parent = 0, Qt::WFlags f = 0 );

protected:
    virtual void drawContents( QPainter *p );
    void drawSegment( QPainter *p, const QRect &r, const QColor &c );
};

class GraphLegend : public QFrame
{
    Q_OBJECT
public:
    GraphLegend( QWidget *parent = 0, Qt::WFlags f = 0 );

    void setData( const GraphData *p );
    virtual QSize sizeHint() const;
    void setOrientation(Qt::Orientation o);

protected:
    virtual void paintEvent( QPaintEvent *pe );

private:
    const GraphData *data;
    bool horz;
};

#endif
