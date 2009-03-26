/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtXMLPatterns module of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef PATTERNIST_VARIABLELOADER_P_H
#define PATTERNIST_VARIABLELOADER_P_H

#include <QtCore/QSet>

#include "qdynamiccontext_p.h"
#include "qexternalvariableloader_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class VariableLoader : public ExternalVariableLoader
    {
    public:
        typedef QHash<QXmlName, QXmlItem> BindingHash;
        typedef QExplicitlySharedDataPointer<VariableLoader> Ptr;

        inline VariableLoader(const BindingHash &bindings,
                              const NamePool::Ptr &np,
                              const QHash<QXmlName, QIODevice *> &deviceVars) : m_bindingHash(bindings)
                                                                              , m_namePool(np)
                                                                              , m_deviceVariables(deviceVars)
        {
        }

        virtual QPatternist::SequenceType::Ptr announceExternalVariable(const QXmlName name,
                                                                        const QPatternist::SequenceType::Ptr &declaredType);
        virtual QPatternist::Item::Iterator::Ptr evaluateSequence(const QXmlName name,
                                                                  const QPatternist::DynamicContext::Ptr &);

        virtual QPatternist::Item evaluateSingleton(const QXmlName name,
                                                    const QPatternist::DynamicContext::Ptr &);
    private:

        inline QPatternist::Item itemForName(const QXmlName &name) const;

        const BindingHash                   m_bindingHash;
        const NamePool::Ptr                 m_namePool;
        const QHash<QXmlName, QIODevice *>  m_deviceVariables;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
