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

#ifndef QWBXMLREADER_H
#define QWBXMLREADER_H

#include <qmap.h>
#include <qxml.h>

#include <qtopiaglobal.h>

typedef QMap<int, QString> QWbXmlTagSet;


class QWbXmlReaderPrivate;
class QWbXmlToXmlContentHandlerPrivate;


class QTOPIAPHONE_EXPORT QWbXmlReader : public QXmlReader
{
public:
    QWbXmlReader();
    QWbXmlReader( const QWbXmlTagSet& tags, const QWbXmlTagSet& attrs );
    virtual ~QWbXmlReader();

    bool feature( const QString& name, bool *ok = 0 ) const;
    void setFeature( const QString& name, bool value );
    bool hasFeature( const QString& name ) const;

    void* property( const QString& name, bool *ok = 0 ) const;
    void setProperty( const QString& name, void* value );
    bool hasProperty( const QString& name ) const;

    void setEntityResolver( QXmlEntityResolver* handler );
    QXmlEntityResolver* entityResolver() const;
    void setDTDHandler( QXmlDTDHandler* handler );
    QXmlDTDHandler* DTDHandler() const;
    void setContentHandler( QXmlContentHandler* handler );
    QXmlContentHandler* contentHandler() const;
    void setErrorHandler( QXmlErrorHandler* handler );
    QXmlErrorHandler* errorHandler() const;
    void setLexicalHandler( QXmlLexicalHandler* handler );
    QXmlLexicalHandler* lexicalHandler() const;
    void setDeclHandler( QXmlDeclHandler* handler );
    QXmlDeclHandler* declHandler() const;

    // Note: the source input must be base64-encoded when calling this.
    //obsolete
    bool parse( const QXmlInputSource& ) {
        qFatal("WBXMLReader::parse(const QXmlInputSource&) is obsolete. Use WBXMLReader::parse(const QXmlInputSource*).");
        return false;
    }
    bool parse( const QXmlInputSource* input );

    // The following are supplied binary inputs.
    bool parse( const QByteArray& input );
    bool parse( QIODevice& input );

    void setTagSets( const QWbXmlTagSet& tags, const QWbXmlTagSet& attrs );

    QString toXml( const QByteArray& input );
    QString toXml( QIODevice& input );

protected:
    virtual QString resolveOpaque( const QString& attr, const QByteArray& data );

private:
    QWbXmlReaderPrivate *d;

    void nextToken();
    int readByte();
    int readInt();
    QString readString();
    QString getIndexedString( int index );
    void parseElementBody();
    void parseAttributes( QXmlAttributes& attrs );
};


class QTOPIAPHONE_EXPORT QWbXmlToXmlContentHandler : public QXmlContentHandler
{
public:
    QWbXmlToXmlContentHandler();
    virtual ~QWbXmlToXmlContentHandler();

    void setDocumentLocator( QXmlLocator* locator );
    bool startDocument();
    bool endDocument();
    bool startPrefixMapping( const QString& prefix, const QString& uri );
    bool endPrefixMapping( const QString& prefix );
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );
    bool ignorableWhitespace( const QString& ch );
    bool processingInstruction( const QString& target, const QString& data );
    bool skippedEntity( const QString& name );
    QString errorString() const;

    QString toString() const;

private:
    QWbXmlToXmlContentHandlerPrivate *d;

    void addQuoted( const QString& value );
    void addIndent();
};

#endif
