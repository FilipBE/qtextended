/*
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#include "KURL.h"
#include "qurl.h"

namespace WebCore {

static const char hexnumbers[] = "0123456789ABCDEF";
static inline char toHex(char c)
{
    return hexnumbers[c & 0xf];
}

KURL::KURL(const QUrl& url)
{
    *this = KURL(url.toEncoded().constData());
}

KURL::operator QUrl() const
{
    DeprecatedString s = url();

    QByteArray ba;
    ba.reserve(s.length());

    const char *src = s.ascii();
    const char *host = strstr(src, "://");
    if (host)
        host += 3;

    const char *path = host ? strstr(host, "/") : 0;

    for (; *src; ++src) {
        const char chr = *src;

        switch (chr) {
            encode:
            case '{':
            case '}':
            case '|':
            case '\\':
            case '^':
            case '`':
                ba.append('%');
                ba.append(toHex((chr & 0xf0) >> 4));
                ba.append(toHex(chr & 0xf));
                break;
            case '[':
            case ']':
                // special case: if this is the host part, don't encode
                // otherwise, encode
                if (!host || (path && src >= path))
                    goto encode;
                // fall through
            default:
                ba.append(chr);
                break;
        }
    }

    QUrl url = QUrl::fromEncoded(ba);
    return url;
}

}

