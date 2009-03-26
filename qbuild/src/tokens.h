/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef TOKENS_H
#define TOKENS_H

enum Token {
    // Lexer tokens
    NOTOKEN, // 0
    INCOMPLETE,
    WHITESPACE,
    LANGLE,
    RANGLE,
    LPAREN,
    RPAREN,
    RBRACE,
    LBRACE,
    RBRACKET,
    LBRACKET,
    COMMA,
    NEWLINE,
    PLUS,
    MINUS,
    STAR,


    EQUAL, // 16
    TOKEN_OP_BEGIN = EQUAL,
    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    TILDE_EQ,
    TOKEN_OP_END = TILDE_EQ,

    NAME, // 21
    DIGIT,
    CHARACTER,

    ELSE, // 24

    DOT, // 25
    AMPERSAND,
    AT,
    QUESTION,
    ESCAPE_ESCAPE,
    QUOTE,
    ESCAPE_QUOTE,
    DOLLAR,

    DOLLAR_DOLLAR, // Used 33

    HASH, // 34
    BACKSLASH,
    SLASH,
    COLON,
    SEMICOLON,
    NOT,
    PIPE,
    SINGLE_QUOTE,
    BACK_QUOTE,
    CARROT,
    PERCENT,

    OTHER, // 45

    SCRIPT_START, // 46
    SCRIPT_END,

    // Preprocessor tokens
    STRING_START,
    STRING_END,
    SINGLE_SPACE
};

const char *tokenToString(Token);

#endif
