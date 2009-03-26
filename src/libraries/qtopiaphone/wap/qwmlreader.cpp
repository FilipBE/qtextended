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

#include <qwmlreader.h>

/*!
    \class QWmlReader
    \inpublicgroup QtTelephonyModule

    \brief The QWmlReader class reads Wireless Markup Language (WML) documents

    The QWmlReader class extends QWbXmlReader to provide support for reading
    documents in the Wireless Markup Language (WML).  This language is defined in
    the WAP standard
    \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-238-wml-20010911-a.pdf}{wap-238-wml-20010911-a.pdf}.

    \ingroup telephony
    \sa QWbXmlReader
*/

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    Construct a WBXML reader and initialize it to process Wireless Markup
    Language (WML) documents.
*/
QWmlReader::QWmlReader()
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    set(tags, 0x1C, "a");
    set(tags, 0x1D, "td");
    set(tags, 0x1E, "tr");
    set(tags, 0x1F, "table");
    set(tags, 0x20, "p");
    set(tags, 0x21, "postfield");
    set(tags, 0x22, "anchor");
    set(tags, 0x23, "access");
    set(tags, 0x24, "b");
    set(tags, 0x25, "big");
    set(tags, 0x26, "br");
    set(tags, 0x27, "card");
    set(tags, 0x28, "do");
    set(tags, 0x29, "em");
    set(tags, 0x2A, "fieldset");
    set(tags, 0x2B, "go");
    set(tags, 0x2C, "head");
    set(tags, 0x2D, "i");
    set(tags, 0x2E, "img");
    set(tags, 0x2F, "input");
    set(tags, 0x30, "meta");
    set(tags, 0x31, "noop");
    set(tags, 0x32, "prev");
    set(tags, 0x33, "onevent");
    set(tags, 0x34, "optgroup");
    set(tags, 0x35, "option");
    set(tags, 0x36, "refresh");
    set(tags, 0x37, "select");
    set(tags, 0x38, "small");
    set(tags, 0x39, "strong");
    set(tags, 0x3B, "template");
    set(tags, 0x3C, "timer");
    set(tags, 0x3D, "u");
    set(tags, 0x3E, "setvar");
    set(tags, 0x3F, "wml");

    set(attrs, 0x05, "accept-charset");
    set(attrs, 0x06, "align=bottom");
    set(attrs, 0x07, "align=center");
    set(attrs, 0x08, "align=left");
    set(attrs, 0x09, "align=middle");
    set(attrs, 0x0A, "align=right");
    set(attrs, 0x0B, "align=top");
    set(attrs, 0x0C, "alt");
    set(attrs, 0x0D, "content");
    set(attrs, 0x0F, "domain");
    set(attrs, 0x10, "emptyok=false");
    set(attrs, 0x11, "emptyok=true");
    set(attrs, 0x12, "format");
    set(attrs, 0x13, "height");
    set(attrs, 0x14, "hspace");
    set(attrs, 0x15, "ivalue");
    set(attrs, 0x16, "iname");
    set(attrs, 0x18, "label");
    set(attrs, 0x19, "localsrc");
    set(attrs, 0x1A, "maxlength");
    set(attrs, 0x1B, "method=get");
    set(attrs, 0x1C, "method=post");
    set(attrs, 0x1D, "mode=nowrap");
    set(attrs, 0x1E, "mode=wrap");
    set(attrs, 0x1F, "multiple=false");
    set(attrs, 0x20, "multiple=true");
    set(attrs, 0x21, "name");
    set(attrs, 0x22, "newcontext=false");
    set(attrs, 0x23, "newcontext=true");
    set(attrs, 0x24, "onpick");
    set(attrs, 0x25, "onenterbackward");
    set(attrs, 0x26, "onenterforward");
    set(attrs, 0x27, "ontimer");
    set(attrs, 0x28, "optional=false");
    set(attrs, 0x29, "optional=true");
    set(attrs, 0x2A, "path");
    set(attrs, 0x2E, "scheme");
    set(attrs, 0x2F, "sendreferer=false");
    set(attrs, 0x30, "sendreferer=true");
    set(attrs, 0x31, "size");
    set(attrs, 0x32, "src");
    set(attrs, 0x33, "ordered=true");
    set(attrs, 0x34, "ordered=false");
    set(attrs, 0x35, "tabindex");
    set(attrs, 0x36, "title");
    set(attrs, 0x37, "type");
    set(attrs, 0x38, "type=accept");
    set(attrs, 0x39, "type=delete");
    set(attrs, 0x3A, "type=help");
    set(attrs, 0x3B, "type=password");
    set(attrs, 0x3C, "type=onpick");
    set(attrs, 0x3D, "type=onenterbackward");
    set(attrs, 0x3E, "type=onenterforward");
    set(attrs, 0x3F, "type=ontimer");
    set(attrs, 0x45, "type=options");
    set(attrs, 0x46, "type=prev");
    set(attrs, 0x47, "type=reset");
    set(attrs, 0x48, "type=text");
    set(attrs, 0x49, "type=vnd.");
    set(attrs, 0x4A, "href");
    set(attrs, 0x4B, "href=http://");
    set(attrs, 0x4C, "href=https://");
    set(attrs, 0x4D, "value");
    set(attrs, 0x4E, "vspace");
    set(attrs, 0x4F, "width");
    set(attrs, 0x50, "xml:lang");
    set(attrs, 0x52, "align");
    set(attrs, 0x53, "columns");
    set(attrs, 0x54, "class");
    set(attrs, 0x55, "id");
    set(attrs, 0x56, "forua=false");
    set(attrs, 0x57, "forua=true");
    set(attrs, 0x58, "src=http://");
    set(attrs, 0x59, "src=https://");
    set(attrs, 0x5A, "http-equiv");
    set(attrs, 0x5B, "http-equiv=Content-Type");
    set(attrs, 0x5C, "content=application/vnd.wap.wmlc;charset=");
    set(attrs, 0x5D, "http-equiv=Expires");
    set(attrs, 0x85, ".com/");
    set(attrs, 0x86, ".edu/");
    set(attrs, 0x87, ".net/");
    set(attrs, 0x88, ".org/");
    set(attrs, 0x89, "accept");
    set(attrs, 0x8A, "bottom");
    set(attrs, 0x8B, "clear");
    set(attrs, 0x8C, "delete");
    set(attrs, 0x8D, "help");
    set(attrs, 0x93, "middle");
    set(attrs, 0x94, "nowrap");
    set(attrs, 0x95, "onpick");
    set(attrs, 0x96, "onenterbackward");
    set(attrs, 0x97, "onenterforward");
    set(attrs, 0x98, "ontimer");
    set(attrs, 0x99, "options");
    set(attrs, 0x9A, "password");
    set(attrs, 0x9B, "reset");
    set(attrs, 0x9E, "top");
    set(attrs, 0x9F, "unknown");
    set(attrs, 0xA0, "wrap");
    set(attrs, 0xA1, "www.");

    setTagSets( tags, attrs );
}

/*!
    Destruct a WML reader.
*/
QWmlReader::~QWmlReader()
{
}
