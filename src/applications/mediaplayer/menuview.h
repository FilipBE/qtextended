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

#ifndef MENUVIEW_H
#define MENUVIEW_H

#include "menumodel.h"

#include <QtGui>

class MenuViewStyle;

class MenuView : public QListView
{
    Q_OBJECT
public:
    explicit MenuView( QWidget* parent = 0 );
    ~MenuView();

signals:
    void selected( const QModelIndex& index );
    void held( const QModelIndex& index );

private slots:
    void emitHeld();

protected:
    void keyPressEvent( QKeyEvent* e );

    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

private:
    MenuViewStyle *m_style;

    QTimer *m_holdtimer;
    QMouseEvent m_eventcache;
};

class MenuStack
{
public:
    explicit MenuStack( MenuView* view )
        : m_view( view )
    { }

    MenuModel* top() const { return m_stack.top(); }

    void push( MenuModel* model );
    MenuModel* pop();

private:
    MenuView* m_view;
    QStack<MenuModel*> m_stack;
};

#endif
