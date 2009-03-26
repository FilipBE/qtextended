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

#ifndef QHELPDBREADER_H
#define QHELPDBREADER_H

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

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QByteArray>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpDBReader : public QObject
{
    Q_OBJECT

public:
    QHelpDBReader(const QString &dbName, const QString &uniqueId,
        QObject *parent);
    ~QHelpDBReader();

    bool init();
    
    QString errorMessage() const;

    QString databaseName() const;
    QString namespaceName() const;
    QString virtualFolder() const;
    QList<QStringList> filterAttributeSets() const;
    QStringList files(const QStringList &filterAttributes,
        const QString &extensionFilter = QString()) const;
    bool fileExists(const QString &virtualFolder, const QString &filePath,
        const QStringList &filterAttributes = QStringList()) const;
    QByteArray fileData(const QString &virtualFolder,
        const QString &filePath) const;

    QStringList customFilters() const;
    QStringList filterAttributes(const QString &filterName = QString()) const;
    QStringList indicesForFilter(const QStringList &filterAttributes) const;
    void linksForKeyword(const QString &keyword, const QStringList &filterAttributes,
        QMap<QString, QUrl> &linkMap) const;

    void linksForIdentifier(const QString &id, const QStringList &filterAttributes,
        QMap<QString, QUrl> &linkMap) const;

	QList<QByteArray> contentsForFilter(const QStringList &filterAttributes) const;
    QUrl urlOfPath(const QString &relativePath) const; 

    bool createAttributesCache(const QStringList &attributes);
    QVariant metaData(const QString &name) const;

private:
    QUrl buildQUrl(const QString &ns, const QString &folder,
        const QString &relFileName, const QString &anchor) const;
    QString mergeList(const QStringList &list) const;
    QString quote(const QString &string) const;
    
    bool m_initDone;
    QString m_dbName;
    QString m_uniqueId;
    QString m_error;
    QSqlQuery *m_query;
    mutable QString m_namespace;
    QSet<QString> m_viewAttributes;
    bool m_useAttributesCache;
};

QT_END_NAMESPACE

#endif
