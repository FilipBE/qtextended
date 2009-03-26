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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "lexer.h"
#include <QByteArray>

struct PreprocessorToken
{
    PreprocessorToken() : token(NOTOKEN), cStr(0), cStrLen(0),
                          line(-1), offset(-1) {}
    PreprocessorToken(const PreprocessorToken &other) : token(other.token),
                                                        cStr(other.cStr),
                                                        cStrLen(other.cStrLen),
                                                        line(other.line),
                                                        offset(other.offset)
    {}
    PreprocessorToken &operator=(const PreprocessorToken &other) {
        token = other.token;
        cStr = other.cStr;
        cStrLen = other.cStrLen;
        line = other.line;
        offset = other.offset;
        return *this;
    }

    Token token;

    const char *cStr;
    unsigned int cStrLen;
    int line;
    int offset;
};

/*
   Collapses whitespace and strings.

   Adds two new types of tokens:
       * SingleSpace (doesn't refer to any part of the document)
       * String (has a bytearray instead of a document reference)

*/
QList<PreprocessorToken>
preprocess(const char *document, const QList<LexerToken> &tokens);
void dumpTokens(const QList<PreprocessorToken> &tokens);
#endif
