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

#ifndef Patternist_ExpressionFactory_H
#define Patternist_ExpressionFactory_H

#include "qexpression_p.h"

#include <QSharedData>
#include <QUrl>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QIODevice;

namespace QPatternist
{
    /**
     * @short The central entry point for compiling expressions.
     *
     * @ingroup Patternist_expressions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT ExpressionFactory : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<ExpressionFactory> Ptr;

        /**
         * @short This constructor cannot be synthesized since we
         * use the Q_DISABLE_COPY macro.
         */
        inline ExpressionFactory()
        {
        }

        virtual ~ExpressionFactory()
        {
        }

        /**
         * The XPath implementation, emphazing the parser, support
         * different types of languages which all are sub-sets or very
         * close to the XPath 2.0 syntax. This enum is used for communicating
         * what particular language "accent" an expression is of, and should be compiled
         * for.
         *
         * @author Frans Englich <fenglich@trolltech.com>
         */
        enum LanguageAccent
        {
            /**
             * Signifies the XPath 2.0 language.
             *
             * @see <a href="http://www.w3.org/TR/xpath20">XML Path Language (XPath) 2.0</a>
             */
            XPath10     = 1,

            /**
             * Signifies the XPath 1.0 language.
             *
             * @see <a href="http://www.w3.org/TR/xpath">XML Path
             * Language (XPath) Version 1.0</a>
             */
            XPath20     = 2,

            /*
             * Signifies the XSL-T 2.0 Attribute Value Template. That is,
             * a plain attribute value template that contains
             * embedded XPath 2.0 expressions.
             *
             * @see <a href="http://www.w3.org/TR/xslt20/#attribute-value-templates">XSL
             * Transformations (XSLT) Version 2.0, 5.6 Attribute Value Templates</a>
             *
            AVT20
             */

            /**
             * Signifies the XQuery 1.0 language.
             * @see <a href="http://www.w3.org/TR/xquery/">XQuery 1.0: An XML Query Language</a>
             */
            XQuery10    = 4
        };

        enum CompilationStage
        {
            QueryBodyInitial        = 1,
            QueryBodyTypeCheck      = 2,
            QueryBodyCompression    = 4,
            UserFunctionTypeCheck   = 8,
            UserFunctionCompression = 16,
            GlobalVariableTypeCheck = 32
        };

        /**
         * Creates a compiled representation of the XPath expression @p expr, with Static
         * Context information supplied via @p context. This is for example whether the expression
         * is an XPath 1.0 or XPath 2.0 expression, or what functions that are available.
         *
         * @p requiredType specifies what type results of the evaluating the expression
         * must match. Passing CommonValues::ZeroOrMoreItems allows anything as result, while
         * passing CommonSequenceTypes::EBV means anything but an Effective %Boolean Value extractable
         * result is a type error, for example.
         *
         * @note An empty @p expr is an invalid XPath expression. It will be reported as such,
         * but it is neverthless the caller's resonsibility to ensure that it's not that(since
         * it is likely invalid already in the medium it was stored).
         */
        virtual Expression::Ptr createExpression(const QString &expr,
                                                 const StaticContext::Ptr &context,
                                                 const LanguageAccent lang,
                                                 const SequenceType::Ptr &requiredType,
                                                 const QUrl &queryURI);

        Expression::Ptr createExpression(QIODevice *const device,
                                         const StaticContext::Ptr &context,
                                         const LanguageAccent lang,
                                         const SequenceType::Ptr &requiredType,
                                         const QUrl &queryURI);

        /**
         * Finds the last paths of a set of paths(if any) and tells the Path
         * so, such that it can generate the code for checking XPTY0018.
         *
         * Must be called before typeCheck() is called on the operand, since
         * the typeCheck() uses the information for type checking.
         */
        static void registerLastPath(const Expression::Ptr &operand);

    protected:
        /**
         * This function is called by createExpression() each time
         * after a pass on the AST has been completed. Under a typical
         * compilation this function is thus called three times: after the initial
         * build, after the Expression::typeCheck() stage, and after
         * Expression::compress(). @p tree is the AST after each pass.
         *
         * This mechanism is currently used for debugging, since it provides a
         * way of introspecting what the compilation process do to the tree. The
         * current implementation do nothing.
         */
        virtual void processTreePass(const Expression::Ptr &tree,
                                     const CompilationStage stage);

    private:
        Q_DISABLE_COPY(ExpressionFactory)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
