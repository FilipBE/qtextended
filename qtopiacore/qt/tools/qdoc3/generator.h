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
  generator.h
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qlist.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include "node.h"
#include "text.h"

QT_BEGIN_NAMESPACE

class ClassNode;
class Config;
class CodeMarker;
class FakeNode;
class FunctionNode;
class InnerNode;
class Location;
class NamespaceNode;
class Node;
class Tree;

class Generator
{
public:
    Generator();
    virtual ~Generator();

    virtual void initializeGenerator(const Config &config);
    virtual void terminateGenerator();
    virtual QString format() = 0;
    virtual bool canHandleFormat(const QString &format) { return format == this->format(); }
    virtual void generateTree(const Tree *tree, CodeMarker *marker) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static Generator *generatorForFormat( const QString& format );

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual void endText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );

    virtual void generateText( const Text& text, const Node *relative,
			       CodeMarker *marker );
    virtual void generateBody( const Node *node, CodeMarker *marker );
    virtual void generateAlsoList( const Node *node, CodeMarker *marker );
    virtual void generateInherits( const ClassNode *classe,
				   CodeMarker *marker );
    virtual void generateInheritedBy( const ClassNode *classe,
				      CodeMarker *marker );

    void generateThreadSafeness( const Node *node, CodeMarker *marker );
    void generateSince(const Node *node, CodeMarker *marker);
    void generateStatus( const Node *node, CodeMarker *marker );
    const Atom *generateAtomList( const Atom *atom, const Node *relative,
				  CodeMarker *marker, bool generate,
				  int& numGeneratedAtoms );
    void generateExampleFiles(const FakeNode *fake, CodeMarker *marker);
    void generateModuleWarning( const ClassNode *classe, CodeMarker *marker);

    virtual int skipAtoms(const Atom *atom, Atom::Type type) const;
    virtual QString fullName(const Node *node, const Node *relative,
                             CodeMarker *marker) const;

    const QString& outputDir() { return outDir; }
    QString indent( int level, const QString& markedCode );
    QString plainCode( const QString& markedCode );
    virtual QString typeString( const Node *node );
    virtual QString imageFileName( const Node *relative, const QString& fileBase );
    void setImageFileExtensions( const QStringList& extensions );
    void unknownAtom( const Atom *atom );
    QMap<QString, QString> &formattingLeftMap();
    QMap<QString, QString> &formattingRightMap();

    QMap<QString, QStringList> editionModuleMap;
    QMap<QString, QStringList> editionGroupMap;

    static QString trimmedTrailing(const QString &string);
    static bool matchAhead( const Atom *atom, Atom::Type expectedAtomType );
    static void supplementAlsoList(const Node *node, QList<Text> &alsoList);

private:
    void generateOverload( const Node *node, CodeMarker *marker );
    void generateReimplementedFrom( const FunctionNode *func,
				    CodeMarker *marker );
    void appendFullName( Text& text, const Node *apparentNode,
			 const Node *relative, CodeMarker *marker,
			 const Node *actualNode = 0 );
    void appendSortedNames(Text& text, const ClassNode *classe,
                           const QList<RelatedClass> &classes,
                           CodeMarker *marker);

    QString amp;
    QString lt;
    QString gt;
    QString quot;
    QRegExp tag;

    static QList<Generator *> generators;
    static QMap<QString, QMap<QString, QString> > fmtLeftMaps;
    static QMap<QString, QMap<QString, QString> > fmtRightMaps;
    static QMap<QString, QStringList> imgFileExts;
    static QSet<QString> outputFormats;
    static QStringList imageFiles;
    static QStringList imageDirs;
    static QString outDir;
    static QString project;
};

QT_END_NAMESPACE

#endif
