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

#ifndef QXMLQUERY_H
#define QXMLQUERY_H

#include <QtCore/QUrl>
#include <QtXmlPatterns/QAbstractXmlNodeModel>
#include <QtXmlPatterns/QAbstractXmlReceiver>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(XmlPatterns)

class QAbstractMessageHandler;
class QAbstractUriResolver;
class QAbstractXmlReceiver;
class QXmlName;
class QXmlNamePool;
class QXmlNodeIndex;
class QXmlQueryPrivate;
class QXmlResultItems;
class QXmlSerializer;
class QIODevice;

/* The members in the namespace QPatternistSDK are internal, not part of the public API, and
 * unsupported. Using them leads to undefined behavior. */
namespace QPatternistSDK
{
    class TestCase;
}

class Q_XMLPATTERNS_EXPORT QXmlQuery
{
public:
    QXmlQuery();
    QXmlQuery(const QXmlQuery &other);
    QXmlQuery(const QXmlNamePool &np);
    ~QXmlQuery();
    QXmlQuery &operator=(const QXmlQuery &other);

    void setMessageHandler(QAbstractMessageHandler *messageHandler);
    QAbstractMessageHandler *messageHandler() const;

    void setQuery(const QString &sourceCode, const QUrl &documentURI = QUrl());
    void setQuery(QIODevice *sourceCode, const QUrl &documentURI = QUrl());
    void setQuery(const QUrl &queryURI, const QUrl &baseURI = QUrl());

    QXmlNamePool namePool() const;

    void bindVariable(const QXmlName &name, const QXmlItem &value);
    void bindVariable(const QString &localName, const QXmlItem &value);

    void bindVariable(const QXmlName &name, QIODevice *);
    void bindVariable(const QString &localName, QIODevice *);

    bool isValid() const;

    void evaluateTo(QXmlResultItems *result) const;
    bool evaluateTo(QAbstractXmlReceiver *callback) const;
    bool evaluateTo(QStringList *target) const;

    void setUriResolver(const QAbstractUriResolver *resolver);
    const QAbstractUriResolver *uriResolver() const;

    void setFocus(const QXmlItem &item);

private:
    friend class QXmlName;
    friend class QXmlSerializer;
    friend class QPatternistSDK::TestCase;
    QXmlQueryPrivate *d;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
