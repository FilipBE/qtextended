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

#include "preprocessor.h"
#include "qoutput.h"

/*!
  \class PreprocessorToken
*/
/*!
  \fn PreprocessorToken::PreprocessorToken()
  \internal
*/
/*!
  \fn PreprocessorToken::PreprocessorToken(const PreprocessorToken&)
  \internal
*/
/*!
  \fn PreprocessorToken::operator=(const PreprocessorToken&)
  \internal
*/
/*!
  \variable PreprocessorToken::token
*/
/*!
  \variable PreprocessorToken::cStr
*/
/*!
  \variable PreprocessorToken::cStrLen
*/
/*!
  \variable PreprocessorToken::line
*/
/*!
  \variable PreprocessorToken::offset
*/

/*!
  \relates PreprocessorToken
  Returns a list of preprocessor tokens in \a document. The \a _tokens value must
  the list of tokens as returned by tokenize().

  The logic used the preprocessing works like this.

  Enter/Leave string when <QUOTE>.  Everything is preserved within a string.
  When not in string:
  \list
  \o Remove all tokens between <HASH> and <NEWLINE>
  \o Simplify <WHITESPACE> such that it is replaced with a <SINGLE_SPACE>
     except around NEWLINE's
  \o Replace all <CHARACTER>[<DIGIT>|<CHARACTER>]* with <NAME>
  \o Remove multiple <NEWLINE>s
  \o Remove all <BACKSLASH><WHITESPACE>*[<NEWLINE>]
  \endlist
*/
QList<PreprocessorToken>
preprocess(const char *document, const QList<LexerToken> &_tokens)
{
    QList<PreprocessorToken> rv;
    bool inString = false;

    // First remove comments to simplify susequent processing
    QList<LexerToken> tokens = _tokens;
    bool whiteSpaceLine = true;
    for (QList<LexerToken>::Iterator iter = tokens.begin();
            iter != tokens.end();
            ) {

        if (iter->token == QUOTE) {
            inString = !inString;
            ++iter;
        } else if (inString) {
            ++iter;
        } else if (iter->token == HASH) {
            // Remove until NEWLINE
            while (iter != tokens.end() && iter->token != NEWLINE)
                iter = tokens.erase(iter);
            if (iter != tokens.end() && whiteSpaceLine)
                iter = tokens.erase(iter); // Erase NEWLINE too
        } else {
            if (iter->token != WHITESPACE)
                whiteSpaceLine = false;
            if (iter->token == NEWLINE)
                whiteSpaceLine = true;
            ++iter;
        }
    }

    // Now do the rest
    for (int ii = 0; ii < tokens.count(); ++ii) {

        const LexerToken &token = tokens.at(ii);

        if (token.token == QUOTE) {

            if (inString && (ii + 1 < tokens.count()) &&
               tokens.at(ii + 1).token == QUOTE) {

                PreprocessorToken newToken;
                newToken.token = token.token;
                newToken.cStr = document + token.start;
                newToken.cStrLen = token.end - token.start + 1;
                newToken.line = token.line;
                newToken.offset = token.offset;
                rv.append(newToken);
                ++ii;

            } else {
                inString = !inString;
                Token stringToken = inString?STRING_START:STRING_END;
                PreprocessorToken newToken;
                newToken.token = stringToken;
                newToken.line = token.line;
                newToken.offset = token.offset;
                rv.append(newToken);
            }

        } else if (inString) {

            // Lexer token is valid
            PreprocessorToken newToken;
            newToken.token = token.token;
            newToken.cStr = document + token.start;
            newToken.cStrLen = token.end - token.start + 1;
            newToken.line = token.line;
            newToken.offset = token.offset;
            rv.append(newToken);

        } else if (token.token == BACKSLASH) {

            // Remove <BACKSLASH><WHITESPACE>*[<NEWLINE>]
            while ((ii + 1) < tokens.count() &&
                  tokens.at(ii + 1).token == WHITESPACE)
                ++ii;
            if ((ii + 1) < tokens.count() && tokens.at(ii + 1).token == NEWLINE)
                ++ii;

        } else if (token.token == WHITESPACE) {

            if (rv.isEmpty() || rv.last().token == SINGLE_SPACE)
                continue; // Remove multiple whitespace's

            PreprocessorToken newToken;
            newToken.token = SINGLE_SPACE;
            newToken.line = token.line;
            newToken.offset = token.offset;
            rv.append(newToken);

        } else if (token.token == CHARACTER) {

            // Name token
            while (++ii < tokens.count()) {
                if (tokens.at(ii).token != CHARACTER &&
                   tokens.at(ii).token != DIGIT)
                    break;
            }

            PreprocessorToken newToken;
            newToken.token = NAME;
            newToken.cStr = document + token.start;
            newToken.cStrLen = tokens.at(ii - 1).end - token.start + 1;
            newToken.line = token.line;
            newToken.offset = token.offset;
            rv.append(newToken);
            --ii; // Counter for increase when we loop

        } else if (token.token == NEWLINE) {
            // Remove spaces
            while (!rv.isEmpty() && rv.last().token == SINGLE_SPACE)
                rv.removeLast();

            if (!rv.isEmpty() && rv.last().token != NEWLINE) {
                PreprocessorToken newToken;
                newToken.token = NEWLINE;
                newToken.line = token.line;
                newToken.offset = token.offset;
                rv.append(newToken);
            }

        } else {
            // Lexer token is valid
            PreprocessorToken newToken;
            newToken.token = token.token;
            newToken.cStr = document + token.start;
            newToken.cStrLen = token.end - token.start + 1;
            newToken.line = token.line;
            newToken.offset = token.offset;
            rv.append(newToken);
        }
    }

    // Remove trailing whitespace
    while (!rv.isEmpty() && rv.last().token == SINGLE_SPACE)
        rv.removeLast();
    // Add final NEWLINE
    if (!rv.isEmpty() && rv.last().token != NEWLINE) {
        PreprocessorToken newToken;
        newToken.token = NEWLINE;
        rv.append(newToken);
    }

    return rv;
}

void dumpTokens(const QList<PreprocessorToken> &tokens)
{
    for (int ii = 0; ii < tokens.count(); ++ii) {
        if (tokens.at(ii).token == NEWLINE ||
           tokens.at(ii).token == STRING_START ||
           tokens.at(ii).token == STRING_END) {
            qWarning() << tokens.at(ii).line << ":" << tokens.at(ii).offset << tokenToString(tokens.at(ii).token);
        } else {
            QByteArray ba(tokens.at(ii).cStr, tokens.at(ii).cStrLen);
            qWarning() << tokens.at(ii).line << ":" << tokens.at(ii).offset << tokenToString(tokens.at(ii).token) << ba;
        }
    }
}

