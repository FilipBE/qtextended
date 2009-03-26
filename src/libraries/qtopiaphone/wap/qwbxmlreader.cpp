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

#include <qwbxmlreader.h>
#include <qtextcodec.h>
#include <qbuffer.h>

// Standard WBXML control codes.
#define WBXML_SWITCH_PAGE       0x00
#define WBXML_END               0x01
#define WBXML_ENTITY            0x02
#define WBXML_STR_I             0x03
#define WBXML_LITERAL           0x04
#define WBXML_EXT_I_0           0x40
#define WBXML_EXT_I_1           0x41
#define WBXML_EXT_I_2           0x42
#define WBXML_PI                0x43
#define WBXML_LITERAL_C         0x44
#define WBXML_EXT_T_0           0x80
#define WBXML_EXT_T_1           0x81
#define WBXML_EXT_T_2           0x82
#define WBXML_STR_T             0x83
#define WBXML_LITERAL_A         0x84
#define WBXML_EXT_0             0xC0
#define WBXML_EXT_1             0xC1
#define WBXML_EXT_2             0xC2
#define WBXML_OPAQUE            0xC3
#define WBXML_LITERAL_AC        0xC4
#define WBXML_HAS_ATTRS         0x80
#define WBXML_HAS_CONTENT       0x40
#define WBXML_TAG_MASK          0x3F

// Standard publicid's for WAP-related DTD's.
static const char * const builtin_publicids[] = {
    "", "",     // 0 and 1 are reserved
    "-//WAPFORUM//DTD WML 1.0//EN",
    "-//WAPFORUM//DTD WTA 1.0//EN",
    "-//WAPFORUM//DTD WML 1.1//EN",
    "-//WAPFORUM//DTD SI 1.0//EN",
    "-//WAPFORUM//DTD SL 1.0//EN",
    "-//WAPFORUM//DTD CO 1.0//EN",
    "-//WAPFORUM//DTD CHANNEL 1.1//EN",
    "-//WAPFORUM//DTD WML 1.2//EN",
    "-//WAPFORUM//DTD WML 1.3//EN",
    "-//WAPFORUM//DTD PROV 1.0//EN",
    "-//WAPFORUM//DTD WTA-WML 1.2//EN",
    "-//WAPFORUM//DTD CHANNEL 1.2//EN"
};
#define num_builtin_publicids   \
    (sizeof(builtin_publicids) / sizeof(const char *))

// Tokens that may be encountered in WBXML input.
enum QWbXmlToken
{
    QWbXmlToken_EOF,
    QWbXmlToken_Tag,
    QWbXmlToken_AttrStart,
    QWbXmlToken_EndTag,
    QWbXmlToken_EndAttrs,
    QWbXmlToken_PI,
    QWbXmlToken_String

};

class QWbXmlReaderPrivate
{
public:
    QWbXmlReaderPrivate()
    {
        this->entityResolver = 0;
        this->DTDHandler = 0;
        this->contentHandler = 0;
        this->errorHandler = 0;
        this->lexicalHandler = 0;
        this->declHandler = 0;
    }

    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    QXmlEntityResolver* entityResolver;
    QXmlDTDHandler* DTDHandler;
    QXmlContentHandler* contentHandler;
    QXmlErrorHandler* errorHandler;
    QXmlLexicalHandler* lexicalHandler;
    QXmlDeclHandler* declHandler;

    QIODevice *input;
    QWbXmlToken tokenType;
    QString tokenValue;
    int tagPage;
    int attrPage;
    bool tagMode;
    bool hasContent;
    QByteArray stringTable;
    QTextCodec *codec;
    bool sawEOF;
    QString currentAttr;
};

/*!
    \class QWbXmlReader
    \inpublicgroup QtTelephonyModule

    \brief The QWbXmlReader class reads the contents of a Wireless Binary XML (WBXML) stream

    The QWbXmlReader base class provides an interface to parse
    Wireless Binary XML (WBXML) data streams according to the WAP
    standard \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-192-wbxml-20010725-a.pdf}{wap-192-wbxml-20010725-a.pdf}.  The data
    appears as an XML object that can be navigated using the
    facilities in QXmlReader.

    WBXML files use binary numbers to indicate the names of tags
    and attributes.  The user, usually a subclass, is responsible
    for providing the mappings from binary numbers to names.

    There is no standard list of tag mappings: each format that is
    based on WBXML has its own set.  The subclasses WmlReader and
    OtaReader provide two common sets, for the Wireless Markup
    Language (WML), and Over-The-Air (OTA) Network Configuration
    messages respectively.

    \ingroup telephony
    \sa QWmlReader, QOtaReader
*/

/*!
    Construct a new WBXML reader object, with no default tag sets.
*/
QWbXmlReader::QWbXmlReader()
{
    d = new QWbXmlReaderPrivate();
}

/*!
    Construct a new WBXML reader object, with specific tag sets,
    \a tags and \a attrs.
*/
QWbXmlReader::QWbXmlReader( const QWbXmlTagSet& tags, const QWbXmlTagSet& attrs )
{
    d = new QWbXmlReaderPrivate();
    setTagSets( tags, attrs );
}

/*!
    Destruct this WBXML reader.
*/
QWbXmlReader::~QWbXmlReader()
{
    delete d;
}

/*!
    Check for feature \a name and return its value.  Also return whether
    the feature exists in \a ok.
*/
bool QWbXmlReader::feature( const QString& , bool *ok ) const
{
    if ( ok )
        *ok = false;
    return false;
}

/*!
    Set the feature \a name to \a value.
*/
void QWbXmlReader::setFeature( const QString& , bool )
{
}

/*!
    Determine if this reader has the feature \a name.
*/
bool QWbXmlReader::hasFeature( const QString& ) const
{
    return false;
}

/*!
    Check for property \a name and return its value.  Also return whether
    the property exists in \a ok.
*/
void* QWbXmlReader::property( const QString& , bool *ok ) const
{
    if ( ok )
        *ok = false;
    return 0;
}

/*!
    Set the property \a name to \a value.
*/
void QWbXmlReader::setProperty( const QString& , void* )
{
}

/*!
    Determine if this reader has the property \a name.
*/
bool QWbXmlReader::hasProperty( const QString& ) const
{
    return false;
}

/*!
    Set the entity resolver to \a handler.
*/
void QWbXmlReader::setEntityResolver( QXmlEntityResolver* handler )
{
    d->entityResolver = handler;
}

/*!
    Return the current entity resolver.
*/
QXmlEntityResolver* QWbXmlReader::entityResolver() const
{
    return d->entityResolver;
}

/*!
    Set the DTD \a handler.
*/
void QWbXmlReader::setDTDHandler( QXmlDTDHandler* handler )
{
    d->DTDHandler = handler;
}

/*!
    Return the current DTD handler.
*/
QXmlDTDHandler* QWbXmlReader::DTDHandler() const
{
    return d->DTDHandler;
}

/*!
    Set the content \a handler.
*/
void QWbXmlReader::setContentHandler( QXmlContentHandler* handler )
{
    d->contentHandler = handler;
}

/*!
    Return the current content handler.
*/
QXmlContentHandler* QWbXmlReader::contentHandler() const
{
    return d->contentHandler;
}

/*!
    Set the error \a handler.
*/
void QWbXmlReader::setErrorHandler( QXmlErrorHandler* handler )
{
    d->errorHandler = handler;
}

/*!
    Return the current error handler.
*/
QXmlErrorHandler* QWbXmlReader::errorHandler() const
{
    return d->errorHandler;
}

/*!
    Set the lexical \a handler.
*/
void QWbXmlReader::setLexicalHandler( QXmlLexicalHandler* handler )
{
    d->lexicalHandler = handler;
}

/*!
    Return the current lexical handler.
*/
QXmlLexicalHandler* QWbXmlReader::lexicalHandler() const
{
    return d->lexicalHandler;
}

/*!
    Set the declaration \a handler.
*/
void QWbXmlReader::setDeclHandler( QXmlDeclHandler* handler )
{
    d->declHandler = handler;
}

/*!
    Return the current declaration handler.
*/
QXmlDeclHandler* QWbXmlReader::declHandler() const
{
    return d->declHandler;
}

/*!
    \fn bool QWbXmlReader::parse( const QXmlInputSource& input )

    This function is obsolete.  It parses the data from \a input.
*/

/*!
    Parse the specified QXmlInputSource "input" object.  Use of this
    function is not recommended, because QXmlInputSource is string-based,
    whereas WBXML requires binary input.  To preserve the binary data,
    the data within "input" should be base64-encoded before calling
    this function.
*/
bool QWbXmlReader::parse( const QXmlInputSource* input )
{
    // QXmlInputSource is string-based, which will have problems with
    // binary WBXML data.  So, we assume that the caller has base64-encoded
    // the data before passing it to us.
    QByteArray data = input->data().toUtf8();
    QByteArray decoded = QByteArray::fromBase64( data );
    return parse( decoded );
}

/*!
    Parse the WBXML data in the "input" array.
*/
bool QWbXmlReader::parse( const QByteArray& input )
{
    QByteArray data( input );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    return parse( buffer );
}

/*!
    Parse the WBXML data from the specified "input" device.
*/
bool QWbXmlReader::parse( QIODevice& input )
{
    // The parse process is pointless if there is no content handler.
    if ( ! d->contentHandler )
        return false;

    // Initialize the token parsing state.
    d->input = &input;
    d->tokenType = QWbXmlToken_Tag;
    d->tokenValue = QString();
    d->tagPage = 0;
    d->attrPage = 0;
    d->tagMode = true;
    d->hasContent = false;
    d->codec = 0;
    d->sawEOF = false;

    // Start the document.
    if ( ! d->contentHandler->startDocument() )
        return false;

    // Parse the header information.
    readByte();             // Skip the version number.
    int publicid = readInt();
    int publicindex = -1;
    if ( publicid == 0 )
        publicindex = readInt();
    int charset = readInt();
    if ( charset <= 0 )
        charset = 106;      // Default to UTF-8.
    d->codec = QTextCodec::codecForMib( charset );
    if ( !(d->codec) )
        d->codec = QTextCodec::codecForMib( 106 );

    // Load the string table.
    int len = readInt();
    if ( len > 0 ) {
        d->stringTable.resize( (uint)len );
        if ( input.read( d->stringTable.data(), (uint)len ) != len ) {
            return false;
        } else if ( d->stringTable.data()[len - 1] != 0x00 ) {
            return false;
        }
    }

    // Notify the lexical handler of the document type's publicid.
    if ( d->lexicalHandler ) {
        QString id;
        if ( publicid == 0 )
            id = getIndexedString( publicindex );
        else if ( publicid >= 2 && publicid < (int)num_builtin_publicids )
            id = builtin_publicids[publicid];
        if ( !id.isEmpty() ) {
            if ( ! d->lexicalHandler->startDTD
                    ( QString(), id, QString() ) ) {
                return false;
            }
            if ( ! d->lexicalHandler->endDTD() ) {
                return false;
            }
        }
    }

    // Parse the top-level document element body.
    nextToken();
    parseElementBody();

    // End the document.
    return d->contentHandler->endDocument();
}

/*!
    Set the mappings to be used to parse tag and attribute numbers
    to \a tags and \a attrs respectively.
*/
void QWbXmlReader::setTagSets( const QWbXmlTagSet& tags, const QWbXmlTagSet& attrs )
{
    d->tags = tags;
    d->attrs = attrs;
}

/*!
    Parse the contents of \a input and convert it into regular XML text.
*/
QString QWbXmlReader::toXml( const QByteArray& input )
{
    QXmlContentHandler *oldHandler = contentHandler();
    QWbXmlToXmlContentHandler handler;
    setContentHandler( &handler );
    parse( input );
    setContentHandler( oldHandler );
    return handler.toString();
}

/*!
    Parse the contents of \a input and convert it into regular XML text.
*/
QString QWbXmlReader::toXml( QIODevice& input )
{
    QXmlContentHandler *oldHandler = contentHandler();
    QWbXmlToXmlContentHandler handler;
    setContentHandler( &handler );
    parse( input );
    setContentHandler( oldHandler );
    return handler.toString();
}

/*!
    Resolve an OPAQUE data blob within WBXML input into its string version.
    The \a attr parameter is the name of the attribute containing the OPAQUE
    data to be resolved, and the \a data parameter contains the data to
    be resolved.  The \a attr will be QString() for OPAQUE data blobs
    within element bodies.
*/
QString QWbXmlReader::resolveOpaque( const QString&, const QByteArray& )
{
    return QString("");
}

void QWbXmlReader::nextToken()
{
    char ch;
    int fullTag;

again:
    // Bail out if we already saw EOF previously.
    if ( d->sawEOF )
        return;

    // Get the next character and check for EOF.
    if ( ! ( d->input->getChar(&ch) ) ) {
        d->tokenType = QWbXmlToken_EOF;
        d->sawEOF = true;
        return;
    }

    // Determine the kind of token that we have.
    switch (ch) {

        case WBXML_SWITCH_PAGE:
        {
            if ( d->tagMode )
                d->tagPage = readByte() << 6;
            else
                d->attrPage = readByte() << 8;
            goto again;
        }
        // Not reached.

        case WBXML_END:
        {
            // End of current tag or attribute context.
            if ( d->tagMode ) {
                d->tokenType = QWbXmlToken_EndTag;
            } else if ( d->hasContent ) {
                d->tokenType = QWbXmlToken_EndAttrs;
                d->tagMode = true;
            } else {
                d->tokenType = QWbXmlToken_EndTag;
                d->tagMode = true;
            }
        }
        break;

        case WBXML_ENTITY:
        {
            // Unicode character entity.
            d->tokenType = QWbXmlToken_String;
            d->tokenValue = QString( QChar( (ushort)readInt() ) );
        }
        break;

        case WBXML_STR_I:
        {
            // Inline string value.
            d->tokenType = QWbXmlToken_String;
            d->tokenValue = readString();
        }
        break;

        case WBXML_EXT_I_0:
        case WBXML_EXT_I_1:
        case WBXML_EXT_I_2:
        {
            // Inline string extension token - not used at present.
            readString();
            goto again;
        }
        // Not reached.

        case WBXML_PI:
        {
            // Processing instruction, which is followed by attributes.
            d->tokenType = QWbXmlToken_PI;
            d->tagMode = false;
            d->hasContent = false;
        }
        break;

        case WBXML_EXT_T_0:
        case WBXML_EXT_T_1:
        case WBXML_EXT_T_2:
        {
            // Integer-based extension token - not used at present.
            readInt();
            goto again;
        }
        // Not reached.

        case WBXML_STR_T:
        {
            // Reference into the string table.
            d->tokenType = QWbXmlToken_String;
            d->tokenValue = getIndexedString( readInt() );
        }
        break;

        case WBXML_EXT_0:
        case WBXML_EXT_1:
        case WBXML_EXT_2:
        {
            // Byte-based extension token - not used at present.
            goto again;
        }
        // Not reached.

        case WBXML_OPAQUE:
        {
            // Opaque binary data - read and resolve it.
            QByteArray opaque;
            int len = readInt();
            while ( len > 0 && !(d->sawEOF) ) {
                if ( ! d->input->getChar(&ch) ) {
                    d->sawEOF = true;
                } else {
                    opaque.resize( opaque.size() + 1 );
                    opaque[opaque.size() - 1] = ch;
                }
                --len;
            }
            d->tokenType = QWbXmlToken_String;
            d->tokenValue = resolveOpaque( d->currentAttr, opaque );
        }
        break;

        default:
        {
            if ( d->tagMode ) {
                // Handle tag names.
                if ( (ch & WBXML_TAG_MASK) == WBXML_LITERAL ) {
                    d->tokenValue = getIndexedString( readInt() );
                } else {
                    fullTag = (ch & WBXML_TAG_MASK) | d->tagPage;
                    if ( d->tags.contains( fullTag ) ) {
                        d->tokenValue = d->tags[fullTag];
                    } else {
                        d->tokenValue = "x-tag-" + QString::number( fullTag );
                    }
                }
                d->tokenType = QWbXmlToken_Tag;
                if ( ( ch & WBXML_HAS_ATTRS ) != 0 ) {
                    d->tagMode = false;
                }
                if ( ( ch & WBXML_HAS_CONTENT ) != 0 ) {
                    d->hasContent = true;
                } else {
                    d->hasContent = false;
                }
            } else {
                // Handle attribute names.
                if ( ch == WBXML_LITERAL ) {
                    d->tokenType = QWbXmlToken_AttrStart;
                    d->tokenValue = getIndexedString( readInt() );
                } else {
                    fullTag = ( ch & 0xFF ) | d->attrPage;
                    if ( d->attrs.contains( fullTag ) ) {
                        d->tokenValue = d->attrs[fullTag];
                    } else {
                        d->tokenValue = "x-attr-" + QString::number( fullTag );
                    }
                    if ( ( ch & 0x80 ) == 0 )
                        d->tokenType = QWbXmlToken_AttrStart;
                    else
                        d->tokenType = QWbXmlToken_String;
                }
            }
        }
        break;
    }
}

int QWbXmlReader::readByte()
{
    char ch;
    if ( d->input->getChar(&ch) )
        return int(ch) & 0xFF;
    d->sawEOF = true;
    return 0;
}

int QWbXmlReader::readInt()
{
    int value = 0;
    char ch;
    while ( d->input->getChar(&ch) ) {
        value = (value << 7) | (ch & 0x7F);
        if ( (ch & 0x80) == 0 ) {
            return value;
        }
    }
    d->sawEOF = true;
    return value;
}

QString QWbXmlReader::readString()
{
    QByteArray str;
    char ch;
    while ( !(d->sawEOF) ) {
        if ( ! d->input->getChar(&ch) ) {
            d->sawEOF = true;
        } else if ( ch == 0 ) {
            break;
        } else {
            str.append(ch);
        }
    }
    return d->codec->toUnicode( str.data(), str.length() );
}

QString QWbXmlReader::getIndexedString( int index )
{
    if ( index < 0 || index >= (int)( d->stringTable.count() ) ) {
        return QString();
    } else {
        return d->codec->toUnicode( d->stringTable.data() + index,
                                    qstrlen( d->stringTable.data() + index ) );
    }
}

void QWbXmlReader::parseElementBody()
{
    QString name;
    QXmlAttributes attrs;
    bool hasAttrs;
    bool hasContent;

    while ( d->tokenType != QWbXmlToken_EOF) {
        switch ( d->tokenType ) {

            case QWbXmlToken_Tag:
            {
                name = d->tokenValue;
                hasAttrs = !d->tagMode;
                hasContent = d->hasContent;
                nextToken();
                attrs.clear();
                if ( hasAttrs )
                    parseAttributes( attrs );
                d->contentHandler->startElement
                    ( QString(), name, name, attrs );
                if ( hasContent )
                    parseElementBody();
                d->contentHandler->endElement( QString(), name, name );
            }
            break;

            case QWbXmlToken_EndTag:
            {
                nextToken();
                return;
            }
            // Not reached.

            case QWbXmlToken_PI:
            {
                nextToken();
                attrs.clear();
                parseAttributes( attrs );
                if ( attrs.length() > 0 ) {
                    d->contentHandler->processingInstruction
                        ( attrs.localName(0), attrs.value(0) );
                }
            }
            break;

            case QWbXmlToken_String:
            {
                d->contentHandler->characters( d->tokenValue );
                nextToken();
            }
            break;

            default:
            {
                nextToken();
            }
            break;
        }
    }
}

void QWbXmlReader::parseAttributes( QXmlAttributes& attrs )
{
    QString name;
    QString value;
    int index;

    d->currentAttr = QString();

    for (;;) {
        switch ( d->tokenType ) {

            case QWbXmlToken_EOF:
            case QWbXmlToken_EndTag:
            case QWbXmlToken_EndAttrs:
            {
                if ( !name.isEmpty() ) {
                    attrs.append( name, QString(), name, value );
                }
                nextToken();
                return;
            }
            // Not reached.

            case QWbXmlToken_AttrStart:
            {
                if ( !name.isEmpty() ) {
                    attrs.append( name, QString(), name, value );
                }
                d->currentAttr = name;
                name = d->tokenValue;
                index = name.indexOf(QChar('='));
                if ( index != -1 ) {
                    // Attribute value prefix of the form "name=prefix".
                    value = name.mid( index + 1);
                    name = name.left( index );
                } else {
                    // Just the attribute name, with no prefix.
                    value = "";
                }
                nextToken();
            }
            break;

            case QWbXmlToken_String:
            {
                value += d->tokenValue;
                nextToken();
            }
            break;

            default:
            {
                nextToken();
            }
            break;
        }
    }

    d->currentAttr = QString();
}

/*!
    \class QWbXmlToXmlContentHandler
    \inpublicgroup QtTelephonyModule

    \brief The QWbXmlToXmlContentHandler class assists with converting WBXML documents into ordinary XML.

    The QWbXmlToXmlContentHandler class assists with converting WBXML
    documents into ordinary XML.  It is used by QWbXmlReader::toXml().

    \ingroup telephony
    \sa QWbXmlReader
*/

class QWbXmlToXmlContentHandlerPrivate
{
public:
    QWbXmlToXmlContentHandlerPrivate()
    {
        lastWasTag = false;
        indent = 0;
    }

    QString result;
    bool lastWasTag;
    int indent;
};

/*!
    Construct a new WBXML to XML content conversion handler.
*/
QWbXmlToXmlContentHandler::QWbXmlToXmlContentHandler()
{
    d = new QWbXmlToXmlContentHandlerPrivate();
}

/*!
    Destruct a WBXML to XML content conversion handler.
*/
QWbXmlToXmlContentHandler::~QWbXmlToXmlContentHandler()
{
}

/*!
    Set the document \a locator.
*/
void QWbXmlToXmlContentHandler::setDocumentLocator( QXmlLocator* )
{
    // Nothing to do here.
}

/*!
    Start parsing the document.
*/
bool QWbXmlToXmlContentHandler::startDocument()
{
    d->result += "<?xml version=\"1.0\"?>\n";
    return true;
}

/*!
    End parsing the document.
*/
bool QWbXmlToXmlContentHandler::endDocument()
{
    d->result += "\n";
    return false;
}

/*!
    Start a namespace \a prefix mapping for \a uri.
*/
bool QWbXmlToXmlContentHandler::startPrefixMapping( const QString& , const QString& )
{
    return true;
}

/*!
    End a namspace \a prefix mapping.
*/
bool QWbXmlToXmlContentHandler::endPrefixMapping( const QString& )
{
    return true;
}

/*!
    Start parsing an element called \a qName, which can be be broken up
    into \a namespaceURI and \a localName.  The attribute list is \a atts.
*/
bool QWbXmlToXmlContentHandler::startElement( const QString& , const QString& , const QString& qName, const QXmlAttributes& atts )
{
    if ( d->lastWasTag ) {
        d->result += "\n";
        addIndent();
    }
    ++(d->indent);
    d->result += "<" + qName;
    for( int index = 0; index < atts.length(); ++index ) {
        d->result += " " + atts.qName( index ) + "=\"";
        addQuoted( atts.value( index ) );
        d->result += "\"";
    }
    d->result += ">";
    d->lastWasTag = true;
    return true;
}

/*!
    End an element called \a qName, which can be broken up into
    \a namespaceURI and \a localName.
*/
bool QWbXmlToXmlContentHandler::endElement( const QString& , const QString& , const QString& qName )
{
    --(d->indent);
    if ( d->lastWasTag ) {
        d->result += "\n";
        addIndent();
    }
    d->result += "</" + qName + ">";
    d->lastWasTag = true;
    return true;
}

/*!
    Process the character data in \a ch.
*/
bool QWbXmlToXmlContentHandler::characters( const QString& ch )
{
    addQuoted( ch );
    d->lastWasTag = false;
    return true;
}

/*!
    Process the ignorable whitespace in \a ch.
*/
bool QWbXmlToXmlContentHandler::ignorableWhitespace( const QString& )
{
    return true;
}

/*!
    Handle the processing instruction \a target, with content \a data.
*/
bool QWbXmlToXmlContentHandler::processingInstruction( const QString& , const QString& )
{
    return true;
}

/*!
    Process the skipped entity \a name.
*/
bool QWbXmlToXmlContentHandler::skippedEntity( const QString& )
{
    return true;
}

/*!
    Return a string that describes the last occurring error.
*/
QString QWbXmlToXmlContentHandler::errorString() const
{
    return "";
}

/*!
    Return the final XML string representation.
*/
QString QWbXmlToXmlContentHandler::toString() const
{
    return d->result;
}

void QWbXmlToXmlContentHandler::addQuoted( const QString& value )
{
    for( int posn = 0; posn < value.length(); ++posn ) {
        if ( value[posn] == '&' ) {
            d->result += "&amp;";
        } else if ( value[posn] == '<' ) {
            d->result += "&lt;";
        } else if ( value[posn] == '>' ) {
            d->result += "&gt;";
        } else if ( value[posn] == '"' ) {
            d->result += "&quot;";
        } else {
            d->result += value[posn];
        }
    }
}

void QWbXmlToXmlContentHandler::addIndent()
{
    int temp = d->indent;
    while ( temp > 0 ) {
        d->result += "  ";
        --temp;
    }
}

