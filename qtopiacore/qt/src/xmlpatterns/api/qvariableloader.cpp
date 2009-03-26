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

#include <QVariant>
#include <QStringList>

#include "qanyuri_p.h"
#include "qatomicstring_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qinteger_p.h"
#include "qitem_p.h"
#include "qsequencetype_p.h"
#include "qvariableloader_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

namespace QPatternist
{

    class VariantListIterator : public ListIteratorPlatform<QVariant, Item, VariantListIterator>
    {
    public:
        inline VariantListIterator(const QVariantList &list) : ListIteratorPlatform<QVariant, Item, VariantListIterator>(list)
        {
        }

    private:
        friend class ListIteratorPlatform<QVariant, Item, VariantListIterator>;

        inline Item inputToOutputItem(const QVariant &inputType) const
        {
            return AtomicValue::toXDM(inputType);
        }
    };

    class StringListIterator : public ListIteratorPlatform<QString, Item, StringListIterator>
    {
    public:
        inline StringListIterator(const QStringList &list) : ListIteratorPlatform<QString, Item, StringListIterator>(list)
        {
        }

    private:
        friend class ListIteratorPlatform<QString, Item, StringListIterator>;

        static inline Item inputToOutputItem(const QString &inputType)
        {
            return AtomicString::fromValue(inputType);
        }
    };
}

SequenceType::Ptr VariableLoader::announceExternalVariable(const QXmlName name,
                                                           const SequenceType::Ptr &declaredType)
{
    Q_UNUSED(declaredType);
    const QXmlItem &item = m_bindingHash.value(name);

    if(item.isNull())
    {
        if(m_deviceVariables.contains(name))
            return CommonSequenceTypes::ExactlyOneAnyURI;
        else
            return SequenceType::Ptr();
    }
    else
        return makeGenericSequenceType(QPatternist::AtomicValue::qtToXDMType(item), QPatternist::Cardinality::exactlyOne());
}

Item::Iterator::Ptr VariableLoader::evaluateSequence(const QXmlName name,
                                                     const DynamicContext::Ptr &)
{
    const QXmlItem &item = m_bindingHash.value(name);
    /* Item can be null here, since it's maybe a variable that we have in m_deviceVariables. */
    const QVariant v(item.toAtomicValue());

    switch(v.type())
    {
        case QVariant::StringList:
            return Item::Iterator::Ptr(new StringListIterator(v.toStringList()));
        case QVariant::List:
            return Item::Iterator::Ptr(new VariantListIterator(v.toList()));
        default:
            return makeSingletonIterator(itemForName(name));
    }
}

QPatternist::Item VariableLoader::itemForName(const QXmlName &name) const
{
    const QXmlItem &item = m_bindingHash.value(name);

    if(item.isNode())
        return Item::fromPublic(item);
    else
    {
        const QVariant atomicValue(item.toAtomicValue());
        /* If the atomicValue is null it means it doesn't exist in m_bindingHash, and therefore it must
         * be a QIODevice, since Patternist guarantees to only ask for variables that announceExternalVariable()
         * has accepted. */
        if(atomicValue.isNull())
            return Item(AnyURI::fromValue(QLatin1String("tag:trolltech.com,2007:QtXmlPatterns:QIODeviceVariable:") + m_namePool->stringForLocalName(name.localName())));
        else
            return AtomicValue::toXDM(atomicValue);
    }
}

Item VariableLoader::evaluateSingleton(const QXmlName name,
                                       const DynamicContext::Ptr &)
{
    return itemForName(name);
}

QT_END_NAMESPACE

