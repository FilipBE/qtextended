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

#ifndef LEXER_H
#define LEXER_H

#include <QList>
#include "tokens.h"

struct LexerToken
{
    LexerToken() : token(NOTOKEN), start(-1), end(-1), line(-1), offset(-1) {}
    LexerToken(const LexerToken &other) : token(other.token),
                                          start(other.start),
                                          end(other.end),
                                          line(other.line),
                                          offset(other.offset) {}
    LexerToken &operator=(const LexerToken &other) {
        token = other.token;
        start = other.start;
        end = other.end;
        line = other.line;
        offset = other.offset;
        return *this;
    }

    Token token;
    int start;
    int end;
    int line;
    int offset;
};

QList<LexerToken> tokenize(const char *text, const char *filename);
void dumpTokens(const char *text, const QList<LexerToken> &tokens);

#endif
