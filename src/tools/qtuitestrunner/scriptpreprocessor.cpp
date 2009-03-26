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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "scriptpreprocessor.h"
#include "qscriptsystemtest.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QByteArray>

ScriptPreprocessor::ScriptPreprocessor()
{
    QScriptEngine engine;
    QScriptSystemTest::loadInternalScript("config.js", engine);

    QScriptValue settings = engine.globalObject().property("preprocess");
    if (!settings.isObject()) return;

    /* The documentation for the following settings objects is in config.js */

    {
        QScriptValueIterator it(settings.property("functionAppends"));
        while (it.hasNext()) {
            it.next();
            functionAppends[it.name()] = it.value().toString();
        }
    }
}

void ScriptPreprocessor::preprocess(QString &script)
{
    QString out;
    out.reserve(script.size());

    bool in_singleline_comment  = false;
    bool in_multiline_comment   = false;
    bool in_singlequote_literal = false;
    bool in_doublequote_literal = false;

    const char singlequote = '\'';
    const char doublequote = '"';
    const char brace_open  = '(';
    const char brace_close = ')';
    const char curlybrace_open  = '{';
    const char curlybrace_close = '}';
    const char newline = '\n';
    const char backslash = '\\';
    const char forwardslash = '/';
    const char asterisk = '*';

    QString identifier_chars = "_";
    for (char c = '0'; c <= '9'; ++c) identifier_chars.append(c);
    for (char c = 'a'; c <= 'z'; ++c) identifier_chars.append(c);
    for (char c = 'A'; c <= 'Z'; ++c) identifier_chars.append(c);

    int braces = 0;
    int curlybraces = 0;

    QString function_append;
    int function_append_braces = 0;

    for (int i = 0; i < script.count(); ++i) {
        char c1 = script[i].toLatin1();
        char c2 = script[i+1].toLatin1(); // OK; QByteArray is always null-terminated.

        if (in_singleline_comment) {
            if (newline == c1) {
                in_singleline_comment = false;
                out.append(c1);
                continue;
            }
            out.append(c1);
            continue;
        }

        if (in_multiline_comment) {
            if (asterisk == c1 && forwardslash == c2) {
                in_multiline_comment = false;
                out.append(c1);
                out.append(c2);
                ++i;
                continue;
            }
            out.append(c1);
            continue;
        }

        if (in_singlequote_literal) {
            if (backslash == c1) {
                out.append(c1);
                out.append(c2);
                ++i;
                continue;
            }
            if (singlequote == c1) {
                in_singlequote_literal = false;
                out.append(c1);
                continue;
            }
            out.append(c1);
            continue;
        }

        if (in_doublequote_literal) {
            if (backslash == c1) {
                out.append(c1);
                out.append(c2);
                ++i;
                continue;
            }
            if (doublequote == c1) {
                in_doublequote_literal = false;
                out.append(c1);
                continue;
            }
            out.append(c1);
            continue;
        }

        switch(c1) {

            case singlequote:
                in_singlequote_literal = true;
                out.append(c1);
                continue;

            case doublequote:
                in_doublequote_literal = true;
                out.append(c1);
                continue;

            case forwardslash:
                out.append(c1);
                if (c2 == forwardslash) {
                    in_singleline_comment = true;
                    out.append(c2);
                    ++i;
                }
                if (c2 == asterisk) {
                    in_multiline_comment = true;
                    out.append(c2);
                    ++i;
                }
                continue;

            case brace_open:
                ++braces;
                out.append(c1);
                continue;

            case brace_close:
                --braces;
                out.append(c1);
                if (!function_append.isEmpty() && function_append_braces == braces) {
                    out.append(function_append);
                    function_append = QString();
                }
                continue;

            case curlybrace_open:
                ++curlybraces;
                out.append(c1);
                continue;

            case curlybrace_close:
                --curlybraces;
                out.append(c1);
                continue;

            default:
                // Look ahead to next non-identifier character
                int tok_len;
                for (tok_len = 0; i + tok_len < script.count() && identifier_chars.contains(script[i+tok_len]); ++tok_len) {}
                if (tok_len < 2) {
                    out.append(c1);
                    continue;
                }
                QString tok = script.mid(i, tok_len);

                // Apply preprocessing rules

                // 1. Function appends - text placed immediately after the closing
                // bracket of a function invocation.
                if (functionAppends.contains(tok)) {
                    function_append = functionAppends[tok];
                    function_append_braces = braces;
                }

                out.append(tok);
                i += tok_len - 1;
                continue;
        }
    }

    out.squeeze();

    script = out;
}
