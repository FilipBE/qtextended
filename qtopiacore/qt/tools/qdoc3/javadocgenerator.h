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

#ifndef JAVADOCGENERATOR_H
#define JAVADOCGENERATOR_H

#include "htmlgenerator.h"

QT_BEGIN_NAMESPACE

class JavadocGenerator : public HtmlGenerator
{
public:
    JavadocGenerator();
    ~JavadocGenerator();

    void initializeGenerator(const Config &config);
    void terminateGenerator();
    QString format();
    bool canHandleFormat(const QString &format) { return format == "HTML" || format == "javadoc"; }
    void generateTree(const Tree *tree, CodeMarker *marker);
    QString typeString(const Node *node);
    QString imageFileName(const Node *relative, const QString &fileBase);

protected:
    QString fileExtension(const Node *node);
    void startText( const Node *relative, CodeMarker *marker );
    void endText( const Node *relative, CodeMarker *marker );
    int generateAtom( const Atom *atom, const Node *relative, CodeMarker *marker );
    void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    void generateFakeNode( const FakeNode *fake, CodeMarker *marker );

    void generateText( const Text& text, const Node *relative, CodeMarker *marker );
    void generateBody( const Node *node, CodeMarker *marker );
    void generateAlsoList( const Node *node, CodeMarker *marker );

    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );
    QString refForAtom(Atom *atom, const Node *node);

private:
    void generateDcf(const QString &fileBase, const QString &startPage,
                     const QString &title, DcfSection &dcfRoot);
    void generateIndex(const QString &fileBase, const QString &url,
                       const QString &title);
    void generateIndent();
    void generateDoc(const Node *node, CodeMarker *marker);
    void generateEnumItemDoc(const Text &text, const Node *node, CodeMarker *marker);

    QString buffer;
    QIODevice *oldDevice;
    int currentDepth;
};

QT_END_NAMESPACE

#endif
