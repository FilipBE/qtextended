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

#ifndef QTCONCURRENT_EXCEPTION_H
#define QTCONCURRENT_EXCEPTION_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_QFUTURE

#include <QtCore/qatomic.h>

#ifndef QT_NO_EXCEPTIONS
#  include <exception>
#endif

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(Core)

namespace QtConcurrent
{

#ifndef QT_NO_EXCEPTIONS

class Q_CORE_EXPORT Exception : public std::exception
{
public:
    virtual void raise() const;
    virtual Exception *clone() const;
};

class Q_CORE_EXPORT UnhandledException : public Exception
{
public:
    void raise() const;
    Exception *clone() const;
};

namespace internal {

class Base;
class ExceptionHolder
{
public:
    ExceptionHolder(Exception *exception = 0);
    ExceptionHolder(const ExceptionHolder &other);
    void operator=(const ExceptionHolder &other);
    ~ExceptionHolder();
    Exception *exception() const;
    Base *base;
};

class Q_CORE_EXPORT ExceptionStore
{
public:
    void setException(const Exception &e);
    bool hasException() const;
    ExceptionHolder exception();
    void throwPossibleException();
    bool hasThrown() const;
    ExceptionHolder exceptionHolder;
};

} // namespace internal

#else // QT_NO_EXCEPTIONS

namespace internal {

class Q_CORE_EXPORT ExceptionStore
{
public:
    ExceptionStore() { }
    inline void throwPossibleException() const {}
};

} // namespace internal

#endif

} // namespace QtConcurrent

QT_END_NAMESPACE
QT_END_HEADER

#endif // QT_NO_CONCURRENT

#endif
