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

#include "abstractpromotioninterface.h"

QT_BEGIN_NAMESPACE

QDesignerPromotionInterface::~QDesignerPromotionInterface()
{
}

/*!
    \class QDesignerPromotionInterface

    \brief The QDesignerPromotionInterface provides functions for modifying
           the promoted classes in Designer.
    \inmodule QtDesigner
    \internal
    \since 4.3
*/

/*!
    \class QDesignerPromotionInterface::PromotedClass
    A pair of database items containing the base class and the promoted class.

    \typedef QDesignerPromotionInterface::PromotedClasses
    A list of PromotedClass items.

    virtual QDesignerPromotionInterface::PromotedClasses promotedClasses()  const

    Returns a list of promoted classes along with their base classes in alphabetical order.
    It can be used to populate tree models for editing promoted widgets.

*/

/*!
    \fn virtual QSet<QString> QDesignerPromotionInterface::referencedPromotedClassNames()  const

    Returns a set of promoted classed that are referenced by the currently opened forms.
*/

/*! 
    \fn virtual bool QDesignerPromotionInterface::addPromotedClass(const QString &baseClass, const QString &className, const QString &includeFile, QString *errorMessage)

    Add a promoted class named \a with the base class \a and include file \a includeFile. Returns \c true on success or \c false along
    with an error message in \a errorMessage on failure.
*/

/*! 
    \fn  virtual bool QDesignerPromotionInterface::removePromotedClass(const QString &className, QString *errorMessage)

    Remove the promoted class named \a className unless it is referenced by a form. Returns \c true on success or \c false along
    with an error message in \a errorMessage on failure.
*/

/*! 
    \fn  virtual bool QDesignerPromotionInterface::changePromotedClassName(const QString &oldClassName, const QString &newClassName,  QString *errorMessage)

    Change the class name of a promoted class from \a oldClassName to  \a newClassName. Returns \c true on success or \c false along
    with an error message in \a errorMessage on failure.
*/

/*! 
    \fn  virtual bool QDesignerPromotionInterface::setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage)

    Change the include file of a promoted class named \a className to be \a includeFile. Returns \c true on success or \c false along
    with an error message in \a errorMessage on failure.
*/

/*! \fn virtual QList<QDesignerWidgetDataBaseItemInterface *> QDesignerPromotionInterface::promotionBaseClasses() const

     Return a list of base classes that are suitable for promotion.
*/

QT_END_NAMESPACE
