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
    jambiapiparser.h
*/

#ifndef JAMBIAPIPARSER_H
#define JAMBIAPIPARSER_H

#include <QStack>
#include <QXmlDefaultHandler>

#include "codeparser.h"

QT_BEGIN_NAMESPACE

struct ClassOrEnumInfo
{
    QString tag;
    QString javaName;
    QString cppName;
    Node *javaNode;
    Node *cppNode;

    ClassOrEnumInfo() : javaNode(0), cppNode(0) {}
};

class JambiApiParser : public CodeParser, private QXmlDefaultHandler
{
public:
    JambiApiParser(Tree *cppTree);
    ~JambiApiParser();

    void initializeParser(const Config &config);
    void terminateParser();
    QString language();
    QString sourceFileNameFilter();
    void parseSourceFile(const Location &location, const QString &filePath, Tree *tree);
    virtual void doneParsingSourceFiles(Tree *tree);

private:
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool fatalError(const QXmlParseException &exception);
    void jambifyDocsPass2(Node *node);
    bool makeFunctionNode(InnerNode *parent, const QString &synopsis, FunctionNode **funcPtr);

    Tree *cppTre;
    Tree *javaTre;

    bool metJapiTag;
    Location japiLocation;
    QStack<ClassOrEnumInfo> classAndEnumStack;
};

QT_END_NAMESPACE

#endif
