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

#ifndef Patternist_DynamicContext_H
#define Patternist_DynamicContext_H

#include "qautoptr_p.h"
#include "qcachecells_p.h"
#include "qexternalvariableloader_p.h"
#include "qitem_p.h"
#include "qnamepool_p.h"
#include "qnodebuilder_p.h"
#include "qprimitives_p.h"
#include "qreportcontext_p.h"
#include "qresourceloader_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDateTime;
template<typename T> class QVector;

namespace QPatternist
{
    class DayTimeDuration;
    class Expression;

    /**
     * @short Carries information and facilities used at runtime, and hence
     * provides a state for that stage in a thread-safe manner.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#eval_context">XQuery
     * 1.0: An XML Query Language, 2.1.2 Dynamic Context</a>
     * @see <a href="http://www.w3.org/TR/xquery/#id-dynamic-evaluation">XQuery
     * 1.0: An XML Query Language, 2.2.3.2 Dynamic Evaluation Phase</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DynamicContext : public ReportContext
    {
    public:
        typedef QExplicitlySharedDataPointer<DynamicContext> Ptr;

        virtual ~DynamicContext()
        {
        }

        /**
         * This function intentionally returns by reference.
         *
         * @see globalItemCacheCell()
         */
        virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot) = 0;

        /**
         * This function intentionally returns by reference.
         *
         * @see globalItemSequenceCacheCells
         */
        virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot) = 0;

        virtual xsInteger contextPosition() const = 0;
        virtual Item contextItem() const = 0;
        virtual xsInteger contextSize() = 0;

        virtual void setRangeVariable(const VariableSlotID slot,
                                      const Item &newValue) = 0;
        virtual Item rangeVariable(const VariableSlotID slot) const = 0;
        virtual void setExpressionVariable(const VariableSlotID slot,
                                           const QExplicitlySharedDataPointer<Expression> &newValue) = 0;
        virtual QExplicitlySharedDataPointer<Expression>
        expressionVariable(const VariableSlotID slot) const = 0;

        virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const = 0;
        virtual void setPositionIterator(const VariableSlotID slot,
                                         const Item::Iterator::Ptr &newValue) = 0;

        virtual void setFocusIterator(const Item::Iterator::Ptr &it) = 0;
        virtual Item::Iterator::Ptr focusIterator() const = 0;

        virtual QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const = 0;
        virtual QDateTime currentDateTime() const = 0;

        virtual QAbstractXmlReceiver *outputReceiver() const = 0;
        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const = 0;
        virtual ResourceLoader::Ptr resourceLoader() const = 0;
        virtual ExternalVariableLoader::Ptr externalVariableLoader() const = 0;
        virtual NamePool::Ptr namePool() const = 0;

        DynamicContext::Ptr createFocus();
        DynamicContext::Ptr createStack();
        DynamicContext::Ptr createReceiverContext(QAbstractXmlReceiver *const receiver);

        /**
         * Whenever a tree gets built, this function is called. DynamicContext
         * has the responsibility of keeping a copy of @p nm, such that it
         * doesn't go out of scope, since no one else will reference @p nm.
         *
         * The caller guarantees that @p nm is not @c null.
         */
        virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm) = 0;

        /**
         * Same as itemCacheCell(), but is only used for global varibles. This
         * is needed because sometimes stack frames needs to be created for
         * other kinds of variables(such as in the case of user function
         * calls), while the global variable(s) needs to continue to use the
         * same cache, instead of one for each new stack frame, typically an
         * instance of StackContextBase.
         *
         * This has two effects:
         *
         * - It's an optimization. Instead of that a global variable gets evaluated each
         * time a user function is called, think recursive functions, it's done
         * only once.
         * - Query stability, hence affects things like node identity and
         * therfore conformance. Hence affects for instance what nodes a query
         * returns, since node identity affect node deduplication.
         */
        virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot) = 0;

        /**
         * Same as itemSequenceCacheCells() but applies only for global
         * variables.
         *
         * @see globalItemCacheCell()
         */
        virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot) = 0;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
