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

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QtGui>

class Flake;

class FlakeFactory
{
public:
    FlakeFactory( int maxpool );

    ~FlakeFactory();

    Flake* create();

    void retireAll();

    int count() { return m_count; }
    Flake* flakes() const { return m_pool; }

private:
    int m_count;
    Flake *m_pool;
};

class VisualizationWidget : public QWidget
{
    Q_OBJECT
public:
    VisualizationWidget( QWidget* parent = 0 );

    void setActive( bool active );

protected:
    void paintEvent( QPaintEvent* e );

    // Create new set of flakes every tick
    void timerEvent( QTimerEvent* e );

private:
    void init();

    bool m_init;
    bool m_isactive;

    int m_generate;
    int m_update;

    FlakeFactory m_flakefactory;

    QPixmap m_flakepixmap;
    int m_halfwidth;
};

#endif
