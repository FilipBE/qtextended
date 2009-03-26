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

#include "lexer.h"
#include "keywords.cpp"
#include "qoutput.h"

/*!
  \class LexerToken
*/
/*!
  \fn LexerToken::LexerToken()
  \internal
*/
/*!
  \fn LexerToken::LexerToken(const LexerToken&)
  \internal
*/
/*!
  \fn LexerToken::operator=(const LexerToken&)
  \internal
*/
/*!
  \variable LexerToken::token
  \brief foo
*/
/*!
  \variable LexerToken::start
  \brief foo
*/
/*!
  \variable LexerToken::end
  \brief foo
*/
/*!
  \variable LexerToken::line
  \brief foo
*/
/*!
  \variable LexerToken::offset
  \brief foo
*/

/*!
  \relates LexerToken
  Returns a list of lexer tokens in \a text.
  The \a filename is not used.
*/
QList<LexerToken> tokenize(const char *text, const char * /*filename*/)
{
    QList<LexerToken> rv;

    int lineNo = 0;
    int charNo = 0;
    int state = 0;
    int tokenStart = 0;
    bool other = false;

    const char *textprog = text;

    bool done = false;
    while (!done) {
        char textchar = *textprog;
        done = !textchar;

        if (other) {
            if (keywords[state].next[(int)textchar]) {

                // Do other token
                LexerToken token;
                token.token = OTHER;
                token.start = tokenStart;
                token.end = textprog - text - 1;
                token.line = lineNo + 1;
                token.offset = charNo - (token.end - token.start);
                tokenStart = token.end + 1;
                rv.append(token);
                other = false;

            } else {
                goto continue_loop;
            }
        }

        if (keywords[state].next[(int)textchar]) {

            state = keywords[state].next[(int)textchar];

        } else if (0 == state ||
                  keywords[state].token == INCOMPLETE) {

            other = true;
            if (keywords[state].token == INCOMPLETE) {
                state = 0;
                continue;
            }

        } else {

            // Token completed
            Token tokenType = keywords[state].token;
            bool tokenCollapsed = false;
            if (tokenType == CHARACTER ||
               tokenType == DIGIT ||
               tokenType == WHITESPACE) {

                Token lastTokenType =
                    rv.isEmpty()?NOTOKEN:rv.last().token;
                if (tokenType == lastTokenType) {

                    rv.last().end = textprog - text - 1;
                    tokenStart = rv.last().end + 1;

                    tokenCollapsed = true;
                }
            }

            if (!tokenCollapsed) {
                LexerToken token;
                token.token = keywords[state].token;
                token.start = tokenStart;
                token.end = textprog - text - 1;
                token.line = lineNo + 1;
                token.offset = charNo - (token.end - token.start);
                tokenStart = token.end + 1;
                rv.append(token);
            }

            state = keywords[0].next[(int)textchar];
            if (0 == state)
                other = true;
        }

continue_loop:
        // Reset error reporting variables
        if (textchar == '\n') {
            ++lineNo;
            charNo = 0;
        } else {
            charNo++;
        }

        // Increment ptrs
        ++textprog;
    }

    return rv;
}

void dumpTokens(const char *text, const QList<LexerToken> &tokens)
{
    for (int ii = 0; ii < tokens.count(); ++ii) {
        QByteArray ba(text + tokens.at(ii).start, tokens.at(ii).end - tokens.at(ii).start + 1);
        qWarning() << tokens.at(ii).line << ":" << tokens.at(ii).offset << tokenToString(tokens.at(ii).token) << "(" << tokens.at(ii).start << "-" << tokens.at(ii).end << ")" << ba;
    }
}

