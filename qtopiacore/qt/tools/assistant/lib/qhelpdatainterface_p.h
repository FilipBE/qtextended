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

#ifndef QHELPDATAINTERFACE_H
#define QHELPDATAINTERFACE_H

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

#include "qhelp_global.h"

#include <QtCore/QStringList>
#include <QtCore/QSharedData>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QHELP_EXPORT QHelpDataContentItem
{
public:
    QHelpDataContentItem(QHelpDataContentItem *parent, const QString &title,
        const QString &reference);
    ~QHelpDataContentItem();

    QString title() const;
    QString reference() const;
    QList<QHelpDataContentItem*> children() const;

private:
    QString m_title;
    QString m_reference;
    QList<QHelpDataContentItem*> m_children;
};

struct QHELP_EXPORT QHelpDataIndexItem {
    QHelpDataIndexItem() {}
    QHelpDataIndexItem(const QString &n, const QString &id, const QString &r)
	    : name(n), identifier(id), reference(r) {}

    QString name;
    QString identifier;
    QString reference;
    
    bool operator==(const QHelpDataIndexItem & other) const;
};

class QHelpDataFilterSectionData : public QSharedData
{
public:
    ~QHelpDataFilterSectionData()
    {
        qDeleteAll(contents);
    }

    QStringList filterAttributes;
    QList<QHelpDataIndexItem> indices;
    QList<QHelpDataContentItem*> contents;
    QStringList files;
};

class QHELP_EXPORT QHelpDataFilterSection
{
public:
    QHelpDataFilterSection();

    void addFilterAttribute(const QString &filter);
    QStringList filterAttributes() const;
    
    void addIndex(const QHelpDataIndexItem &index);
    void setIndices(const QList<QHelpDataIndexItem> &indices);
    QList<QHelpDataIndexItem> indices() const;
    
    void addContent(QHelpDataContentItem *content);
    void setContents(const QList<QHelpDataContentItem*> &contents);
    QList<QHelpDataContentItem*> contents() const;
    
    void addFile(const QString &file);
    void setFiles(const QStringList &files);
    QStringList files() const;

private:
    QSharedDataPointer<QHelpDataFilterSectionData> d;
};

struct QHELP_EXPORT QHelpDataCustomFilter {
    QStringList filterAttributes;
    QString name;
};

class QHELP_EXPORT QHelpDataInterface
{
public:
    QHelpDataInterface() {}
    virtual ~QHelpDataInterface() {}

    virtual QString namespaceName() const = 0;
    virtual QString virtualFolder() const = 0;
    virtual QList<QHelpDataCustomFilter> customFilters() const = 0;
    virtual QList<QHelpDataFilterSection> filterSections() const = 0;
    virtual QMap<QString, QVariant> metaData() const = 0;
    virtual QString rootPath() const = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QHELPDATAINTERFACE_H
