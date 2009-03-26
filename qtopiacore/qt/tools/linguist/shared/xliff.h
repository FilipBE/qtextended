/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "metatranslator.h"

#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtXml/QXmlAttributes>
#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlParseException>

QT_BEGIN_NAMESPACE

class XLIFFHandler : public QXmlDefaultHandler
{
public:
    XLIFFHandler( MetaTranslator *translator );

    virtual bool startElement( const QString& namespaceURI,
                               const QString& localName, const QString& qName,
                               const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
                             const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

    virtual bool endDocument();

private:
    enum XliffContext {
        XC_xliff,
        XC_group,
        XC_context_group,
        XC_context,
        XC_context_linenumber,
        XC_ph,
        XC_restype_plurals
    };
    void pushContext(XliffContext ctx);
    bool popContext(XliffContext ctx);
    XliffContext currentContext() const;
    bool hasContext(XliffContext ctx) const;

private:
    MetaTranslator *tor;
    TranslatorMessage::Type m_type;
    QString m_language;
    QString m_context;
    QString m_source;
    QString m_comment;
    QStringList translations;
    QString m_fileName;
    int     m_lineNumber;

    QString accum;
    QString m_ctype;
    int ferrorCount;
    bool contextIsUtf8;
    bool messageIsUtf8;
    const QString m_URI;  // convenience and efficiency; urn:oasis:names:tc:xliff:document:1.1
    const QString m_URI12;  // convenience and efficiency; urn:oasis:names:tc:xliff:document:1.1
    QStack<int> m_contextStack;
};

QT_END_NAMESPACE
