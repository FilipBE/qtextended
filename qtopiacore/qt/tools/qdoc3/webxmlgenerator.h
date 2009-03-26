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
  WebXMLGenerator.h
*/

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include "codemarker.h"
#include "config.h"
#include "pagegenerator.h"

QT_BEGIN_NAMESPACE

class WebXMLGenerator : public PageGenerator
{
public:
    WebXMLGenerator();
    ~WebXMLGenerator();

    virtual void initializeGenerator(const Config &config);
    virtual void terminateGenerator();
    virtual QString format();
    virtual void generateTree(const Tree *tree, CodeMarker *marker);

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom(QXmlStreamWriter &writer, const Atom *atom,
                             const Node *relative, CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    virtual void generateFakeNode(const FakeNode *fake, CodeMarker *marker);
    virtual QString fileExtension(const Node *node);

    virtual const Atom *addAtomElements(QXmlStreamWriter &writer, const Atom *atom,
                                 const Node *relative, CodeMarker *marker);
    virtual void generateIndexSections(QXmlStreamWriter &writer, const Node *node,
                                       CodeMarker *marker);
    virtual void generateInnerNode( const InnerNode *node, CodeMarker *marker );

private:
    const QPair<QString,QString> anchorForNode(const Node *node);
    void findAllClasses(const InnerNode *node);
    void findAllNamespaces(const InnerNode *node);
    const Node *findNode(const Atom *atom, const Node *relative, CodeMarker *marker);
    const Node *findNode(const QString &title, const Node *relative, CodeMarker *marker);
    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative,
                               CodeMarker *marker, const QMap<QString,
                               const Node *> &nodeMap);
    void generateFullName(QXmlStreamWriter &writer, const Node *apparentNode,
                          const Node *relative, CodeMarker *marker,
                          const Node *actualNode = 0);
    void generateRelations(QXmlStreamWriter &writer, const Node *node, CodeMarker *marker);
    void generateTableOfContents(QXmlStreamWriter &writer, const Node *node,
                                 Doc::SectioningUnit sectioningUnit,
                                 int numColumns, const Node *relative);
    void startLink(QXmlStreamWriter &writer, const Atom *atom, const Node *node,
                   const Node *relative);
    QString targetType(const Node *node);

    const Tree *tre;
    bool generateIndex;
    bool inLink;
    bool inContents;
    bool inSectionHeading;
    bool inTableHeader;
    int numTableRows;
    bool threeColumnEnumValueTable;
    QMap<QString, QMap<QString, const Node *> > moduleClassMap;
    QMap<QString, QMap<QString, const Node *> > moduleNamespaceMap;
    QMap<QString, const Node *> namespaceIndex;
    QMap<QString, const Node *> serviceClasses;
    QString link;
    QString project;
    QString projectDescription;
    QString projectUrl;
    QStringList sectionNumber;
};

QT_END_NAMESPACE

#endif
