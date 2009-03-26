/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <private/qhiviewwidget_mac_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

extern HIViewRef qt_mac_hiview_for(const QWidget *w); //qwidget_mac.cpp
extern HIViewRef qt_mac_hiview_for(WindowPtr w); //qwidget_mac.cpp

QHIViewWidget::QHIViewWidget(WindowRef windowref, bool createSubs, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    create((WId)qt_mac_hiview_for(windowref), false);
    Rect rect;
    GetWindowBounds(windowref, kWindowContentRgn, &rect);
    setGeometry(QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));
    if(createSubs)
        createQWidgetsFromHIViews();
}

QHIViewWidget::~QHIViewWidget()
{
}

QHIViewWidget::QHIViewWidget(HIViewRef hiviewref, bool createSubs, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    create((WId)hiviewref, false);
    setVisible(HIViewIsVisible(hiviewref));
    if(createSubs)
        createQWidgetsFromHIViews();
}

void QHIViewWidget::createQWidgetsFromHIViews()
{
    // Nicely walk through and make HIViewWidget out of all the children
    addViews_recursive(HIViewGetFirstSubview(qt_mac_hiview_for(this)), this);
}

void QHIViewWidget::addViews_recursive(HIViewRef child, QWidget *parent)
{
    if (!child)
        return;
    QWidget *widget = QWidget::find(WId(child));
    if(!widget)
        widget = new QHIViewWidget(child, false, parent);
    addViews_recursive(HIViewGetFirstSubview(child), widget);
    addViews_recursive(HIViewGetNextView(child), parent);
}

QT_END_NAMESPACE
