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
  tokenizer.h
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <qstack.h>
#include <qstring.h>

#include <stdio.h>

#include "location.h"

QT_BEGIN_NAMESPACE

/*
  Here come the C++ tokens we support.  The first part contains
  all-purpose tokens; then come keywords.
  
  If you add a keyword, make sure to modify the keyword array in
  tokenizer.cpp as well, and possibly adjust Tok_FirstKeyword and
  Tok_LastKeyword.
*/
enum { Tok_Eoi, Tok_Ampersand, Tok_Aster, Tok_Caret, Tok_LeftParen, Tok_RightParen, Tok_LeftParenAster,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon, Tok_LeftAngle,
       Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen, Tok_LeftBracket, Tok_RightBracket,
       Tok_Tilde, Tok_SomeOperator, Tok_Number, Tok_String, Tok_Doc, Tok_Comment, Tok_Ident,

       Tok_char, Tok_class, Tok_const, Tok_double, Tok_enum, Tok_explicit, Tok_friend, Tok_inline,
       Tok_int, Tok_long, Tok_namespace, Tok_operator, Tok_private, Tok_protected, Tok_public,
       Tok_short, Tok_signals, Tok_signed, Tok_slots, Tok_static, Tok_struct, Tok_template,
       Tok_typedef, Tok_typename, Tok_union, Tok_unsigned, Tok_using, Tok_virtual, Tok_void, Tok_volatile,
       Tok_int64, Tok_Q_OBJECT, Tok_Q_OVERRIDE, Tok_Q_PROPERTY, Tok_Q_DECLARE_SEQUENTIAL_ITERATOR,
       Tok_Q_DECLARE_MUTABLE_SEQUENTIAL_ITERATOR, Tok_Q_DECLARE_ASSOCIATIVE_ITERATOR,
       Tok_Q_DECLARE_MUTABLE_ASSOCIATIVE_ITERATOR, Tok_Q_DECLARE_FLAGS, Tok_Q_SIGNALS, Tok_Q_SLOTS,
       Tok_QT_COMPAT, Tok_QT_COMPAT_CONSTRUCTOR, Tok_QT_DEPRECATED, Tok_QT_MOC_COMPAT,
       Tok_QT_MODULE, Tok_QT3_SUPPORT, Tok_QT3_SUPPORT_CONSTRUCTOR, Tok_QT3_MOC_SUPPORT,
       Tok_QDOC_PROPERTY,

       Tok_FirstKeyword = Tok_char, Tok_LastKeyword = Tok_QDOC_PROPERTY };

/*
  The Tokenizer class implements lexical analysis of C++ source
  files.

  Not every operator or keyword of C++ is recognized; only those that
  are interesting to us. Some Qt keywords or macros are also
  recognized.
*/

class Tokenizer
{
public:
    Tokenizer( const Location& loc, const QByteArray &in );
    Tokenizer( const Location& loc, FILE *in );

    ~Tokenizer();

    int getToken();
    void setParsingFnOrMacro(bool macro) { parsingMacro = macro; }
    bool parsingFnOrMacro() const { return parsingMacro; }

    const Location &location() const { return yyTokLoc; }
    QString previousLexeme() const { return QString( yyPrevLex ); }
    QString lexeme() const { return QString( yyLex ); }
    QString version() const { return yyVersion; }
    int braceDepth() const { return yyBraceDepth; }
    int parenDepth() const { return yyParenDepth; }
    int bracketDepth() const { return yyBracketDepth; }

    static void initialize(const Config &config);
    static void terminate();
    static bool isTrue(const QString &condition);

private:
    void init();
    void start( const Location& loc );
    /*
      This limit on the length of a lexeme seems fairly high, but a
      doc comment can be arbitrarily long. The previous 65,536 limit
      was reached by Mark Summerfield.
    */
    enum { yyLexBufSize = 524288 };

    int getch()
    {
        return yyPos == yyIn.size() ? EOF : yyIn[yyPos++];
    }

    inline int getChar()
    {
        if ( yyCh == EOF )
            return EOF;
        if ( yyLexLen < yyLexBufSize - 1 ) {
            yyLex[yyLexLen++] = (char) yyCh;
            yyLex[yyLexLen] = '\0';
        }
        yyCurLoc.advance( yyCh );
        int ch = getch();
        if (ch == EOF)
            return EOF;
        // cast explicitely to make sure the value of ch 
        // is in range [0..255] to avoid assert messages 
        // when using debug CRT that checks its input.
        return int(uint(uchar(ch))); 
    }

    int getTokenAfterPreprocessor();
    void pushSkipping( bool skip );
    bool popSkipping();

    Location yyTokLoc;
    Location yyCurLoc;
    char *yyLexBuf1;
    char *yyLexBuf2;
    char *yyPrevLex;
    char *yyLex;
    size_t yyLexLen;
    QStack<bool> yyPreprocessorSkipping;
    int yyNumPreprocessorSkipping;
    int yyBraceDepth;
    int yyParenDepth;
    int yyBracketDepth;
    int yyCh;

    QString yyVersion;
    bool parsingMacro;

protected:
    QByteArray yyIn;
    int yyPos;
};

QT_END_NAMESPACE

#endif
