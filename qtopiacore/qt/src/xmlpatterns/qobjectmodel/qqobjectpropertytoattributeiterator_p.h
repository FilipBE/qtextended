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

#ifndef Patternist_QObjectPropertyToAttributeIterator_h
#define Patternist_QObjectPropertyToAttributeIterator_h

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * Remember that the XPath Data Models index starts from 1, while
     * QMetaObject::propertyOffset() starts from 0.
     */
    class QObjectPropertyToAttributeIterator : public QAbstractXmlForwardIterator<QXmlNodeModelIndex>
    {
    public:
        inline QObjectPropertyToAttributeIterator(const QObjectNodeModel *const nm,
                                                  QObject *const object) : m_nodeModel(nm)
                                                                         , m_object(object)
                                                                         , m_propertyCount(object->metaObject()->propertyCount())
                                                                         , m_currentPos(0)
        {
        }

        virtual QXmlNodeModelIndex next()
        {
            if(m_currentPos == -1 || m_currentPos == m_propertyCount)
            {
                m_currentPos = -1;
                return QXmlNodeModelIndex();
            }

            QXmlNodeModelIndex retval(m_nodeModel->createIndex(m_object, QObjectNodeModel::IsAttribute | m_currentPos));
            ++m_currentPos;

            return retval;
        }

        virtual QXmlNodeModelIndex current() const
        {
            if(m_currentPos == -1)
                return QXmlNodeModelIndex();
            else
                return m_nodeModel->createIndex(m_object, QObjectNodeModel::IsAttribute | (m_currentPos - 1));
        }

        virtual xsInteger position() const
        {
            return m_currentPos;
        }

        virtual xsInteger count()
        {
            return m_propertyCount;
        }

        virtual QXmlNodeModelIndex::Iterator::Ptr copy() const
        {
            return QXmlNodeModelIndex::Iterator::Ptr(new QObjectPropertyToAttributeIterator(m_nodeModel, m_object));
        }

    private:
        const QObjectNodeModel *const   m_nodeModel;
        QObject *const                  m_object;
        const int                       m_propertyCount;
        xsInteger                       m_currentPos;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
