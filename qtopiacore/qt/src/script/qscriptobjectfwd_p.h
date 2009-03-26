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

#ifndef QSCRIPTOBJECTFWD_P_H
#define QSCRIPTOBJECTFWD_P_H

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

#include <QtCore/qglobal.h>

#ifndef QT_NO_SCRIPT

#include "qscriptobjectdata_p.h"
#include "qscriptbuffer_p.h"
#include "qscriptmemberfwd_p.h"
#include "qscriptvalueimplfwd_p.h"

QT_BEGIN_NAMESPACE

class QScriptObject
{
public:
    inline void reset();
    inline void finalize(QScriptEngine *engine);
    inline void finalizeData(QScriptEngine *engine);

    inline bool findMember(QScriptNameIdImpl *nameId,
                           QScript::Member *m) const;

    inline bool findGetter(QScript::Member *m) const;

    inline bool findSetter(QScript::Member *m) const;

    inline int memberCount() const;

    inline void createMember(QScriptNameIdImpl *nameId,
                             QScript::Member *member, uint flags);

    inline void member(int index, QScript::Member *member);

    inline void put(const QScript::Member &m, const QScriptValueImpl &v);

    inline QScriptValueImpl &reference(const QScript::Member &m);

    inline void get(const QScript::Member &m, QScriptValueImpl *v);

    inline void removeMember(const QScript::Member &member);

    QScriptValueImpl m_prototype;
    QScriptValueImpl m_scope;
    QScriptValueImpl m_internalValue; // [[value]]
    QScriptObjectData *m_data;
    QScript::Buffer<QScript::Member> m_members;
    QScript::Buffer<QScriptValueImpl> m_objects;
    qint64 m_id;
    QScriptClassInfo *m_class;
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
