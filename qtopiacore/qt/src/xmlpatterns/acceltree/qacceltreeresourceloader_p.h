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

#ifndef Patternist_AccelTreeResourceLoader_H
#define Patternist_AccelTreeResourceLoader_H

#include <QtCore/QHash>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkReply>

#include "qacceltree_p.h"
#include "qnamepool_p.h"
#include "qreportcontext_p.h"
#include "qresourceloader_p.h"
#include "qabstractxmlreceiver.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QIODevice;

namespace QPatternist
{
    class NetworkLoop : public QEventLoop
    {
        Q_OBJECT
    public:
        NetworkLoop() : m_hasReceivedError(false)
        {
        }

    public Q_SLOTS:
        void error(QNetworkReply::NetworkError code)
        {
            Q_UNUSED(code);
            m_hasReceivedError = true;
            exit(1);
        }

        void finished()
        {
            if(m_hasReceivedError)
                exit(1);
            else
                exit(0);
        }
    private:
        bool m_hasReceivedError;
    };

    /**
     * @short Handles requests for documents, and instantiates
     * them as AccelTree instances.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT AccelTreeResourceLoader : public ResourceLoader
    {
    public:
        /**
         * AccelTreeResourceLoader does not own @p networkManager.
         *
         * AccelTreeResourceLoader does not own @p context.
         */
        AccelTreeResourceLoader(const NamePool::Ptr &np,
                                QNetworkAccessManager *const networkManager,
                                ReportContext *const context);

        virtual Item openDocument(const QUrl &uri,
                                  const ReportContext::Ptr &context);
        virtual SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint);
        virtual bool isDocumentAvailable(const QUrl &uri);

        /**
         * Helper function that basically do QNetworkAccessManager::get(), but
         * does it blocked.
         *
         * The return QNetworkReply has emitted QNetworkReply::finished().
         *
         * The caller owns the return QIODevice instance.
         *
         * @p context may be @c null or valid.
         *
         * Calls ReportContext::error() on failure.
         */
        static QNetworkReply *load(const QUrl &uri,
                                   QNetworkAccessManager *const networkManager,
                                   const ReportContext::Ptr &context);

    private:
        static bool streamToReceiver(QIODevice *const dev,
                                     QAbstractXmlReceiver *const receiver,
                                     const NamePool::Ptr &np,
                                     const ReportContext::Ptr &context,
                                     const QUrl &uri);
        bool retrieveDocument(const QUrl &uri,
                              const ReportContext::Ptr &context);

        QHash<QUrl, AccelTree::Ptr>     m_loadedDocuments;
        const NamePool::Ptr             m_namePool;
        QNetworkAccessManager *const    m_networkAccessManager;
        /**
         * We don't store a reference pointer here because then we get a
         * circular reference with GenericStaticContext, when it stores us as a
         * member.
         */
        ReportContext *const            m_context;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
