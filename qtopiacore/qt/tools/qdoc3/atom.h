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
  atom.h
*/

#ifndef ATOM_H
#define ATOM_H

#include <qstring.h>

QT_BEGIN_NAMESPACE

class Atom
{
public:
    enum Type { AbstractLeft, AbstractRight, AutoLink, BaseName, BriefLeft, BriefRight, C,
                CaptionLeft, CaptionRight, Code, CodeBad, CodeNew, CodeOld, CodeQuoteArgument,
                CodeQuoteCommand, FootnoteLeft, FootnoteRight, FormatElse, FormatEndif, FormatIf,
                FormattingLeft, FormattingRight, GeneratedList, Image, ImageText, InlineImage,
                LegaleseLeft, LegaleseRight, LineBreak, Link, LinkNode, ListLeft, ListItemNumber,
                ListTagLeft, ListTagRight, ListItemLeft, ListItemRight, ListRight, Nop, ParaLeft,
                ParaRight, QuotationLeft, QuotationRight, RawString, SectionLeft, SectionRight,
                SectionHeadingLeft, SectionHeadingRight, SidebarLeft, SidebarRight,
                SnippetCommand, SnippetIdentifier, SnippetLocation, String,
                TableLeft, TableRight, TableHeaderLeft, TableHeaderRight, TableRowLeft,
                TableRowRight, TableItemLeft, TableItemRight, TableOfContents, Target,
                UnhandledFormat, UnknownCommand,
                Last = UnknownCommand };

    Atom(Type type, const QString &string = "")
	: nex(0), typ(type), str(string) { }
    Atom(Atom *prev, Type type, const QString &string = "")
	: nex(prev->nex), typ(type), str(string) { prev->nex = this; }

    void appendChar( QChar ch ) { str += ch; }
    void appendString( const QString& string ) { str += string; }
    void chopString() { str.chop(1); }
    void setString(const QString &string) { str = string; }
    Atom *next() { return nex; }
    void setNext( Atom *newNext ) { nex = newNext; }

    const Atom *next() const { return nex; }
    Type type() const { return typ; }
    QString typeString() const;
    const QString& string() const { return str; }

private:
    Atom *nex;
    Type typ;
    QString str;
};

#define ATOM_FORMATTING_BOLD            "bold"
#define ATOM_FORMATTING_INDEX           "index"
#define ATOM_FORMATTING_ITALIC          "italic"
#define ATOM_FORMATTING_LINK            "link"
#define ATOM_FORMATTING_PARAMETER       "parameter"
#define ATOM_FORMATTING_SUBSCRIPT       "subscript"
#define ATOM_FORMATTING_SUPERSCRIPT     "superscript"
#define ATOM_FORMATTING_TELETYPE        "teletype"
#define ATOM_FORMATTING_UNDERLINE       "underline"

#define ATOM_LIST_BULLET                "bullet"
#define ATOM_LIST_TAG                   "tag"
#define ATOM_LIST_VALUE                 "value"
#define ATOM_LIST_LOWERALPHA            "loweralpha"
#define ATOM_LIST_LOWERROMAN            "lowerroman"
#define ATOM_LIST_NUMERIC               "numeric"
#define ATOM_LIST_UPPERALPHA            "upperalpha"
#define ATOM_LIST_UPPERROMAN            "upperroman"

QT_END_NAMESPACE

#endif
