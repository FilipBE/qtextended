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

#ifndef QHPWRITER_H
#define QHPWRITER_H

#include <QtXml/QXmlStreamWriter>
#include "filterpage.h"

QT_BEGIN_NAMESPACE

class AdpReader;

class QhpWriter : public QXmlStreamWriter
{
public:
    enum IdentifierPrefix {SkipAll, FilePrefix, GlobalPrefix};
    QhpWriter(const QString &namespaceName,
        const QString &virtualFolder);
    void setAdpReader(AdpReader *reader);
    void setFilterAttributes(const QStringList &attributes);
    void setCustomFilters(const QList<CustomFilter> filters);
    void setFiles(const QStringList &files);
    void generateIdentifiers(IdentifierPrefix prefix,
        const QString prefixString = QString()); 
    bool writeFile(const QString &fileName);

private:
    void writeCustomFilters();
    void writeFilterSection();
    void writeToc();
    void writeKeywords();
    void writeFiles();

    QString m_namespaceName;
    QString m_virtualFolder;
    AdpReader *m_adpReader;
    QStringList m_filterAttributes;
    QList<CustomFilter> m_customFilters;
    QStringList m_files;
    IdentifierPrefix m_prefix;
    QString m_prefixString;
};

QT_END_NAMESPACE

#endif
