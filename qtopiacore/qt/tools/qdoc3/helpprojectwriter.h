/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef HELPPROJECTWRITER_H
#define HELPPROJECTWRITER_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "config.h"
#include "node.h"

QT_BEGIN_NAMESPACE

class Tree;

struct SubProject
{
    QString title;
    QString indexTitle;
    QHash<Node::Type, QSet<FakeNode::SubType> > selectors;
    QMap<QString, const Node *> nodes;
};

struct HelpProject
{
    QString name;
    QString helpNamespace;
    QString virtualFolder;
    QString fileName;
    QString indexRoot;
    QString indexTitle;
    QList<QStringList> keywords;
    QSet<QString> files;
    QSet<QString> extraFiles;
    QSet<QString> filterAttributes;
    QHash<QString, QSet<QString> > customFilters;
    QSet<QString> excluded;
    QMap<QString, SubProject> subprojects;
    QHash<const Node *, QSet<Node::Status> > memberStatus;
};

class HelpProjectWriter
{
public:
    HelpProjectWriter(const Config &config, const QString &defaultFileName);
    void addExtraFile(const QString &file);
    void addExtraFiles(const QSet<QString> &files);
    void generate(const Tree *tre);

private:
    void generateProject(HelpProject &project);
    void generateSections(HelpProject &project, QXmlStreamWriter &writer,
                          const Node *node);
    bool generateSection(HelpProject &project, QXmlStreamWriter &writer,
                         const Node *node);
    QStringList keywordDetails(const Node *node) const;
    void writeNode(HelpProject &project, QXmlStreamWriter &writer, const Node *node);
    void readSelectors(SubProject &subproject, const QStringList &selectors);

    const Tree *tree;

    QString outputDir;
    QList<HelpProject> projects;
};

QT_END_NAMESPACE

#endif
