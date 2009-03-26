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

#include <QCoreApplication>
#include <QMetaProperty>

#include "qcommonvalues_p.h"
#include "qitemmappingiterator_p.h"
#include "qnamepool_p.h"
#include "qsequencemappingiterator_p.h"
#include "qxpathhelper_p.h"

#include "qqobjectnodemodel_p.h"
#include "qqobjectpropertytoattributeiterator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

QMetaProperty QObjectNodeModel::toMetaProperty(const QXmlNodeModelIndex n)
{
    const int propertyOffset = n.additionalData() & (~IsAttribute);
    const QObject *const qo = asQObject(n);
    return qo->metaObject()->property(propertyOffset);
}

QObjectNodeModel::QObjectNodeModel(const NamePool::Ptr &np) : m_namePool(np)
                                                            , m_baseURI(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()))
{
    Q_ASSERT(m_baseURI.isValid());
}

QObject *QObjectNodeModel::asQObject(const QXmlNodeModelIndex n)
{
    return static_cast<QObject *>(n.internalPointer());
}

bool QObjectNodeModel::isProperty(const QXmlNodeModelIndex n)
{
    return n.additionalData() & IsAttribute;
}

QUrl QObjectNodeModel::baseUri(const QXmlNodeModelIndex &) const
{
    return m_baseURI;
}

QUrl QObjectNodeModel::documentUri(const QXmlNodeModelIndex &) const
{
    return m_baseURI;
}

QXmlNodeModelIndex::NodeKind QObjectNodeModel::kind(const QXmlNodeModelIndex &ni) const
{
    if(isProperty(ni))
        return QXmlNodeModelIndex::Attribute;
    else
        return QXmlNodeModelIndex::Element;
}

QXmlNodeModelIndex::DocumentOrder QObjectNodeModel::compareOrder(const QXmlNodeModelIndex &, const QXmlNodeModelIndex &) const
{
    return QXmlNodeModelIndex::Follows; // TODO
}

QXmlNodeModelIndex QObjectNodeModel::root(const QXmlNodeModelIndex &n) const
{
    QObject *p = asQObject(n);
    Q_ASSERT(p);

    do
    {
        QObject *const candidate = p->parent();
        if(candidate)
            p = candidate;
        else
            break;
    }
    while(true);

    return createIndex(p, 0);
}

QXmlNodeModelIndex QObjectNodeModel::parent(const QXmlNodeModelIndex &ni) const
{
    QObject *const p = asQObject(ni)->parent();

    if(p)
        return createIndex(p, 0);
    else
        return QXmlNodeModelIndex();
}

QXmlNodeModelIndex QObjectNodeModel::mapToItem(QObject *const object,
                                               const DynamicContext::Ptr &) const
{
    return createIndex(object, 0);
}

QAbstractXmlForwardIterator<QObject *>::Ptr QObjectNodeModel::mapToSequence(const QObject *const object,
                                                         const DynamicContext::Ptr &) const
{
    return makeListIterator(object->children());
}

QXmlNodeModelIndex::List QObjectNodeModel::ancestors(const QXmlNodeModelIndex n) const
{
    /* We simply throw all of them into a QList and return an iterator over it. */
    const QObject *p = asQObject(n);
    Q_ASSERT(p);

    QXmlNodeModelIndex::List result;
    do
    {
        QObject *const candidate = p->parent();
        if(candidate)
        {
            result.append(createIndex(candidate, 0));
            p = candidate;
        }
        else
            break;
    }
    while(true);

    return result;
}

QXmlNodeModelIndex::Iterator::Ptr QObjectNodeModel::iterate(const QXmlNodeModelIndex &ni, QXmlNodeModelIndex::Axis axis) const
{
    switch(axis)
    {
        case QXmlNodeModelIndex::AxisChild:
            return makeItemMappingIterator<QXmlNodeModelIndex>(this, makeListIterator(asQObject(ni)->children()), DynamicContext::Ptr());
        case QXmlNodeModelIndex::AxisSelf:
            return makeSingletonIterator(ni);
        case QXmlNodeModelIndex::AxisDescendant:
            return makeItemMappingIterator<QXmlNodeModelIndex>(this, makeSequenceMappingIterator<QObject *>(this, mapToSequence(asQObject(ni), DynamicContext::Ptr()), DynamicContext::Ptr()),
                                                               DynamicContext::Ptr());
        case QXmlNodeModelIndex::AxisAttribute:
            return QXmlNodeModelIndex::Iterator::Ptr(new QObjectPropertyToAttributeIterator(this, asQObject(ni)));
        case QXmlNodeModelIndex::AxisAncestor:
            return makeListIterator(ancestors(ni));
        case QXmlNodeModelIndex::AxisAncestorOrSelf:
        {
            QXmlNodeModelIndex::List ans(ancestors(ni));
            ans.prepend(ni);
            return makeListIterator(ans);
        }
        case QXmlNodeModelIndex::AxisDescendantOrSelf:
        case QXmlNodeModelIndex::AxisFollowing:
        case QXmlNodeModelIndex::AxisFollowingSibling:
        case QXmlNodeModelIndex::AxisParent:
            Q_ASSERT_X(false, Q_FUNC_INFO, "This shouldn't be received.");
        case QXmlNodeModelIndex::AxisPreceding:
        case QXmlNodeModelIndex::AxisPrecedingSibling:
        case QXmlNodeModelIndex::AxisNamespace:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "Not implemented.");
            return QXmlNodeModelIndex::Iterator::Ptr();
        }
    }

    Q_ASSERT_X(false, Q_FUNC_INFO,
               "Unknown axis.");
    return QXmlNodeModelIndex::Iterator::Ptr();
}

QXmlName QObjectNodeModel::name(const QXmlNodeModelIndex &ni) const
{
    const char * className = 0;

    if(isProperty(ni))
        className = toMetaProperty(ni).name();
    else
        className = asQObject(ni)->metaObject()->className();

    Q_ASSERT(className);
    QXmlName qName;
    qName.setLocalName(m_namePool->allocateLocalName(QLatin1String(className)));

    return qName;
}

QVector<QXmlName> QObjectNodeModel::namespaceBindings(const QXmlNodeModelIndex &) const
{
    return QVector<QXmlName>();
}

void QObjectNodeModel::sendNamespaces(const QXmlNodeModelIndex &,
                                      const QExplicitlySharedDataPointer<QAbstractXmlReceiver> &) const
{
    /* Do nothing. */
}

Item QObjectNodeModel::propertyValue(const QXmlNodeModelIndex &n) const
{
    const QObject *const qo = asQObject(n);

    const QVariant value(toMetaProperty(n).read(qo));

    /* Property "objectName", possibly others, may have no values, and therefore
     * invalid QVariants will be returned. */
    if(value.isValid())
        return AtomicValue::toXDM(value);
    else
        return Item();
}

QString QObjectNodeModel::stringValue(const QXmlNodeModelIndex &n) const
{
    if(isProperty(n))
    {
        const Item p(propertyValue(n));
        if(p)
            return p.stringValue();
        else
            return QString();
    }
    else
        return QString();
}

QVariant QObjectNodeModel::typedValue(const QXmlNodeModelIndex &n) const
{
    return stringValue(n);
}

Item::Iterator::Ptr QObjectNodeModel::sequencedTypedValue(const QXmlNodeModelIndex &n) const
{
    if(isProperty(n))
        return makeSingletonIterator(Item(propertyValue(n)));
    else
        return CommonValues::emptyIterator;
}

ItemType::Ptr QObjectNodeModel::type(const QXmlNodeModelIndex &ni) const
{
    return XPathHelper::typeFromKind(isProperty(ni) ? QXmlNodeModelIndex::Attribute : QXmlNodeModelIndex::Element);
}

QXmlNodeModelIndex QObjectNodeModel::id(const QString &) const
{
    return QXmlNodeModelIndex();
}

QXmlNodeModelIndex QObjectNodeModel::idref(const QString &) const
{
    return QXmlNodeModelIndex();
}

QT_END_NAMESPACE
