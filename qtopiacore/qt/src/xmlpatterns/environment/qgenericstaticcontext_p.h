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

#ifndef Patternist_GenericStaticContext_H
#define Patternist_GenericStaticContext_H

#include <QUrl>

#include "qstaticcontext_p.h"
#include "qfunctionfactory_p.h"
#include "qschematypefactory_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Provides setters and getters for the properties defined in StaticContext.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericStaticContext : public StaticContext
    {
    public:
        typedef QExplicitlySharedDataPointer<GenericStaticContext> Ptr;
        /**
         * Constructs a GenericStaticContext. The components are initialized as per
         * the recommended default values in XQuery 1.0. <tt>Default order for empty sequences</tt>,
         * orderingEmptySequence(), is initialized to Greatest.
         *
         * @see <a href="http://www.w3.org/TR/xquery/#id-xq-static-context-components">XQuery
         * 1.0: An XML Query Language, C.1 Static Context Components</a>
         * @param errorHandler the error handler. May be null.
         * @param np the NamePool. May not be null.
         * @param aBaseURI the base URI in the static context. Must be absolute
         * and valid.
         */
        GenericStaticContext(const NamePool::Ptr &np,
                             QAbstractMessageHandler *const errorHandler,
                             const QUrl &aBaseURI);

        virtual NamespaceResolver::Ptr namespaceBindings() const;
        virtual void setNamespaceBindings(const NamespaceResolver::Ptr &);

        virtual FunctionFactory::Ptr functionSignatures() const;
        virtual SchemaTypeFactory::Ptr schemaDefinitions() const;

        /**
         * Returns a DynamicContext used for evaluation at compile time.
         *
         * @bug The DynamicContext isn't stable. It should be cached privately.
         */
        virtual DynamicContext::Ptr dynamicContext() const;

        virtual QUrl baseURI() const;
        virtual void setBaseURI(const QUrl &uri);

        virtual bool compatModeEnabled() const;

        /**
         * @returns always the Unicode codepoint collation URI
         */
        virtual QUrl defaultCollation() const;

        virtual QAbstractMessageHandler * messageHandler() const;

        virtual void setDefaultCollation(const QUrl &uri);

        virtual BoundarySpacePolicy boundarySpacePolicy() const;
        virtual void setBoundarySpacePolicy(const BoundarySpacePolicy policy);

        virtual ConstructionMode constructionMode() const;
        virtual void setConstructionMode(const ConstructionMode mode);

        virtual OrderingMode orderingMode() const;
        virtual void setOrderingMode(const OrderingMode mode);
        virtual OrderingEmptySequence orderingEmptySequence() const;
        virtual void setOrderingEmptySequence(const OrderingEmptySequence ordering);

        virtual QString defaultFunctionNamespace() const;
        virtual void setDefaultFunctionNamespace(const QString &ns);

        virtual QString defaultElementNamespace() const;
        virtual void setDefaultElementNamespace(const QString &ns);

        virtual InheritMode inheritMode() const;
        virtual void setInheritMode(const InheritMode mode);

        virtual PreserveMode preserveMode() const;
        virtual void setPreserveMode(const PreserveMode mode);

        virtual ItemType::Ptr contextItemType() const;
        void setContextItemType(const ItemType::Ptr &type);

        virtual StaticContext::Ptr copy() const;

        virtual ResourceLoader::Ptr resourceLoader() const;
        void setResourceLoader(const ResourceLoader::Ptr &loader);

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
        virtual NamePool::Ptr namePool() const;

        virtual void addLocation(const SourceLocationReflection *const reflection,
                                 const QSourceLocation &location);
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;

        virtual LocationHash sourceLocations() const;
        virtual QAbstractUriResolver *uriResolver() const;

    private:
        BoundarySpacePolicy         m_boundarySpacePolicy;
        ConstructionMode            m_constructionMode;
        QString                     m_defaultElementNamespace;
        QString                     m_defaultFunctionNamespace;
        OrderingEmptySequence       m_orderingEmptySequence;
        OrderingMode                m_orderingMode;
        QUrl                        m_defaultCollation;
        QUrl                        m_baseURI;
        QAbstractMessageHandler *   m_messageHandler;
        PreserveMode                m_preserveMode;
        InheritMode                 m_inheritMode;
        NamespaceResolver::Ptr      m_namespaceResolver;
        ExternalVariableLoader::Ptr m_externalVariableLoader;
        ResourceLoader::Ptr         m_resourceLoader;
        const NamePool::Ptr         m_namePool;
        ItemType::Ptr               m_contextItemType;
        LocationHash                m_locations;
        QAbstractUriResolver *      m_uriResolver;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
