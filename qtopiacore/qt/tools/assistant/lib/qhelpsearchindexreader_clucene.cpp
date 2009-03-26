/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "qhelpenginecore.h"
#include "fulltextsearch/qsearchable_p.h"
#include "fulltextsearch/qqueryparser_p.h"
#include "fulltextsearch/qindexreader_p.h"
#include "qhelpsearchindexreader_clucene_p.h"

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

namespace qt {
    namespace fulltextsearch {
        namespace clucene {

QHelpSearchIndexReader::QHelpSearchIndexReader()
    : QThread()
    , m_cancel(false)
{
    // nothing todo
}

QHelpSearchIndexReader::~QHelpSearchIndexReader()
{
    mutex.lock();
    this->m_cancel = true;
    waitCondition.wakeOne();
    mutex.unlock();

    wait();
}

void QHelpSearchIndexReader::cancelSearching()
{
    mutex.lock();
    this->m_cancel = true;
    mutex.unlock();
}

void QHelpSearchIndexReader::search(const QString &collectionFile, 
                                    const QString &indexFilesFolder,
                                    const QList<QHelpSearchQuery> &queryList)
{
    QMutexLocker lock(&mutex);
    
    this->hitList.clear();
    this->m_cancel = false;
    this->m_query = queryList;
    this->m_collectionFile = collectionFile;
    this->m_indexFilesFolder = indexFilesFolder;

    start(QThread::NormalPriority);
}

int QHelpSearchIndexReader::hitsCount() const
{
    return hitList.count();
}

QHelpSearchEngine::SearchHit QHelpSearchIndexReader::hit(int index) const
{
    return hitList.at(index);
}

void QHelpSearchIndexReader::run()
{
    mutex.lock();
    
    if (m_cancel) {
        mutex.unlock();
        return;
    }

    const QString collectionFile(this->m_collectionFile);
    const QList<QHelpSearchQuery> &queryList = this->m_query;
    const QString indexPath(m_indexFilesFolder);

    mutex.unlock();

    QHelpEngineCore engine(collectionFile, 0);
    if (!engine.setupData())
        return;

    QFileInfo fInfo(indexPath);
    if (fInfo.exists() && !fInfo.isWritable()) {
        qWarning("Full Text Search, could not read index (missing permissions).");
        return;
    }

    if(QCLuceneIndexReader::indexExists(indexPath)) {
        mutex.lock();
        if (m_cancel) {
            mutex.unlock();
            return;
        }
        mutex.unlock();

        emit searchingStarted();

#if !defined(QT_NO_EXCEPTIONS)
        try {
#endif
            QCLuceneBooleanQuery booleanQuery;
            if (!buildQuery(booleanQuery, queryList)) {
                emit searchingFinished(0);
                return;
            }

            const QStringList attribList = engine.filterAttributes(engine.currentFilter());
            if (!attribList.isEmpty()) {
                QCLuceneStandardAnalyzer analyzer;
                QCLuceneQuery* query = QCLuceneQueryParser::parse(QLatin1String("+")
                    + attribList.join(QLatin1String(" +")), QLatin1String("attribute"),
                    analyzer);

                if (!query) {
                    emit searchingFinished(0);
                    return;
                }
                booleanQuery.add(query, true, true, false);
            }

            QCLuceneIndexSearcher indexSearcher(indexPath);
            QCLuceneHits hits = indexSearcher.search(booleanQuery);
            const QStringList namespaceList = engine.registeredDocumentations();

            QSet<QString> pathSet;
            QCLuceneDocument document;
            for (qint32 i = 0; i < hits.length(); i++) {
                document = hits.document(i);
                const QString path = document.get(QLatin1String("path"));
                if (!pathSet.contains(path) && namespaceList.contains(
                    document.get(QLatin1String("namespace")), Qt::CaseInsensitive)) {
                    pathSet.insert(path);
                    hitList.append(qMakePair(path, document.get(QLatin1String("title"))));
                }
                document.clear();

                mutex.lock();
                if (m_cancel) {
                    mutex.unlock();
                    emit searchingFinished(0);
                    return;
                }
                mutex.unlock();
            }

            indexSearcher.close();
            emit searchingFinished(hitList.count());

#if !defined(QT_NO_EXCEPTIONS)
        } catch(...) {
            hitList.clear();
            emit searchingFinished(0);
        }
#endif
    }
}

bool QHelpSearchIndexReader::defaultQuery(const QString &term,
                                          QCLuceneBooleanQuery &booleanQuery)
{
    QCLuceneStandardAnalyzer analyzer;
    
    const QLatin1String c("content");
    const QLatin1String t("titleTokenized");

    QCLuceneQuery *query = QCLuceneQueryParser::parse(term, c, analyzer);
    QCLuceneQuery *query2 = QCLuceneQueryParser::parse(term, t, analyzer);
    if (query && query2) {
        booleanQuery.add(query, true, false, false);
        booleanQuery.add(query2, true, false, false);
        return true;
    }

    return false;
}

bool QHelpSearchIndexReader::buildQuery(QCLuceneBooleanQuery &booleanQuery, 
                                        const QList<QHelpSearchQuery> &queryList)
{
    foreach (const QHelpSearchQuery query, queryList) {
        switch (query.fieldName) {
            case QHelpSearchQuery::FUZZY: {
                const QLatin1String fuzzy("~");
                foreach (const QString term, query.wordList) {
                    if (term.isEmpty() || !defaultQuery(term.toLower() + fuzzy, booleanQuery))
                        return false;
                }
            }   break;

            case QHelpSearchQuery::WITHOUT: {
                QStringList stopWords = QCLuceneStopAnalyzer().englishStopWords();
                foreach (const QString term, query.wordList) {
                    if (stopWords.contains(term, Qt::CaseInsensitive))
                        continue;

                    QCLuceneQuery *query = new QCLuceneTermQuery(QCLuceneTerm(
                        QLatin1String("content"), term.toLower()));
                    QCLuceneQuery *query2 = new QCLuceneTermQuery(QCLuceneTerm(
                        QLatin1String("titleTokenized"), term.toLower()));

                    if (query && query2) {
                        booleanQuery.add(query, true, false, true);
                        booleanQuery.add(query2, true, false, true);
                    } else {
                        return false;
                    }
                }
            }   break;

            case QHelpSearchQuery::PHRASE: {
                const QString term = query.wordList.at(0).toLower();
                if (term.contains(QLatin1Char(' '))) {
                    QStringList termList = term.split(QLatin1String(" "));
                    QCLucenePhraseQuery *q = new QCLucenePhraseQuery();
                    QStringList stopWords = QCLuceneStopAnalyzer().englishStopWords();
                    foreach (const QString t, termList) {
                        if (!stopWords.contains(t, Qt::CaseInsensitive))
                            q->addTerm(QCLuceneTerm(QLatin1String("content"), t.toLower()));
                    }
                    booleanQuery.add(q, true, true, false);
                } else {
                    QCLuceneQuery *query = new QCLuceneTermQuery(QCLuceneTerm(
                        QLatin1String("content"), term.toLower()));
                    QCLuceneQuery *query2 = new QCLuceneTermQuery(QCLuceneTerm(
                        QLatin1String("titleTokenized"), term.toLower()));

                    if (query && query2) {
                        booleanQuery.add(query, true, true, false);
                        booleanQuery.add(query2, true, false, false);
                    } else {
                        return false;
                    }
                }
            }   break;

            case QHelpSearchQuery::ALL: {
                QStringList stopWords = QCLuceneStopAnalyzer().englishStopWords();
                foreach (const QString term, query.wordList) {
                    if (stopWords.contains(term, Qt::CaseInsensitive))
                        continue;

                    QCLuceneQuery *query = new QCLuceneTermQuery(QCLuceneTerm(
                        QLatin1String("content"), term.toLower()));

                    if (query) {
                        booleanQuery.add(query, true, true, false);
                    } else {
                        return false;
                    }
                }
            }   break;
            
            case QHelpSearchQuery::DEFAULT: {
                QCLuceneStandardAnalyzer analyzer;
                foreach (const QString t, query.wordList) {
                    QCLuceneQuery *query = QCLuceneQueryParser::parse(t.toLower(),
                        QLatin1String("content"), analyzer);
            
                    if (query)
                        booleanQuery.add(query, true, true, false);
                }
            }   break;

            case QHelpSearchQuery::ATLEAST: {
                foreach (const QString term, query.wordList) {
                    if (term.isEmpty() || !defaultQuery(term.toLower(), booleanQuery))
                        return false;
                }
            }
        }
    }

    return true;
}

        }   // namespace clucene
    }   // namespace fulltextsearch
}   // namespace qt

QT_END_NAMESPACE
