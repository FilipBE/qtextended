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

/*
  tree.h
*/

#ifndef TREE_H
#define TREE_H

#include "node.h"
#include <QDomElement>
#include <QXmlStreamWriter>

QT_BEGIN_NAMESPACE

class QStringList;
class TreePrivate;

class Tree
{
 public:
    enum FindFlag { SearchBaseClasses = 0x1, 
                    SearchEnumValues = 0x2, 
                    NonFunction = 0x4 };

    Tree();
    ~Tree();

    Node *findNode(const QStringList &path, Node *relative=0, int findFlags=0);
    Node *findNode(const QStringList &path, 
                   Node::Type type, 
                   Node *relative = 0,
                   int findFlags = 0);
    FunctionNode *findFunctionNode(const QStringList &path, 
                                   Node *relative = 0,
                                   int findFlags = 0);
    FunctionNode *findFunctionNode(const QStringList &parentPath, 
                                   const FunctionNode *clone,
                                   Node *relative = 0, 
                                   int findFlags = 0);
    void addBaseClass(ClassNode *subclass, 
                      Node::Access access, 
                      const QStringList &basePath,
		      const QString &dataTypeWithTemplateArgs,
                      InnerNode *parent = 0);
    void addPropertyFunction(PropertyNode *property, 
                             const QString &funcName,
			     PropertyNode::FunctionRole funcRole);
    void addToGroup(Node *node, const QString &group);
    void addToPublicGroup(Node *node, const QString &group);
    QMultiMap<QString, Node *> groups() const;
    QMultiMap<QString, QString> publicGroups() const;
    void resolveInheritance(NamespaceNode *rootNode = 0);
    void resolveProperties();
    void resolveGroups();
    void resolveTargets();
    void fixInheritance(NamespaceNode *rootNode = 0);
    void setVersion(const QString &version) { vers = version; }
    NamespaceNode *root() { return &roo; }

    QString version() const { return vers; }
    const Node *findNode(const QStringList &path, 
                         const Node *relative = 0, 
                         int findFlags = 0) const;
    const Node *findNode(const QStringList &path, 
                         Node::Type type, const 
                         Node *relative = 0,
                         int findFlags = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &path, 
                                         const Node *relative = 0,
                                         int findFlags = 0) const;
    const FunctionNode *findFunctionNode(const QStringList &parentPath, 
                                         const FunctionNode *clone,
                                         const Node *relative = 0,
                                         int findFlags = 0) const;
    const FakeNode *findFakeNodeByTitle(const QString &title) const;
    const Node *findUnambiguousTarget(const QString &target, Atom *&atom) const;
    Atom *findTarget(const QString &target, const Node *node) const;
    const NamespaceNode *root() const { return &roo; }
    void readIndexes(const QStringList &indexFiles);
    bool generateIndexSection(QXmlStreamWriter &writer, const Node *node,
                              bool generateInternalNodes = false) const;
    void generateIndexSections(QXmlStreamWriter &writer, const Node *node,
                              bool generateInternalNodes = false) const;
    void generateIndex(const QString &fileName, 
                       const QString &url,
                       const QString &title, 
                       bool generateInternalNodes = false) const;
    void generateTagFileCompounds(QXmlStreamWriter &writer, 
                                  const InnerNode *inner) const;
    void generateTagFileMembers(QXmlStreamWriter &writer, 
                                const InnerNode *inner) const;
    void generateTagFile(const QString &fileName) const;
    void addExternalLink(const QString &url, const Node *relative);
    QString fullDocumentName(const Node *node) const;
    QString fullDocumentLocation(const Node *node) const;

 private:
    void resolveInheritance(int pass, ClassNode *classe);
    FunctionNode *findVirtualFunctionInBaseClasses(ClassNode *classe, 
                                                   FunctionNode *clone);
    void fixPropertyUsingBaseClasses(ClassNode *classe, PropertyNode *property);
    NodeList allBaseClasses(const ClassNode *classe) const;
    void readIndexFile(const QString &path);
    void readIndexSection(const QDomElement &element, InnerNode *parent,
                          const QString &indexUrl);
    QString readIndexText(const QDomElement &element);
    void resolveIndex();

 private:
    NamespaceNode roo;
    QString vers;
    TreePrivate *priv;
};

QT_END_NAMESPACE

#endif
