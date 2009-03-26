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

// sdk
#include "abstractmetadatabase.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerMetaDataBaseInterface
    \brief The QDesignerMetaDataBaseInterface class provides an interface to Qt Designer's
    object meta database.
    \inmodule QtDesigner
    \internal
*/

/*!
    Constructs an interface to the meta database with the given \a parent.
*/
QDesignerMetaDataBaseInterface::QDesignerMetaDataBaseInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the interface to the meta database.
*/
QDesignerMetaDataBaseInterface::~QDesignerMetaDataBaseInterface()
{
}

/*!
    \fn QDesignerMetaDataBaseItemInterface *QDesignerMetaDataBaseInterface::item(QObject *object) const

    Returns the item in the meta database associated with the given \a object.
*/

/*!
    \fn void QDesignerMetaDataBaseInterface::add(QObject *object)

    Adds the specified \a object to the meta database.
*/

/*!
    \fn void QDesignerMetaDataBaseInterface::remove(QObject *object)

    Removes the specified \a object from the meta database.
*/

/*!
    \fn QList<QObject*> QDesignerMetaDataBaseInterface::objects() const

    Returns the list of objects that have corresponding items in the meta database.
*/

/*!
    \fn QDesignerFormEditorInterface *QDesignerMetaDataBaseInterface::core() const

    Returns the core interface that is associated with the meta database.
*/


// Doc: Interface only

/*!
    \class QDesignerMetaDataBaseItemInterface
    \brief The QDesignerMetaDataBaseItemInterface class provides an interface to individual
    items in Qt Designer's meta database.
    \inmodule QtDesigner
    \internal

    This class allows individual items in \QD's meta-data database to be accessed and modified.
    Use the QDesignerMetaDataBaseInterface class to change the properties of the database itself.
*/

/*!
    \fn QDesignerMetaDataBaseItemInterface::~QDesignerMetaDataBaseItemInterface()

    Destroys the item interface to the meta-data database.
*/

/*!
    \fn QString QDesignerMetaDataBaseItemInterface::name() const

    Returns the name of the item in the database.

    \sa setName()
*/

/*!
    \fn void QDesignerMetaDataBaseItemInterface::setName(const QString &name)

    Sets the name of the item to the given \a name.

    \sa name()
*/

/*!
    \fn QList<QWidget*> QDesignerMetaDataBaseItemInterface::tabOrder() const

    Returns a list of widgets in the order defined by the form's tab order.

    \sa setTabOrder()
*/


/*!
    \fn void QDesignerMetaDataBaseItemInterface::setTabOrder(const QList<QWidget*> &tabOrder)

    Sets the tab order in the form using the list of widgets defined by \a tabOrder.

    \sa tabOrder()
*/

    
/*!
    \fn bool QDesignerMetaDataBaseItemInterface::enabled() const

    Returns whether the item is enabled.

    \sa setEnabled()
*/

/*!
    \fn void QDesignerMetaDataBaseItemInterface::setEnabled(bool enabled)

    If \a enabled is true, the item is enabled; otherwise it is disabled.

    \sa enabled()
*/

QT_END_NAMESPACE
