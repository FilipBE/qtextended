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

#ifndef Patternist_AccelTreeBuilder_H
#define Patternist_AccelTreeBuilder_H

#include <QSet>
#include <QStack>

#include "private/qxmlutils_p.h"
#include "qacceltree_p.h"
#include "qbuiltintypes_p.h"
#include "qcompressedwhitespace_p.h"
#include "qnamepool_p.h"
#include "qnodebuilder_p.h"
#include "qreportcontext_p.h"
#include "qsourcelocationreflection_p.h"
#include "qpatternistlocale_p.h"
#include <QtDebug>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Builds an AccelTree from a stream of XML/Item events
     * received through the NodeBuilder interface.
     *
     * If FromDocument is @c true, it is assumed that AccelTreeBuilder is fed
     * events from an XML document, otherwise it is assumed the events
     * are from node constructor expressions.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<bool FromDocument>
    class AccelTreeBuilder : public NodeBuilder
                           , public SourceLocationReflection
    {
    public:
        typedef QExplicitlySharedDataPointer<AccelTreeBuilder> Ptr;

        /**
         * @param context may be @c null.
         */
        AccelTreeBuilder(const QUrl &docURI,
                         const QUrl &baseURI,
                         const NamePool::Ptr &np,
                         ReportContext *const context);
        virtual void startDocument();
        virtual void endDocument();
        virtual void startElement(const QXmlName &name);
        virtual void endElement();
        virtual void attribute(const QXmlName &name, const QStringRef &value);
        virtual void characters(const QStringRef &ch);
        virtual void whitespaceOnly(const QStringRef &ch);
        virtual void processingInstruction(const QXmlName &target,
                                           const QString &data);
        virtual void namespaceBinding(const QXmlName &nb);
        virtual void comment(const QString &content);
        virtual void item(const Item &it);

        virtual QAbstractXmlNodeModel::Ptr builtDocument();
        virtual NodeBuilder::Ptr create(const QUrl &baseURI) const;
        virtual void startOfSequence();
        virtual void endOfSequence();

        inline AccelTree::Ptr builtDocument() const
        {
            return m_document;
        }

        virtual void atomicValue(const QVariant &value);

        virtual const SourceLocationReflection *actualReflection() const;
        virtual QSourceLocation sourceLocation() const;

    private:
        inline void startStructure();

        inline AccelTree::PreNumber currentDepth() const
        {
            return m_ancestors.count() -1;
        }

        inline AccelTree::PreNumber currentParent() const
        {
            return m_ancestors.isEmpty() ? -1 : m_ancestors.top();
        }

        enum Constants
        {
            DefaultNodeStackSize = 10,
            SizeIsEmpty = 0
        };

        AccelTree::PreNumber            m_preNumber;
        bool                            m_isPreviousAtomic;
        bool                            m_hasCharacters;
        /**
         * Whether m_characters has been run through
         * CompressedWhitespace::compress().
         */
        bool                            m_isCharactersCompressed;
        QString                         m_characters;
        NamePool::Ptr                   m_namePool;
        AccelTree::Ptr                  m_document;
        QStack<AccelTree::PreNumber>    m_ancestors;
        QStack<AccelTree::PreNumber>    m_size;

        /** If we have already commenced a document, we don't want to
         * add more document nodes. We keep track of them with this
         * counter, which ensures that startDocument() and endDocument()
         * are skipped consistently. */
        AccelTree::PreNumber            m_skippedDocumentNodes;

        /**
         * All attribute values goes through this set such that we store only
         * one QString for identical attribute values.
         */
        QSet<QString>                   m_attributeCompress;
        const QUrl                      m_documentURI;
        /**
         * We don't store a reference pointer here because then we get a
         * circular reference with GenericDynamicContext, when it stores us as
         * a member.
         */
        ReportContext *const            m_context;
    };

#include "qacceltreebuilder.cpp"
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
