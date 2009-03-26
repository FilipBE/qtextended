/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
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

#ifndef QSCRIPTVALUEITERATORIMPL_P_H
#define QSCRIPTVALUEITERATORIMPL_P_H

#include "qscriptvalueimplfwd_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptmemberfwd_p.h"

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QScriptClassDataIterator;
class QScriptNameIdImpl;

class QScriptValueIteratorImpl
{
public:
    QScriptValueIteratorImpl(const QScriptValueImpl &obj);
    ~QScriptValueIteratorImpl();

    bool ignoresDontEnum() const;
    void setIgnoresDontEnum(bool ignore);

    bool enumeratePrototype() const;
    void setEnumeratePrototype(bool enable);

    bool hasNext();
    void next();

    bool hasPrevious();
    void previous();

    QScript::Member *member();

    QScriptNameIdImpl *nameId() const;
    QString name() const;

    QScriptValueImpl value() const;
    void setValue(const QScriptValueImpl &value);

    uint flags() const;

    void remove();

    void toFront();
    void toBack();

    QScriptValueImpl object() const;
    void setObject(const QScriptValueImpl &obj);

private:
    bool acceptsMember(const QScriptValueImpl &o, const QScript::Member &m) const;
    QScriptClassDataIterator *getClassDataIterator();

    QScriptValueImpl m_frontObject;

    bool m_ignoresDontEnum;
    bool m_enumerateProto;

    QScriptValueImpl m_object;
    QScript::Member m_member;

    int m_searchIndex;
    QScriptValueImpl m_foundObject;
    QScript::Member m_foundMember;
    bool m_foundForward;
    QScriptClassDataIterator *m_classDataIterator;
    bool m_searchClassDataIterator;
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
