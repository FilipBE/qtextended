/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

/*!
    \class QStack
    \brief The QStack class is a template class that provides a stack.

    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QStack\<T\> is one of Qt's generic \l{container classes}. It implements
    a stack data structure for items of a same type.

    A stack is a last in, first out (LIFO) structure. Items are added
    to the top of the stack using push() and retrieved from the top
    using pop(). The top() function provides access to the topmost
    item without removing it.

    Example:

    \snippet doc/src/snippets/qstack/main.cpp 0

    The example will output 3, 2, 1 in that order.

    QStack inherits from QVector. All of QVector's functionality also
    applies to QStack. For example, you can use isEmpty() to test
    whether the stack is empty, and you can traverse a QStack using
    QVector's iterator classes (for example, QVectorIterator). But in
    addition, QStack provides three convenience functions that make
    it easy to implement LIFO semantics: push(), pop(), and top().

    QStack's value type must be an \l{assignable data type}. This
    covers most data types that are commonly used, but the compiler
    won't let you, for example, store a QWidget as a value; instead,
    store a QWidget *.

    \sa QVector, QQueue
*/

/*!
    \fn QStack::QStack()

    Constructs an empty stack.
*/

/*!
    \fn QStack::~QStack()

    Destroys the stack. References to the values in the stack, and all
    iterators over this stack, become invalid.
*/

/*!
    \fn void QStack::push(const T& t)

    Adds element \a t to the top of the stack.

    This is the same as QVector::append().

    \sa pop(), top()
*/

/*!
    \fn T& QStack::top()

    Returns a reference to the stack's top item. This function
    assumes that the stack isn't empty.

    This is the same as QVector::last().

    \sa pop(), push(), isEmpty()
*/

/*!
    \fn const T& QStack::top() const

    \overload

    \sa pop(), push()
*/

/*!
    \fn T QStack::pop()

    Removes the top item from the stack and returns it. This function
    assumes that the stack isn't empty.

    \sa top(), push(), isEmpty()
*/
