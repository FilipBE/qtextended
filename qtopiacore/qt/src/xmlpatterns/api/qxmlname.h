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

#ifndef QXMLNAME_H
#define QXMLNAME_H

#include <QtCore/QString>
#include <QtCore/QMetaType>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(XmlPatterns)

class QXmlName;
class QXmlNamePool;
Q_XMLPATTERNS_EXPORT uint qHash(const QXmlName &name);

class Q_XMLPATTERNS_EXPORT QXmlName
{
private:
    enum Constant
    {
        LocalNameOffset     = 0,
        LocalNameLength     = 11,
        NamespaceOffset     = 11,
        NamespaceLength     = 9,
        PrefixLength        = 10,
        InvalidCode         = 1 << 31,
        NamespaceMask       = ((1 << ((NamespaceOffset + NamespaceLength) - NamespaceOffset)) - 1) << NamespaceOffset,
        LocalNameMask       = ((1 << ((LocalNameOffset + LocalNameLength) - LocalNameOffset)) - 1) << LocalNameOffset,
        PrefixOffset        = 21,
        PrefixMask          = ((1 << ((PrefixOffset + PrefixLength) - PrefixOffset)) - 1) << PrefixOffset,
        MaximumPrefixes     = (PrefixMask >> PrefixOffset) - 1,
        MaximumLocalNames   = (LocalNameMask >> LocalNameOffset) - 1,
        MaximumNamespaces   = (NamespaceMask >> NamespaceOffset) - 1,
        ExpandedNameMask    = LocalNameMask | NamespaceMask,
        LexicalQNameMask    = LocalNameMask | PrefixMask
    };

public:

    typedef qint16 NamespaceCode;
    typedef NamespaceCode PrefixCode;
    typedef NamespaceCode LocalNameCode;

    QXmlName();

    QXmlName(QXmlNamePool &namePool,
             const QString &localName,
             const QString &namespaceURI = QString(),
             const QString &prefix = QString());

    QString namespaceUri(const QXmlNamePool &query) const;
    QString prefix(const QXmlNamePool &query) const;
    QString localName(const QXmlNamePool &query) const;
    QString toClarkName(const QXmlNamePool &query) const;
    bool operator==(const QXmlName &other) const;
    bool operator!=(const QXmlName &other) const;
    QXmlName &operator=(const QXmlName &other);
    bool isNull() const;
    static bool isNCName(const QString &candidate);

    /* The members below are internal, not part of the public API, and
     * unsupported. Using them leads to undefined behavior. */
    typedef qint64 Code;

    inline QXmlName(const NamespaceCode uri,
                    const LocalNameCode ln,
                    const PrefixCode p = 0);
    /* The implementation for these functions are in utils/qnamepool_p.h. */
    inline LocalNameCode localName() const;
    inline PrefixCode prefix() const;
    inline bool hasPrefix() const;
    inline bool hasNamespace() const;
    inline NamespaceCode namespaceURI() const;
    inline bool isLexicallyEqual(const QXmlName &other) const;
    inline void setPrefix(const PrefixCode c);
    inline void setNamespaceURI(const NamespaceCode c);
    inline void setLocalName(const LocalNameCode c);
    inline Code code() const;

    friend Q_XMLPATTERNS_EXPORT uint qHash(const QXmlName &);

private:
    inline QXmlName(const int c) : m_qNameCode(c)
    {
    }

    Code m_qNameCode;
};

Q_DECLARE_TYPEINFO(QXmlName, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QXmlName) /* This macro must appear after QT_END_NAMESPACE. */

QT_END_HEADER

#endif
