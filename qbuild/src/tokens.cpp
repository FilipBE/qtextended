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

#include "tokens.h"

/*!
  \headerfile <tokens.h>
  \title tokens.h
  \ingroup headers
  \sa {Header Files}
  \brief tokens.h
*/

/*!
  \enum Token
  \relates <tokens.h>
  These are the tokens used by the lexer/preprocessor.
  \value NOTOKEN
  \value INCOMPLETE
  \value CHARACTER
  \value DIGIT
  \value WHITESPACE
  \value LANGLE
  \value RANGLE
  \value LPAREN
  \value RPAREN
  \value RBRACE
  \value LBRACE
  \value RBRACKET
  \value LBRACKET
  \value COMMA
  \value NEWLINE
  \value PLUS
  \value MINUS
  \value STAR
  \value EQUAL
  \value PLUS_EQ
  \value MINUS_EQ
  \value STAR_EQ
  \value TILDE_EQ
  \value DOT
  \value AT
  \value QUESTION
  \value AMPERSAND
  \value QUOTE
  \value DOLLAR
  \value DOLLAR_DOLLAR
  \value HASH
  \value BACKSLASH
  \value SLASH
  \value COLON
  \value SEMICOLON
  \value NOT
  \value PIPE
  \value SINGLE_QUOTE
  \value BACK_QUOTE
  \value PERCENT
  \value CARROT
  \value ELSE
  \value OTHER
  \value STRING_START
  \value STRING_END
  \value NAME
  \value SINGLE_SPACE
  \value SCRIPT_START
  \value SCRIPT_END
  \value TOKEN_OP_BEGIN
  \value TOKEN_OP_END
  \value ESCAPE_QUOTE
  \value ESCAPE_ESCAPE
*/

/*!
  \relates <tokens.h>
  Returns a string representation of token \a tok.
*/
const char *tokenToString(Token tok)
{
    switch(tok) {
#define CASE(X) case X: return #X;
        CASE(NOTOKEN)
        CASE(INCOMPLETE)
        CASE(CHARACTER)
        CASE(DIGIT)
        CASE(WHITESPACE)
        CASE(LANGLE)
        CASE(RANGLE)
        CASE(LPAREN)
        CASE(RPAREN)
        CASE(RBRACE)
        CASE(LBRACE)
        CASE(RBRACKET)
        CASE(LBRACKET)
        CASE(COMMA)
        CASE(NEWLINE)
        CASE(PLUS)
        CASE(MINUS)
        CASE(STAR)
        CASE(EQUAL)
        CASE(PLUS_EQ)
        CASE(MINUS_EQ)
        CASE(STAR_EQ)
        CASE(TILDE_EQ)
        CASE(DOT)
        CASE(AT)
        CASE(QUESTION)
        CASE(AMPERSAND)
        CASE(QUOTE)
        CASE(DOLLAR)
        CASE(DOLLAR_DOLLAR)
        CASE(HASH)
        CASE(BACKSLASH)
        CASE(SLASH)
        CASE(COLON)
        CASE(SEMICOLON)
        CASE(NOT)
        CASE(PIPE)
        CASE(SINGLE_QUOTE)
        CASE(BACK_QUOTE)
        CASE(PERCENT)
        CASE(CARROT)
        CASE(ELSE)
        CASE(OTHER)
        CASE(STRING_START)
        CASE(STRING_END)
        CASE(NAME)
        CASE(SINGLE_SPACE)
        CASE(SCRIPT_START)
        CASE(SCRIPT_END)
        //CASE(TOKEN_OP_BEGIN)
        //CASE(TOKEN_OP_END)
        CASE(ESCAPE_QUOTE)
        CASE(ESCAPE_ESCAPE)
#undef CASE
    }
    return 0;
}

