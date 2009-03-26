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
#include "qhelp_global.h"
#include "fulltextsearch/qanalyzer_p.h"
#include "fulltextsearch/qdocument_p.h"
#include "fulltextsearch/qindexreader_p.h"
#include "fulltextsearch/qindexwriter_p.h"
#include "qhelpsearchindexwriter_clucene_p.h"

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>

QT_BEGIN_NAMESPACE

namespace qt {
    namespace fulltextsearch {
        namespace clucene {

class DocumentHelper
{
public:
    DocumentHelper(const QString& fileName, const QByteArray &data)
        : fileName(fileName) , data(readData(data)) {}
    ~DocumentHelper() {}

    bool addFieldsToDocument(QCLuceneDocument *document,
        const QString &namespaceName, const QString &attributes = QString())
    {
        if (!document)
            return false;

        if(!data.isEmpty()) {
            QString parsedData = parseData();
            QString parsedTitle = QHelpGlobal::documentTitle(data);

            if(!parsedData.isEmpty()) {
                document->add(new QCLuceneField(QLatin1String("content"),
                    parsedData,QCLuceneField::INDEX_TOKENIZED));
                document->add(new QCLuceneField(QLatin1String("path"), fileName,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(QLatin1String("title"), parsedTitle,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(QLatin1String("titleTokenized"), parsedTitle,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_TOKENIZED));
                document->add(new QCLuceneField(QLatin1String("namespace"), namespaceName,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(QLatin1String("attribute"), attributes,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_TOKENIZED));
                return true;
            }
        }

        return false;
    }

private:
    QString readData(const QByteArray &data)
    {
        QTextStream textStream(data);
        QString charSet = QHelpGlobal::charsetFromData(data);
        textStream.setCodec(QTextCodec::codecForName(charSet.toLatin1().constData()));

        QString stream = textStream.readAll();
        if (stream.isNull() || stream.isEmpty())
            return QString();

        return stream;
    }

    QString parseData() const
    {
        QString parsedContent;
        int length = data.length();
        const QChar *buf = data.unicode();

        QChar str[64];
        QChar c = buf[0];
        bool valid = true;
        int j = 0, i = 0;

        while (j < length) {
            if ( c == QLatin1Char('<') || c == QLatin1Char('&') ) {
                valid = false;
                if ( i > 1 )
                    parsedContent += QLatin1String(" ") + QString(str,i);

                i = 0;
                c = buf[++j];
                continue;
            }
            if ( ( c == QLatin1Char('>') || c == QLatin1Char(';') ) && !valid ) {
                valid = true;
                c = buf[++j];
                continue;
            }
            if ( !valid ) {
                c = buf[++j];
                continue;
            }
            if ( ( c.isLetterOrNumber() || c == QLatin1Char('_') ) && i < 63 ) {
                str[i] = c.toLower();
                ++i;
            } else {
                if ( i > 1 )
                    parsedContent += QLatin1String(" ") + QString(str,i);

                i = 0;
            }
            c = buf[++j];
        }

        if ( i > 1 )
            parsedContent += QLatin1String(" ") + QString(str,i);

        return parsedContent;
    }

private:
    QString fileName;
    QString data;
};


QHelpSearchIndexWriter::QHelpSearchIndexWriter()
    : QThread(0)
    , m_cancel(false)
{
    // nothing todo
}

QHelpSearchIndexWriter::~QHelpSearchIndexWriter()
{
    mutex.lock();
    this->m_cancel = true;
    waitCondition.wakeOne();
    mutex.unlock();

    wait();
}

void QHelpSearchIndexWriter::cancelIndexing()
{
    mutex.lock();
    this->m_cancel = true;
    mutex.unlock();
}

void QHelpSearchIndexWriter::updateIndex(const QString &collectionFile,
                                         const QString &indexFilesFolder,
                                         bool reindex)
{
    QMutexLocker locker(&mutex);

    this->m_cancel = false;
    this->m_reindex = reindex;
    this->m_collectionFile = collectionFile;
    this->m_indexFilesFolder = indexFilesFolder;

    start(QThread::NormalPriority);
}

void QHelpSearchIndexWriter::optimizeIndex()
{
    if (QCLuceneIndexReader::indexExists(m_indexFilesFolder)) {
        if (QCLuceneIndexReader::isLocked(m_indexFilesFolder))
            return;

        QCLuceneStandardAnalyzer analyzer;
        QCLuceneIndexWriter writer(m_indexFilesFolder, analyzer, false);
        writer.optimize();
        writer.close();
    }
}

void QHelpSearchIndexWriter::run()
{
    QMutexLocker mutexLocker(&mutex);

    if (m_cancel)
        return;

    const bool reindex = this->m_reindex;
    const QString collectionFile(this->m_collectionFile);

    mutexLocker.unlock();

    QHelpEngineCore engine(collectionFile, 0);
    if (!engine.setupData())
        return;

    const QLatin1String key("CluceneIndexedNamespaces");
    if (reindex)
        engine.setCustomValue(key, QLatin1String(""));

    QMap<QString, QDateTime> indexMap;
    const QLatin1String oldKey("CluceneSearchNamespaces");
    if (!engine.customValue(oldKey, QString()).isNull()) {
        // old style qhc file < 4.4.2, need to convert...
        const QStringList indexedNamespaces = engine.customValue(oldKey).
            toString().split(QLatin1String("|"), QString::SkipEmptyParts);
        foreach (const QString& nameSpace, indexedNamespaces)
            indexMap.insert(nameSpace, QDateTime());
        engine.removeCustomValue(oldKey);
    } else {
        QDataStream dataStream(engine.customValue(key).toByteArray());
        dataStream >> indexMap;
    }

    QString indexPath = m_indexFilesFolder;
    
    QFileInfo fInfo(indexPath);
    if (fInfo.exists() && !fInfo.isWritable()) {
        qWarning("Full Text Search, could not create index (missing permissions).");
        return;
    }

    emit indexingStarted();

    QCLuceneIndexWriter *writer = 0;
    QCLuceneStandardAnalyzer analyzer;
    const QStringList registeredDocs = engine.registeredDocumentations();

    QLocalSocket localSocket;
    localSocket.connectToServer(QString(QLatin1String("QtAssistant%1"))
        .arg(QLatin1String(QT_VERSION_STR)));

    QLocalServer localServer;
    bool otherInstancesRunning = true;
    if (!localSocket.waitForConnected()) {
        otherInstancesRunning = false;
        localServer.listen(QString(QLatin1String("QtAssistant%1"))
            .arg(QLatin1String(QT_VERSION_STR)));
    }

#if !defined(QT_NO_EXCEPTIONS)
    try {
#endif
        // check if it's locked, and if the other instance is running
        if (!otherInstancesRunning && QCLuceneIndexReader::isLocked(indexPath))
            QCLuceneIndexReader::unlock(indexPath);
        
        if (QCLuceneIndexReader::isLocked(indexPath)) {
            // poll unless indexing finished to fake progress
            while (QCLuceneIndexReader::isLocked(indexPath)) {
                mutexLocker.relock();
                if (m_cancel)
                    break;
                mutexLocker.unlock();
                this->sleep(1);
            }
            emit indexingFinished();
            return;
        }

        if (QCLuceneIndexReader::indexExists(indexPath) && !reindex) {
            foreach(const QString& namespaceName, registeredDocs) {
                mutexLocker.relock();
                if (m_cancel) {
                    emit indexingFinished();
                    return;
                }
                mutexLocker.unlock();

                if (!indexMap.contains(namespaceName)) {
                    // make sure we remove some partly indexed stuff
                    removeDocuments(indexPath, namespaceName);
                } else {
                    QString path = engine.documentationFileName(namespaceName);
                    if (indexMap.value(namespaceName) < QFileInfo(path).lastModified()) {
                        // make sure we remove some outdated indexed stuff
                        indexMap.remove(namespaceName);
                        removeDocuments(indexPath, namespaceName);
                    }
                }
            }
            writer = new QCLuceneIndexWriter(indexPath, analyzer, false);
        } else {
            writer = new QCLuceneIndexWriter(indexPath, analyzer, true);
        }
#if !defined(QT_NO_EXCEPTIONS)
    } catch (...) {
        qWarning("Full Text Search, could not create index writer.");
        return;
    }
#endif

    writer->setMaxFieldLength(QCLuceneIndexWriter::DEFAULT_MAX_FIELD_LENGTH);

    QStringList namespaces;
    foreach(const QString& namespaceName, registeredDocs) {
        mutexLocker.relock();
        if (m_cancel) {
            writer->close();
            delete writer;
            emit indexingFinished();
            return;
        }
        mutexLocker.unlock();

        namespaces.append(namespaceName);
        if (indexMap.contains(namespaceName))
            continue;

        const QList<QStringList> attributeSets =
            engine.filterAttributeSets(namespaceName);

        if (attributeSets.isEmpty()) {
            const QList<QUrl> docFiles = indexableFiles(&engine, namespaceName,
                QStringList());
            if (!addDocuments(docFiles, engine, QStringList(), namespaceName,
                writer, analyzer))
                break;
        } else {
            bool bail = false;
            foreach (const QStringList attributes, attributeSets) {
                const QList<QUrl> docFiles = indexableFiles(&engine, namespaceName,
                    attributes);
                if (!addDocuments(docFiles, engine, attributes, namespaceName,
                    writer, analyzer)) {
                    bail = true;
                    break;
                }
            }
            if (bail)
                break;
        }
        mutexLocker.relock();
        if (!m_cancel) {
            QString path(engine.documentationFileName(namespaceName));
            indexMap.insert(namespaceName, QFileInfo(path).lastModified());
            writeIndexMap(engine, indexMap);
        }
        mutexLocker.unlock();
    }

    writer->close();
    delete writer;

    mutexLocker.relock();
    if (!m_cancel) {
        mutexLocker.unlock();
    
        QStringList indexedNamespaces = indexMap.keys();
        foreach(const QString& namespaceName, indexedNamespaces) {
            mutexLocker.relock();
            if (m_cancel)
                break;
            mutexLocker.unlock();

            if (!namespaces.contains(namespaceName)) {
                indexMap.remove(namespaceName);
                writeIndexMap(engine, indexMap);
                removeDocuments(indexPath, namespaceName);
            }
        }
    }
    emit indexingFinished();
}

bool QHelpSearchIndexWriter::addDocuments(const QList<QUrl> docFiles,
                                          const QHelpEngineCore &engine,
                                          const QStringList &attributes,
                                          const QString &namespaceName,
                                          QCLuceneIndexWriter *writer,
                                          QCLuceneAnalyzer &analyzer)
{
    foreach(const QUrl& url, docFiles) {
        mutex.lock();
        if (m_cancel) {
            mutex.unlock();
            return false;
        }
        mutex.unlock();

        QCLuceneDocument document;
        DocumentHelper helper(url.toString(), engine.fileData(url));
        const QString list = attributes.join(QLatin1String(" "));
        if (helper.addFieldsToDocument(&document, namespaceName, list))
            writer->addDocument(document, analyzer);
        document.clear();
    }

    return true;
}

void QHelpSearchIndexWriter::removeDocuments(const QString &indexPath,
                                             const QString &namespaceName)
{
    if (namespaceName.isEmpty() || QCLuceneIndexReader::isLocked(indexPath))
        return;

    QCLuceneIndexReader reader = QCLuceneIndexReader::open(indexPath);
    reader.deleteDocuments(QCLuceneTerm(QLatin1String("namespace"),
        namespaceName));

    reader.close();
}

bool QHelpSearchIndexWriter::writeIndexMap(QHelpEngineCore& engine,
                                    const QMap<QString, QDateTime>& indexMap)
{
    QByteArray bArray;
    QDataStream data(&bArray, QIODevice::ReadWrite);
    
    data << indexMap;
    return engine.setCustomValue(QLatin1String("CluceneIndexedNamespaces")
        , bArray);
}

QList<QUrl> QHelpSearchIndexWriter::indexableFiles(QHelpEngineCore *helpEngine,
                                                   const QString &namespaceName,
                                                   const QStringList &attributes) const
{
    QList<QUrl> docFiles =
                helpEngine->files(namespaceName, attributes, QLatin1String("html"));
    docFiles += helpEngine->files(namespaceName, attributes, QLatin1String("htm"));
    docFiles += helpEngine->files(namespaceName, attributes, QLatin1String("txt"));
    return docFiles;
}


        }   // namespace clucene
    }   // namespace fulltextsearch
}   // namespace qt

QT_END_NAMESPACE
