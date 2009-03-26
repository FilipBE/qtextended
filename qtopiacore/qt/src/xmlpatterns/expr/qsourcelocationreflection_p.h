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

#ifndef Patternist_SourceLocationReflection_H
#define Patternist_SourceLocationReflection_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QString;

namespace QPatternist
{
    /**
     * @short Base class for all instances that represents something
     * at a certain location.
     *
     * SourceLocationReflection does not provide the source location itself,
     * the address to an instance is the mark for it, that in turn can be used
     * for looking up the source location where that mapping is provided.
     *
     * However, this SourceLocationReflection is not itself the mark. The real
     * mark is retrieved by calling actualReflection(). This mechanism
     * allows a SourceLocationReflection sub-class to delegate, or be an alias,
     * for another source location mark.
     *
     * If sourceLocation() returns a non-null object, it will be used instead
     * of looking up via actualReflection().
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Q_AUTOTEST_EXPORT SourceLocationReflection
    {
    public:
        inline SourceLocationReflection()
        {
        }

        virtual ~SourceLocationReflection()
        {
        }

        virtual const SourceLocationReflection *actualReflection() const = 0;

        /**
         * A description of what represents the source code location, for
         * human consumption. Must be translated, as appropriate.
         */
        virtual QString description() const
        {
            return QString();
        }

        virtual QSourceLocation sourceLocation() const;

    private:
        Q_DISABLE_COPY(SourceLocationReflection)
    };

    class DelegatingSourceLocationReflection : public SourceLocationReflection
    {
    public:
        inline DelegatingSourceLocationReflection(const SourceLocationReflection *const r) : m_r(r)
        {
            Q_ASSERT(r);
        }

        virtual const SourceLocationReflection *actualReflection() const;
        virtual QString description() const;

    private:
        const SourceLocationReflection *const m_r;
    };

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
