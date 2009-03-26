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

#ifndef QHELPCOLLECTIONHANDLER_H
#define QHELPCOLLECTIONHANDLER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

#include <QtSql/QSqlQuery>

QT_BEGIN_NAMESPACE

class QHelpCollectionHandler : public QObject
{
    Q_OBJECT

public:
    struct DocInfo
    {
        QString fileName;
        QString folderName;
        QString namespaceName;
    };
    typedef QList<DocInfo> DocInfoList;

    QHelpCollectionHandler(const QString &collectionFile, QObject *parent = 0);
    ~QHelpCollectionHandler();

    QString collectionFile() const;

    bool openCollectionFile();
    bool copyCollectionFile(const QString &fileName);

    QStringList customFilters() const;
    bool removeCustomFilter(const QString &filterName);
    bool addCustomFilter(const QString &filterName,
        const QStringList &attributes);

    DocInfoList registeredDocumentations() const;
    bool registerDocumentation(const QString &fileName);
    bool unregisterDocumentation(const QString &namespaceName);

    bool removeCustomValue(const QString &key);
    QVariant customValue(const QString &key, const QVariant &defaultValue) const;
    bool setCustomValue(const QString &key, const QVariant &value);

    bool addFilterAttributes(const QStringList &attributes);
    QStringList filterAttributes() const;
    QStringList filterAttributes(const QString &filterName) const;
    
    int registerNamespace(const QString &nspace, const QString &fileName);
    bool registerVirtualFolder(const QString &folderName, int namespaceId);
    void optimizeDatabase(const QString &fileName);

signals:
    void error(const QString &msg);

private:
    bool isDBOpened();
    bool createTables(QSqlQuery *query);

    bool m_dbOpened;
    QString m_collectionFile;
    QString m_connectionName;
    mutable QSqlQuery m_query;
};

QT_END_NAMESPACE

#endif  //QHELPCOLLECTIONHANDLER_H
