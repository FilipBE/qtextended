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

#include <qatresultparser.h>
#include <qatresult.h>
#include <qatutils.h>

/*!
    \class QAtResultParser
    \inpublicgroup QtBaseModule

    \brief The QAtResultParser class provides support for parsing the response to AT modem commands and unsolicited notifications.
    \ingroup telephony::serial

    The following example demonstrates how to parse the response to a
    \c{AT+CPOL} (preferred operator list) query:

    \code
        QAtResultParser parser( result );
        while ( parser.next( "+CPOL:" ) ) {
            uint index = parser.readNumeric();
            uint format = parser.readNumeric();
            QString name;
            if ( format == 2 )  // Numeric format.
                name = QString::number( parser.readNumeric() );
            else                // String format.
                name = parser.readString();
        }
    \endcode

    \sa QAtResult
*/

class QAtResultParserPrivate
{
public:
    QAtResultParserPrivate( const QString& content )
    {
        response = content;
        posn = 0;
        linePosn = 0;
        notification = false;
    }

    QString response;
    QString line;
    int posn;
    int linePosn;
    bool notification;
};

/*!
    Construct an AT modem result parser to parse the content of \a result.
    The caller will typically follow this with a call to next() to position
    the parser on the first line of relevant result data.
*/
QAtResultParser::QAtResultParser( const QAtResult& result )
{
    d = new QAtResultParserPrivate( result.content() );
}

/*!
    Construct an AT modem result parser and initialize it to parse
    the specified unsolicited \a notification.  Notifications are
    expected to have the format \c{NAME: VALUE}.  The next() function
    will be called internally to position the parser at \c{VALUE}.
*/
QAtResultParser::QAtResultParser( const QString& notification )
{
    d = new QAtResultParserPrivate( notification );
    int posn = 0;
    while ( posn < notification.length() && notification[posn] != ':' ) {
        ++posn;
    }
    if ( posn < notification.length() )
        ++posn;     // Account for the colon.
    next( notification.left( posn ) );
    d->notification = true;
}

/*!
    Destruct this AT modem result parser.
*/
QAtResultParser::~QAtResultParser()
{
    delete d;
}

/*!
    Reset this AT modem result parser to the beginning of the content.
*/
void QAtResultParser::reset()
{
    if ( d->notification ) {
        d->linePosn = 0;
    } else {
        d->line = QString();
        d->posn = 0;
        d->linePosn = 0;
    }
}

/*!
    Position this AT modem result parser on the next line that begins
    with \a prefix. Returns true on success; otherwise returns false.

    \sa line(), lines(), readNumeric(), readString()
*/
bool QAtResultParser::next( const QString& prefix )
{
    while ( d->posn < d->response.length() ) {

        // Extract the next line.
        d->line = "";
        d->linePosn = 0;
        while ( d->posn < d->response.length() &&
                d->response[d->posn] != '\n' ) {
            d->line += d->response[(d->posn)++];
        }
        if ( d->posn < d->response.length() ) {
            ++(d->posn);
        }

        // Bail out if the line starts with the expected prefix.
        if ( d->line.startsWith( prefix ) ) {
            d->linePosn = prefix.length();
            while ( d->linePosn < d->line.length() &&
                    d->line[d->linePosn] == ' ' ) {
                ++(d->linePosn);
            }
            d->line = d->line.mid( d->linePosn );
            d->linePosn = 0;
            return true;
        }

    }
    return false;
}

/*!
    Returns the full content of the line that next() positioned us on.
    The line's prefix is not included in the return value.

    \sa next(), readNumeric(), readString()
*/
QString QAtResultParser::line()
{
    return d->line;
}

/*!
    Read a numeric value from the current line.

    \sa readString(), skip()
*/
uint QAtResultParser::readNumeric()
{
    uint value = 0;
    while ( d->linePosn < d->line.length() &&
            d->line[d->linePosn] >= '0' && d->line[d->linePosn] <= '9' ) {
        value = value * 10 + (uint)(d->line[d->linePosn].unicode() - '0');
        ++(d->linePosn);
    }
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] == ' ' ) {
        ++(d->linePosn);
    }
    return value;
}

static QString nextString( const QString& buf, int& posn )
{
    uint posn2 = (uint)posn;
    QString result = QAtUtils::nextString( buf, posn2 );
    posn = (int)posn2;
    return result;
}

/*!
    Read a string from the current line.

    \sa readNumeric(), skip()
*/
QString QAtResultParser::readString()
{
    QString value = nextString( d->line, d->linePosn );
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] == ' ' ) {
        ++(d->linePosn);
    }
    return value;
}

/*!
    Skip the contents of a comma-separated field in the current line.

    \sa readNumeric(), readString()
*/
void QAtResultParser::skip()
{
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == ',' ) {
        ++(d->linePosn);
    }
    while ( d->linePosn < d->line.length() && d->line[d->linePosn] != ',' ) {
        ++(d->linePosn);
    }
}

/*!
    Read the next line of input as literal text, without looking for a prefix.
    This is for results from commands such as \c{AT+CMGL} which place the
    PDU on a line of its own.
*/
QString QAtResultParser::readNextLine()
{
    QString line = "";

    while ( d->posn < d->response.length() &&
            d->response[d->posn] != '\n' ) {
        line += d->response[(d->posn)++];
    }
    if ( d->posn < d->response.length() ) {
        ++(d->posn);
    }

    return line;
}

/*!
    Returns the content of all lines that begin with \a prefix starting
    at the current position.

    \sa next()
*/
QStringList QAtResultParser::lines( const QString& prefix )
{
    QStringList result;
    while ( next( prefix ) ) {
        result << d->line;
    }
    return result;
}

/*!
    Read a list of values surrounded by parentheses.  This is for
    complex command results that cannot be parsed with readNumeric()
    and readString().
*/
QList<QAtResultParser::Node> QAtResultParser::readList()
{
    QList<QAtResultParser::Node> list;
    if ( d->linePosn < d->line.length() && d->line[d->linePosn] == '(' ) {
        ++(d->linePosn);
        while ( d->linePosn < d->line.length() &&
                d->line[d->linePosn] != ')' ) {
            uint ch = d->line[d->linePosn].unicode();
            if ( ch >= '0' && ch <= '9' ) {
                // Parse a number or range.
                uint number = readNumeric();
                if ( d->linePosn < d->line.length() &&
                     d->line[d->linePosn] == '-' ) {
                    ++(d->linePosn);
                    uint last = readNumeric();
                    list.append( QAtResultParser::Node( number, last ) );
                } else {
                    list.append( QAtResultParser::Node( number ) );
                }
            } else if ( ch == '"' ) {
                // Parse a string.
                list.append( QAtResultParser::Node( readString() ) );
            } else {
                // Encountered something unknown - bail out at this point.
                d->linePosn = d->line.length();
                return list;
            }
        }
        if ( d->linePosn < d->line.length() ) {
            // Skip the ')' at the end of the list.
            ++(d->linePosn);
            if ( d->linePosn < d->line.length() &&
                 d->line[d->linePosn] == ',' ) {
                // Skip a trailing comma.
                ++(d->linePosn);
            }
            while ( d->linePosn < d->line.length() &&
                 d->line[d->linePosn] == ' ' ) {
                // Skip trailing white space.
                ++(d->linePosn);
            }
        }
    }
    return list;
}

/*!
    \class QAtResultParser::Node
    \inpublicgroup QtBaseModule
    \ingroup telephony::serial

    \brief The Node class provides access to a generic value parsed from an AT command list.

    Instances of this class are returned from the QAtResultParser::readList() function.

    \sa QAtResultParser::readList()
*/

QAtResultParser::Node::Node( uint number )
{
    _kind = Number;
    _number = number;
    _list = 0;
}

QAtResultParser::Node::Node( uint first, uint last )
{
    _kind = Range;
    _number = first;
    _last = last;
    _list = 0;
}

QAtResultParser::Node::Node( const QString& str )
{
    _kind = String;
    _str = str;
    _list = 0;
}

QAtResultParser::Node::Node( QList<Node> *list )
{
    _kind = List;
    _list = list;
}

/*!
    Create a new result list node from \a other.
*/
QAtResultParser::Node::Node( const Node& other )
{
    _kind = other._kind;
    _number = other._number;
    _last = other._last;
    _str = other._str;
    _list = ( other._list ? new QList<Node>( *other._list ) : 0 );
}

/*!
    Destruct this node.
*/
QAtResultParser::Node::~Node()
{
    if ( _list )
        delete _list;
}

/*!
    \fn bool QAtResultParser::Node::isNumber() const

    Returns true of if this node contains a number; otherwise returns false.

    \sa asNumber()
*/

/*!
    \fn bool QAtResultParser::Node::isRange() const

    Returns true if this node contains a range; otherwise returns false.

    \sa asFirst(), asLast()
*/

/*!
    \fn bool QAtResultParser::Node::isString() const

    Returns true if this node contains a string; otherwise returns false.

    \sa asString()
*/

/*!
    \fn bool QAtResultParser::Node::isList() const

    Returns true if this node contains a list; otherwise returns false.

    \sa asList()
*/

/*!
    \fn uint QAtResultParser::Node::asNumber() const

    Returns the number contained within this node, or return zero if node not a number.

    \sa isNumber()
*/

/*!
    \fn uint QAtResultParser::Node::asFirst() const

    Returns the first number in a range that is contained within this node; otherwise returns zero if not a range.

    \sa isRange(), asLast()
*/

/*!
    \fn uint QAtResultParser::Node::asLast() const

    Returns the last number in a range that is contained within this node; otherwise returns zero if not a range.

    \sa isRange(), asFirst()
*/

/*!
    \fn QString QAtResultParser::Node::asString() const

    Returns the string contained within this node; otherwise returns null if not a string.

    \sa isString()
*/

/*!
    \fn QList<QAtResultParser::Node> QAtResultParser::Node::asList() const

    Returns the list contained within this node, or returns an empty list if this
    node does not contain a list.

    \sa isList()
*/
