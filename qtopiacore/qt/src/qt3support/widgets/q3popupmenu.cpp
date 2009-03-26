/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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
#include "q3popupmenu.h"

QT_BEGIN_NAMESPACE

/*!
    \fn void Q3PopupMenu::setFrameRect(QRect)
    \internal
*/

/*!
    \fn QRect Q3PopupMenu::frameRect() const
    \internal
*/
/*!
    \enum Q3PopupMenu::DummyFrame
    \internal

    \value Box
    \value Sunken
    \value Plain
    \value Raised
    \value MShadow
    \value NoFrame
    \value Panel 
    \value StyledPanel
    \value HLine 
    \value VLine 
    \value GroupBoxPanel
    \value WinPanel 
    \value ToolBarPanel 
    \value MenuBarPanel 
    \value PopupPanel 
    \value LineEditPanel 
    \value TabWidgetPanel 
    \value MShape
*/

/*!
    \fn void Q3PopupMenu::setFrameShadow(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3PopupMenu::frameShadow() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setFrameShape(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3PopupMenu::frameShape() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setFrameStyle(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::frameStyle() const
    \internal
*/

/*!
    \fn int Q3PopupMenu::frameWidth() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setLineWidth(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::lineWidth() const
    \internal
*/

/*!
    \fn void Q3PopupMenu::setMargin(int margin)
    \since 4.2

    Sets the width of the margin around the contents of the widget to \a margin.
    
    This function uses QWidget::setContentsMargins() to set the margin.
    \sa margin(), QWidget::setContentsMargins()
*/

/*!
    \fn int Q3PopupMenu::margin() const 
    \since 4.2

    Returns the with of the the margin around the contents of the widget.
    
    This function uses QWidget::getContentsMargins() to get the margin.
    \sa setMargin(), QWidget::getContentsMargins()
*/

/*!
    \fn void Q3PopupMenu::setMidLineWidth(int)
    \internal
*/

/*!
    \fn int Q3PopupMenu::midLineWidth() const
    \internal
*/

QT_END_NAMESPACE
