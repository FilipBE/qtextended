/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the qmake application of the Qt Toolkit.
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

#ifndef XMLOUTPUT_H
#define XMLOUTPUT_H

#include <qtextstream.h>
#include <qstack.h>

QT_BEGIN_NAMESPACE

class XmlOutput
{
public:
    enum ConverstionType {
        NoConversion,       // No change
        EscapeConversion,   // Use '\"'
        XMLConversion       // Use &quot;
    };
    enum XMLFormat {
        NoNewLine,          // No new lines, unless added manually
        NewLine             // All properties & tags indented on new lines
    };
    enum XMLState {
        Bare,               // Not in tag or attribute
        Tag,                // <tagname attribute1="value"
        Attribute           //  attribute2="value">
    };
    enum XMLType {
        tNothing,           // No XML output, and not state change
        tRaw,               // Raw text (no formating)
        tDeclaration,       // <?xml version="x.x" encoding="xxx"?>
        tTag,               // <tagname attribute1="value"
        tCloseTag,          // Closes an open tag
        tAttribute,         //  attribute2="value">
        tData,              // Tag data (formating done)
        tComment,           // <!-- Comment -->
        tCDATA              // <![CDATA[ ... ]]>
    };

    XmlOutput(QTextStream &file, ConverstionType type = XMLConversion);
    ~XmlOutput();

    // Settings
    void setIndentString(const QString &indentString);
    QString indentString();
    void setIndentLevel(int level);
    int indentLevel();
    void setState(XMLState state);
    XMLState state();


    struct xml_output {
        XMLType xo_type;    // Type of struct instance
        QString xo_text;    // Tag/Attribute name/xml version
        QString xo_value;   // Value of attributes/xml encoding

        xml_output(XMLType type, const QString &text, const QString &value)
            : xo_type(type), xo_text(text), xo_value(value) {}
        xml_output(const xml_output &xo)
            : xo_type(xo.xo_type), xo_text(xo.xo_text), xo_value(xo.xo_value) {}
    };

    // Streams
    XmlOutput& operator<<(const QString& o);
    XmlOutput& operator<<(const xml_output& o);

private:
    void increaseIndent();
    void decreaseIndent();
    void updateIndent();

    QString doConversion(const QString &text);

    // Output functions
    void newTag(const QString &tag);
    void newTagOpen(const QString &tag);
    void closeOpen();
    void closeTag();
    void closeTo(const QString &tag);
    void closeAll();

    void addDeclaration(const QString &version, const QString &encoding);
    void addRaw(const QString &rawText);
    void addAttribute(const QString &attribute, const QString &value);
    void addData(const QString &data);

    // Data
    QTextStream &xmlFile;
    QString indent;

    QString currentIndent;
    int currentLevel;
    XMLState currentState;

    XMLFormat format;
    ConverstionType conversion;
    QStack<QString> tagStack;
};

inline XmlOutput::xml_output noxml()
{
    return XmlOutput::xml_output(XmlOutput::tNothing, QString(), QString());
}

inline XmlOutput::xml_output raw(const QString &rawText)
{
    return XmlOutput::xml_output(XmlOutput::tRaw, rawText, QString());
}

inline XmlOutput::xml_output declaration(const QString &version = QString("1.0"),
                                         const QString &encoding = QString())
{
    return XmlOutput::xml_output(XmlOutput::tDeclaration, version, encoding);
}

inline XmlOutput::xml_output decl(const QString &version = QString("1.0"),
                                  const QString &encoding = QString())
{
    return declaration(version, encoding);
}

inline XmlOutput::xml_output tag(const QString &name)
{
    return XmlOutput::xml_output(XmlOutput::tTag, name, QString());
}

inline XmlOutput::xml_output closetag()
{
    return XmlOutput::xml_output(XmlOutput::tCloseTag, QString(), QString());
}

inline XmlOutput::xml_output closetag(const QString &toTag)
{
    return XmlOutput::xml_output(XmlOutput::tCloseTag, toTag, QString());
}

inline XmlOutput::xml_output closeall()
{
    return XmlOutput::xml_output(XmlOutput::tCloseTag, QString(), QString("all"));
}

inline XmlOutput::xml_output attribute(const QString &name,
                                       const QString &value)
{
    return XmlOutput::xml_output(XmlOutput::tAttribute, name, value);
}

inline XmlOutput::xml_output attr(const QString &name,
                                  const QString &value)
{
    return attribute(name, value);
}

inline XmlOutput::xml_output data(const QString &text = QString())
{
    return XmlOutput::xml_output(XmlOutput::tData, text, QString());
}

inline XmlOutput::xml_output comment(const QString &text)
{
    return XmlOutput::xml_output(XmlOutput::tComment, text, QString());
}

inline XmlOutput::xml_output cdata(const QString &text)
{
    return XmlOutput::xml_output(XmlOutput::tCDATA, text, QString());
}

QT_END_NAMESPACE

#endif // XMLOUTPUT_H
