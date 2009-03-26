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

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "qiodevicedelegate_p.h"
#include "quriloader_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

URILoader::URILoader(QObject *const parent,
                     const QHash<QXmlName, QIODevice *> &deviceBindings,
                     const NamePool::Ptr &np) : QNetworkAccessManager(parent)
                                              , m_variableNS(QLatin1String("tag:trolltech.com,2007:QtXmlPatterns:QIODeviceVariable:"))
                                              , m_namePool(np)
                                              , m_deviceBindings(deviceBindings)
{
}

QNetworkReply *URILoader::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    const QString requestedUrl(req.url().toString());

    /* On the topic of timeouts:
     *
     * Currently the schemes QNetworkAccessManager handles should/will do timeouts for 4.4, but
     * we need to do timeouts for our own. */
    if(requestedUrl.startsWith(m_variableNS))
    {
        /* We got a QIODevice variable. */
        const QString name(requestedUrl.right(requestedUrl.length() - m_variableNS.length()));

        QIODevice *const source = m_deviceBindings.value(m_namePool->allocateQName(QString(), name, QString()));
        if(source)
            return  new QIODeviceDelegate(source);
    }

    return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

QT_END_NAMESPACE

