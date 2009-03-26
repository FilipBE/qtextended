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

#ifndef Patternist_SequenceGeneratingFNs_H
#define Patternist_SequenceGeneratingFNs_H

#include "qanyuri_p.h"
#include "qfunctioncall_p.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#fns-that-generate-sequences">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 15.5 Functions and Operators that Generate Sequences</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Implements the function <tt>fn:id()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IdFN : public FunctionCall
    {
    public:
        IdFN();
        typedef QPair<DynamicContext::Ptr, const QAbstractXmlNodeModel *> IDContext;

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

        inline Item mapToItem(const Item &id,
                              const IDContext &context) const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

    protected:
        /**
         * @short Checks that the root node of @p node is a document node, and
         * otherwise issues an error.
         */
        inline void checkTargetNode(const QXmlNodeModelIndex &node, const DynamicContext::Ptr &context) const;

    private:
        typedef QExplicitlySharedDataPointer<const IdFN> ConstPtr;
        bool m_hasCreatedSorter;
    };

    /**
     * @short Implements the function <tt>fn:idref()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IdrefFN : public IdFN
    {
    public:
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Base class for functions that needs to
     * store the static base URI for use at runtime.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StaticBaseUriContainer
    {
    protected:
        inline StaticBaseUriContainer()
        {
        }

        void prepareStaticBaseURI(const StaticContext::Ptr &context)
        {
            m_staticBaseURI = context->baseURI();
        }

        const QUrl &staticBaseURI() const
        {
            return m_staticBaseURI;
        }

    private:
        Q_DISABLE_COPY(StaticBaseUriContainer)
        QUrl m_staticBaseURI;
    };

    /**
     * @short Implements the function <tt>fn:doc()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DocFN : public FunctionCall
                , private StaticBaseUriContainer
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        /**
         * The implementation of this function is placed in a different compilation unit,
         * namely qsequencefns.cpp, to workaround a compiler bug on
         * solaris-cc-64, suspected to be related to the instanciation of QUrl::toQUrl().
         *
         * @see <a
         * href="http://onesearch.sun.com/search/onesearch/index.jsp?qt=6532605&site=sunsolve&otf=ss&col=support-sunsolve&otf=sunsolve&site=ss&col=search-sunsolve">Sun,
         * multiply-defined label for template instance, bug 6532605</a>
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual SequenceType::Ptr staticType() const;

    private:
        SequenceType::Ptr m_type;
    };

    /**
     * @short Implements the function <tt>fn:doc-available()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DocAvailableFN : public FunctionCall
                         , private StaticBaseUriContainer
    {
    public:
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
    };

    /**
     * @short Implements the function <tt>fn:collection()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CollectionFN : public FunctionCall
    {
    public:
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
