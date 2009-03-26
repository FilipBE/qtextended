/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include <QtDesigner/abstractwidgetfactory.h>
#include "abstractformeditor.h"
#include "abstractwidgetdatabase.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerWidgetFactoryInterface
    \brief The QDesignerWidgetFactoryInterface class provides an interface that is used to control
    the widget factory used by Qt Designer.
    \inmodule QtDesigner
    \internal
*/

/*!
    \fn QDesignerWidgetFactoryInterface::QDesignerWidgetFactoryInterface(QObject *parent)

    Constructs an interface to a widget factory with the given \a parent.
*/
QDesignerWidgetFactoryInterface::QDesignerWidgetFactoryInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    \fn virtual QDesignerWidgetFactoryInterface::~QDesignerWidgetFactoryInterface()
*/
QDesignerWidgetFactoryInterface::~QDesignerWidgetFactoryInterface()
{
}

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerWidgetFactoryInterface::core() const = 0

    Returns the core form editor interface associated with this interface.
*/

/*!
    \fn virtual QWidget* QDesignerWidgetFactoryInterface::containerOfWidget(QWidget *child) const = 0

    Returns the widget that contains the specified \a child widget.
*/

/*!
    \fn virtual QWidget* QDesignerWidgetFactoryInterface::widgetOfContainer(QWidget *container) const = 0


*/

/*!
    \fn virtual QWidget *QDesignerWidgetFactoryInterface::createWidget(const QString &name, QWidget *parent) const = 0

    Returns a new widget with the given \a name and \a parent widget. If no parent is specified,
    the widget created will be a top-level widget.
*/

/*!
    \fn virtual QLayout *QDesignerWidgetFactoryInterface::createLayout(QWidget *widget, QLayout *layout, int type) const = 0

    Returns a new layout of the specified \a type for the given \a widget or \a layout.
*/

/*!
    \fn virtual bool QDesignerWidgetFactoryInterface::isPassiveInteractor(QWidget *widget) = 0
*/

/*!
    \fn virtual void QDesignerWidgetFactoryInterface::initialize(QObject *object) const = 0
*/

QT_END_NAMESPACE
