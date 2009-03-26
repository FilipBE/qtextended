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

#ifndef QTCONCURRENT_FUNCTIONWRAPPERS_H
#define QTCONCURRENT_FUNCTIONWRAPPERS_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_CONCURRENT

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(Core)

#ifndef qdoc

namespace QtConcurrent {

template <typename T>
class FunctionWrapper0
{
public:
    typedef T (*FunctionPointerType)();
    typedef T result_type;
    inline FunctionWrapper0(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()()
    {
        return functionPointer();
    }
private:
    FunctionPointerType functionPointer;
};

template <typename T, typename U>
class FunctionWrapper1
{
public:
    typedef T (*FunctionPointerType)(U u);
    typedef T result_type;
    inline FunctionWrapper1(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(U u)
    {
        return functionPointer(u);
    }

private:
    FunctionPointerType functionPointer;
};

template <typename T, typename U, typename V>
class FunctionWrapper2
{
public:
    typedef T (*FunctionPointerType)(U u, V v);
    typedef T result_type;
    inline FunctionWrapper2(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(U u, V v)
    {
        return functionPointer(u, v);
    }
private:
    FunctionPointerType functionPointer;
};

template <typename T, typename C>
class MemberFunctionWrapper
{
public:
    typedef T (C::*FunctionPointerType)();
    typedef T result_type;
    inline MemberFunctionWrapper(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(C &c)
    {
        return (c.*functionPointer)();
    }
private:
    FunctionPointerType functionPointer;
};

template <typename T, typename C, typename U>
class MemberFunctionWrapper1
{
public:
    typedef T (C::*FunctionPointerType)(U);
    typedef T result_type;

    inline MemberFunctionWrapper1(FunctionPointerType _functionPointer)
        : functionPointer(_functionPointer)
    { }

    inline T operator()(C &c, U u)
    {
        return (c.*functionPointer)(u);
    }

private:
    FunctionPointerType functionPointer;
};

template <typename T, typename C>
class ConstMemberFunctionWrapper
{
public:
    typedef T (C::*FunctionPointerType)() const;
    typedef T result_type;
    inline ConstMemberFunctionWrapper(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(const C &c) const
    {
        return (c.*functionPointer)();
    }
private:
    FunctionPointerType functionPointer;
};

} // namespace QtConcurrent.

#endif //qdoc

QT_END_NAMESPACE
QT_END_HEADER

#endif // QT_NO_CONCURRENT

#endif
