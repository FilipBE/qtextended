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

#include "qscriptsystemtestlog_p.h"
#include <private/qcircularbuffer_p.h>

#include <QStringList>
#include <QVector>

/*
    FIXME: this class shouldn't exist.  The original implementation of this functionality was
    in QtScript.  It leaked memory for unknown reasons (bug 229118).
    Whatever caused the leaks should be found and fixed, at which point this code can be moved
    back to QtScript.
*/

struct LogMessage
{
    LogMessage()
        : raw()
        , text()
        , timestamp(-1)
        , pid(-1)
        , application()
    {}

    QByteArray raw;
    QByteArray text;
    int        timestamp;
    int        pid;
    QByteArray application;
};

struct QScriptSystemTestLogPrivate
{
    LogMessage parse(QByteArray const&);
    LogMessage tryParse(QByteArray const&, bool*);

    QByteArray format;

    QCircularBuffer<LogMessage> messages;
};

QScriptSystemTestLog::QScriptSystemTestLog(QObject* parent)
    : QObject(parent)
    , d(new QScriptSystemTestLogPrivate)
{}

QScriptSystemTestLog::~QScriptSystemTestLog()
{
    delete d;
    d = 0;
}

void QScriptSystemTestLog::addMessages(QList<QByteArray> const& lines)
{
    foreach (QByteArray const& line, lines) {
        d->messages.append( d->parse(line) );
    }
}

void QScriptSystemTestLog::setBufferSize(int limit)
{ d->messages.setLimit(limit); }

void QScriptSystemTestLog::setFormat(QByteArray const& format)
{ d->format = format; }

QByteArray QScriptSystemTestLog::format() const
{ return d->format; }

int QScriptSystemTestLog::count() const
{ return d->messages.count(); }

void QScriptSystemTestLog::clear()
{ return d->messages.clear(); }

LogMessage QScriptSystemTestLogPrivate::parse(QByteArray const& line)
{
    LogMessage msg;

    // Scan from left to right until the line matches the pattern
    bool ok = false;
    QByteArray trimmed = line;
    while (!trimmed.isEmpty() && !ok) {
        msg = tryParse(trimmed, &ok);
        trimmed.remove(0,1);
    }

    if (!ok) {
        msg = LogMessage();
    }

    msg.raw = line;

    return msg;
}

LogMessage QScriptSystemTestLogPrivate::tryParse(QByteArray const& line, bool* ok)
{
    LogMessage msg;
    msg.raw = line;

    bool in_percent = false;
    *ok = true;
    int format_i = 0;
    for (   int line_i = 0;
            format_i < format.length() && line_i < line.length() && *ok;
            ++format_i)
    {
        char c = format.at(format_i);
        const QByteArray remaining = QByteArray::fromRawData(&line.constData()[line_i], line.length() - line_i + 1);

        switch(c) {

        case '%':
            // If we already were processing a %, then this is a real (escaped) %
            if (in_percent) goto plain_ol_character;
            in_percent = true;
            break;

        // %s: actual log message
        case 's':
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            // %s matches everything, so chomp the entire remainder of the line
            msg.text = remaining;
            msg.text.detach();
            line_i     += remaining.length();
            break;

        // %n: application name
        case 'n': {
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            // For simplicity's sake, so we can avoid implementing backtracking, allow
            // an app name to only be alphanumeric and _ .
            msg.application = QByteArray();
            char const* byte = remaining.constData();
            while (byte && (
                        (*byte >= 'a' && *byte <= 'z')
                        ||  (*byte >= 'A' && *byte <= 'Z')
                        ||  (*byte >= '0' && *byte <= '9')
                        ||  (*byte == '_')
                        ))
            {
                msg.application += *byte;
                ++byte;
            }
            if (msg.application.isEmpty()) {
                *ok = false;
            } else {
                line_i += msg.application.length();
            }
            break;
        }

        // %t: monotonic timestamp in milliseconds
        // %p: application pid
        case 't':
        case 'p': {
            if (!in_percent) goto plain_ol_character;
            in_percent = false;
            QByteArray num;
            char const* byte = remaining.constData();
            while (byte && *byte >= '0' && *byte <= '9')
            {
                num += *byte;
                ++byte;
            }
            if (num.isEmpty()) {
                *ok = false;
            } else {
                line_i += num.length();
            }
            if (*ok && c == 't') {
                msg.timestamp = num.toLongLong(ok);
            }
            if (*ok && c == 'p') {
                msg.pid = num.toLong(ok);
            }

            break;
        }

        default:
        plain_ol_character:
            in_percent = false;
            if (c == remaining.at(0)) {
                ++line_i;
            } else {
                *ok = false;
            }
            break;

        }

    }

    // If we ran out of text before consuming the format, the message didn't match
    if (format_i < format.length()) *ok = false;

    return msg;
}

QVariantMap QScriptSystemTestLog::message(int i) const
{
    LogMessage const& msg = d->messages.at(i);
    QVariantMap map;
    map["raw"]  = QString::fromLatin1(msg.raw);
    if (!msg.text.isEmpty())        map["text"]        = QString::fromLatin1(msg.text);
    if (msg.timestamp != -1)        map["timestamp"]   = msg.timestamp;
    if (msg.pid != -1)              map["pid"]         = msg.pid;
    if (!msg.application.isEmpty()) map["application"] = QString::fromLatin1(msg.application);
    return map;
}

