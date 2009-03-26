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

#include <qatutils.h>
#include <qatresultparser.h>
#include <qgsmcodec.h>
#include <qtextcodec.h>

/*!
    \class QAtUtils
    \inpublicgroup QtBaseModule

    \brief The QAtUtils class provides several utility functions that assist with interfacing to AT-based modems.
    \ingroup telephony::serial
*/

static const char hexchars[] = "0123456789ABCDEF";
static bool octalEscapesFlag = false;

/*!
    Quote \a str so that it is suitable to be sent as an AT
    command argument.  The caller will need to add double
    quotes to the start and end of the return value to complete
    the AT command argument.
*/
QString QAtUtils::quote( const QString& str )
{
    // Bail out if the string does not need to be quoted.
    if ( str.indexOf( QChar('"') ) == -1 && str.indexOf( QChar('\\') ) == -1 &&
         str.indexOf( QChar('\r') ) == -1 && str.indexOf( QChar('\n') ) == -1 ) {
        return str;
    }

    // Build the quoted result.
    QString result = "";
    int posn = 0;
    uint ch;
    while ( posn < str.length() ) {
        ch = str[posn++].unicode();
        if ( ch == '"' || ch == '\\' || ch == '\r' || ch == '\n' ) {
            result += (QChar)'\\';
            result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
            result += (QChar)(hexchars[ch & 0x0F]);
        } else {
            result += (QChar)ch;
        }
    }
    return result;
}

/*!
    Convert the byte array, \a binary, into a hexadecimal string.

    \sa fromHex()
*/
QString QAtUtils::toHex( const QByteArray& binary )
{
    QString str = "";
    static char const hexchars[] = "0123456789ABCDEF";

    for ( int i = 0; i < binary.size(); i++ ) {
        str += (QChar)(hexchars[ (binary[i] >> 4) & 0x0F ]);
        str += (QChar)(hexchars[ binary[i] & 0x0F ]);
    }

    return str;
}

/*!
    Convert a hexadecimal string, \a hex, into a byte array.

    \sa toHex()
*/
QByteArray QAtUtils::fromHex( const QString& hex )
{
    QByteArray bytes;
    uint ch;
    int posn;
    int nibble, value, flag, size;

    flag = 0;
    value = 0;
    size = 0;
    for ( posn = 0; posn < hex.length(); ++posn ) {
        ch = (uint)( hex[posn].unicode() );
        if ( ch >= '0' && ch <= '9' ) {
            nibble = ch - '0';
        } else if ( ch >= 'A' && ch <= 'F' ) {
            nibble = ch - 'A' + 10;
        } else if ( ch >= 'a' && ch <= 'f' ) {
            nibble = ch - 'a' + 10;
        } else {
            continue;
        }
        value = (value << 4) | nibble;
        flag = !flag;
        if ( !flag ) {
            bytes.resize( size + 1 );
            bytes[size++] = (char)value;
            value = 0;
        }
    }

    return bytes;
}

/*!
    Decode a phone number from \a value and \a type.  The \a type is
    a type of address octet, usually 145 for international numbers
    and 129 for local numbers.  The return will normalize the
    value to include the \c{+} prefix for international numbers.

    \sa encodeNumber()
*/
QString QAtUtils::decodeNumber( const QString& value, uint type )
{
    if ( type == 145 && value.length() != 0 && value[0] != '+' )
        return "+" + value;
    else
        return value;
}

/*!
    Read a string field and a numeric field from \a parser and
    then decode them into a properly normalized phone number.

    \sa encodeNumber()
*/
QString QAtUtils::decodeNumber( QAtResultParser& parser )
{
    QString value = parser.readString();
    uint type = parser.readNumeric();
    return decodeNumber( value, type );
}

/*!
    Encode the phone number in \a value into a string plus a
    type of address octet.  International numbers that start
    with \c{+} become \c{"number",145}, and local numbers
    become \c{"number",129}.  If \a keepPlus is true,
    then the \c{+} will be left on the resulting number,
    even if the type is 145.

    \sa decodeNumber()
*/
QString QAtUtils::encodeNumber( const QString& value, bool keepPlus )
{
    if ( value.length() > 0 && value[0] == '+' ) {
        if ( keepPlus )
            return "\"" + quote( value ) + "\",145";
        else
            return "\"" + quote( value.mid(1) ) + "\",145";
    } else {
        return "\"" + quote( value ) + "\",129";
    }
}

static int FromHexDigit( uint ch )
{
    if ( ch >= '0' && ch <= '9' ) {
        return (int)( ch - '0' );
    } else if ( ch >= 'A' && ch <= 'F' ) {
        return (int)( ch - 'A' + 10 );
    } else if ( ch >= 'a' && ch <= 'f' ) {
        return (int)( ch - 'a' + 10 );
    } else {
        return -1;
    }
}

static int FromOctalDigit( uint ch )
{
    if ( ch >= '0' && ch <= '7' ) {
        return (int)( ch - '0' );
    } else {
        return -1;
    }
}

/*!
    Extract the next quoted string from \a buf, starting at
    \a posn.

    \sa parseNumber(), skipField()
*/
QString QAtUtils::nextString( const QString& buf, uint& posn )
{
    QString result = "";
    uint ch;
    int digit, digit2, digit3;
    while ( posn < (uint)(buf.length()) && buf[posn] != '"' ) {
        ++posn;
    }
    if ( posn >= (uint)(buf.length()) ) {
        return result;
    }
    ++posn;
    while ( posn < (uint)(buf.length()) && ( ch = buf[posn].unicode() ) != '"' ) {
        ++posn;
        if ( ch == '\\' ) {
            if ( !octalEscapesFlag ) {
                // Hex-quoted character.
                if ( posn >= (uint)buf.length() )
                    break;
                digit = FromHexDigit( buf[posn].unicode() );
                if ( digit == -1 ) {
                    result += (QChar)'\\';
                    continue;
                }
                if ( ( posn + 1 ) >= (uint)buf.length() ) {
                    ch = (uint)digit;
                    ++posn;
                } else {
                    digit2 = FromHexDigit( buf[posn + 1].unicode() );
                    if ( digit2 == -1 ) {
                        ch = (uint)digit;
                        ++posn;
                    } else {
                        ch = (uint)(digit * 16 + digit2);
                        posn += 2;
                    }
                }
            } else {
                // Octal-quoted character.
                if ( posn >= (uint)buf.length() )
                    break;
                digit = FromOctalDigit( buf[posn].unicode() );
                if ( digit == -1 ) {
                    result += (QChar)'\\';
                    continue;
                }
                if ( ( posn + 1 ) >= (uint)buf.length() ) {
                    ch = (uint)digit;
                    ++posn;
                } else {
                    digit2 = FromOctalDigit( buf[posn + 1].unicode() );
                    if ( digit2 == -1 ) {
                        ch = (uint)digit;
                        ++posn;
                    } else {
                        if ( ( posn + 2 ) >= (uint)buf.length() ) {
                            ch = (uint)(digit * 8 + digit2);
                            posn += 2;
                        } else {
                            digit3 = FromOctalDigit( buf[posn + 2].unicode() );
                            if ( digit3 == -1 ) {
                                ch = (uint)(digit * 8 + digit2);
                                posn += 2;
                            } else {
                                ch = (uint)(digit * 64 + digit2 * 8 + digit3);
                                posn += 3;
                            }
                        }
                    }
                }
            }
        }
        result += (QChar)ch;
    }
    if ( posn < (uint)buf.length() ) {
        ++posn;
    }
    return result;
}

/*!
    Utility function for parsing a number from position \a posn in \a str.

    \sa nextString(), skipField()
*/
uint QAtUtils::parseNumber( const QString& str, uint& posn )
{
    uint num = 0;
    while ( posn < (uint)str.length() && ( str[posn] == ' ' || str[posn] == ',' ) ) {
        ++posn;
    }
    while ( posn < (uint)str.length() && str[posn] >= '0' && str[posn] <= '9' ) {
        num = num * 10 + (uint)(str[posn].unicode() - '0');
        ++posn;
    }
    return num;
}

/*!
    Utility function for skipping a comma-delimited field starting
    at \a posn within \a str.

    \sa nextString(), parseNumber()
*/
void QAtUtils::skipField( const QString& str, uint& posn )
{
    if ( posn < (uint)str.length() && str[posn] == ',' ) {
        ++posn;
    }
    while ( posn < (uint)str.length() && str[posn] != ',' ) {
        ++posn;
    }
}

class QGsmHexCodec : public QTextCodec
{
public:
    QGsmHexCodec();
    ~QGsmHexCodec();

    QByteArray name() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

QGsmHexCodec::QGsmHexCodec()
{
}

QGsmHexCodec::~QGsmHexCodec()
{
}

QByteArray QGsmHexCodec::name() const
{
    return QByteArray( "gsm-hex" );
}

int QGsmHexCodec::mibEnum() const
{
    return 61239;
}

QString QGsmHexCodec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    int nibble = 0;
    int value = 0;
    int digit;
    bool secondByte = false;
    while ( length-- > 0 ) {
        char ch = *in++;
        if ( ch >= '0' && ch <= '9' )
            digit = ch - '0';
        else if ( ch >= 'A' && ch <= 'F' )
            digit = ch - 'A' + 10;
        else if ( ch >= 'a' && ch <= 'f' )
            digit = ch - 'a' + 10;
        else
            continue;
        if ( !nibble ) {
            value = digit * 16;
            nibble = 1;
        } else {
            value += digit;
            if ( !secondByte ) {
                if ( value != 0x1B ) {
                    str += QGsmCodec::twoByteToUnicode( (unsigned short)value );
                } else {
                    secondByte = true;
                }
            } else {
                value += 0x1B00;
                str += QGsmCodec::twoByteToUnicode( (unsigned short)value );
                secondByte = false;
            }
            nibble = 0;
        }
    }
    return str;
}

QByteArray QGsmHexCodec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray buf;
    while ( length-- > 0 ) {
        unsigned short ch = QGsmCodec::twoByteFromUnicode( *in++ );
        if ( ch >= 256 ) {
            buf += hexchars[ (ch >> 12) & 0x0F ];
            buf += hexchars[ (ch >> 8) & 0x0F ];
        }
        buf += hexchars[ (ch >> 4) & 0x0F ];
        buf += hexchars[ ch & 0x0F ];
    }
    return buf;
}

class QUcs2HexCodec : public QTextCodec
{
public:
    QUcs2HexCodec();
    ~QUcs2HexCodec();

    QByteArray name() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

QUcs2HexCodec::QUcs2HexCodec()
{
}

QUcs2HexCodec::~QUcs2HexCodec()
{
}

QByteArray QUcs2HexCodec::name() const
{
    return "ucs2-hex";
}

int QUcs2HexCodec::mibEnum() const
{
    return 61240;
}

QString QUcs2HexCodec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    int nibble = 0;
    int value = 0;
    int digit;
    while ( length-- > 0 ) {
        char ch = *in++;
        if ( ch >= '0' && ch <= '9' )
            digit = ch - '0';
        else if ( ch >= 'A' && ch <= 'F' )
            digit = ch - 'A' + 10;
        else if ( ch >= 'a' && ch <= 'f' )
            digit = ch - 'a' + 10;
        else
            continue;
        value = value * 16 + digit;
        ++nibble;
        if ( nibble >= 4 ) {
            str += QChar( (ushort)value );
            nibble = 0;
            value = 0;
        }
    }
    return str;
}

QByteArray QUcs2HexCodec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray buf;
    while ( length-- > 0 ) {
        uint ch = in->unicode();
        ++in;
        buf += hexchars[ (ch >> 12) & 0x0F ];
        buf += hexchars[ (ch >> 8) & 0x0F ];
        buf += hexchars[ (ch >> 4) & 0x0F ];
        buf += hexchars[ ch & 0x0F ];
    }
    return buf;
}

class QCodePage437Codec : public QTextCodec
{
public:
    QCodePage437Codec();
    ~QCodePage437Codec();

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

// Convert IBM437 character codes 0x00 - 0xFF into Unicode.
static ushort const cp437ToUnicode[256] =
   {0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001c, 0x001b, 0x007f, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x001a,
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
    0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
    0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
    0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
    0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x03bc, 0x03c4,
    0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
    0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
    0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0};



// Convert Unicode 0x0000 - 0x00FF into IBM437.
static unsigned char const cp437FromUnicode[256] =
   {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x7f, 0x1b, 0x1a, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x1c,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , '?' , '?' , '?' , '?' , '?' , '?' , '?' ,
    0xff, 0xad, 0x9b, 0x9c, '?' , 0x9d, '?' , 0x15,
    '?' , '?' , 0xa6, 0xae, 0xaa, '?' , '?' , '?' ,
    0xf8, 0xf1, 0xfd, '?' , '?' , '?' , 0x14, 0xfa,
    '?' , '?' , 0xa7, 0xaf, 0xac, 0xab, '?' , 0xa8,
    '?' , '?' , '?' , '?' , 0x8e, 0x8f, 0x92, 0x80,
    '?' , 0x90, '?' , '?' , '?' , '?' , '?' , '?' ,
    '?' , 0xa5, '?' , '?' , '?' , '?' , 0x99, '?' ,
    '?' , '?' , '?' , '?' , 0x9a, '?' , '?' , 0xe1,
    0x85, 0xa0, 0x83, '?' , 0x84, 0x86, 0x91, 0x87,
    0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
    '?' , 0xa4, 0x95, 0xa2, 0x93, '?' , 0x94, 0xf6,
    '?' , 0x97, 0xa3, 0x96, 0x81, '?' , '?' , 0x98};

QCodePage437Codec::QCodePage437Codec()
{
}

QCodePage437Codec::~QCodePage437Codec()
{
}

QByteArray QCodePage437Codec::name() const
{
    return "IBM437";
}

QList<QByteArray> QCodePage437Codec::aliases() const
{
    QList<QByteArray> list;
    list << "CP437";
    return list;
}

int QCodePage437Codec::mibEnum() const
{
    return 437;
}

QString QCodePage437Codec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    if ( length >= 6 && in[0] == '8' && in[1] == '0' &&
         in[length - 4] == 'F' && in[length - 3] == 'F' &&
         in[length - 2] == 'F' && in[length - 1] == 'F') {

        // UCS-2 string embedded within a 437-encoded string.
        int nibble = 0;
        int value = 0;
        int digit;
        in += 2;
        length -= 6;
        while ( length-- > 0 ) {
            char ch = *in++;
            if ( ch >= '0' && ch <= '9' )
                digit = ch - '0';
            else if ( ch >= 'A' && ch <= 'F' )
                digit = ch - 'A' + 10;
            else if ( ch >= 'a' && ch <= 'f' )
                digit = ch - 'a' + 10;
            else
                continue;
            value = value * 16 + digit;
            ++nibble;
            if ( nibble >= 4 ) {
                str += QChar( (ushort)value );
                nibble = 0;
                value = 0;
            }
        }

    } else {

        // Regular 437-encoded string.
        while ( length-- > 0 )
            str += QChar((unsigned int)cp437ToUnicode[*in++ & 0xFF]);

    }
    return str;
}

QByteArray QCodePage437Codec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray result;
    unsigned int ch;
    char *out;

    // Determine if the string should be encoded using the UCS-2 hack.
    bool non437 = false;
    for ( int posn = 0; !non437 && posn < length; ++posn ) {
        ch = in[posn].unicode();
        if ( ch >= 0x0100 )
            non437 = true;
        else if ( cp437FromUnicode[ch] == '?' && ch != '?' )
            non437 = true;
    }
    if ( non437 ) {
        // There is a non CP437 character in this string, so use UCS-2.
        result.resize( length * 4 + 6 );
        out = result.data();
        *out++ = '8';
        *out++ = '0';
        while ( length-- > 0 ) {
            uint ch = in->unicode();
            ++in;
            *out++ = hexchars[ (ch >> 12) & 0x0F ];
            *out++ = hexchars[ (ch >> 8) & 0x0F ];
            *out++ = hexchars[ (ch >> 4) & 0x0F ];
            *out++ = hexchars[ ch & 0x0F ];
        }
        *out++ = 'F';
        *out++ = 'F';
        *out++ = 'F';
        *out   = 'F';
        return result;
    }

    // If we get here, we can guarantee that the string only contains
    // valid CP437 code points between 0x0000 and 0x00FF.
    result.resize( length );
    out = result.data();
    while ( length-- > 0 ) {
        *out++ = (char)cp437FromUnicode[in->unicode()];
        ++in;
    }
    return result;
}

class QCodePage850Codec : public QTextCodec
{
public:
    QCodePage850Codec();
    ~QCodePage850Codec();

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};

// Convert IBM850 character codes 0x00 - 0xFF into Unicode.
static ushort const cp850ToUnicode[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
    0x0018, 0x0019, 0x001c, 0x001b, 0x007f, 0x001d, 0x001e, 0x001f, 
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x001a, 
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7, 
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5, 
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9, 
    0x00ff, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x0192, 
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba, 
    0x00bf, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb, 
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x00c0, 
    0x00a9, 0x2563, 0x2551, 0x2557, 0x255d, 0x00a2, 0x00a5, 0x2510, 
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x00e3, 0x00c3, 
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4, 
    0x00f0, 0x00d0, 0x00ca, 0x00cb, 0x00c8, 0x0131, 0x00cd, 0x00ce, 
    0x00cf, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0x00cc, 0x2580, 
    0x00d3, 0x00df, 0x00d4, 0x00d2, 0x00f5, 0x00d5, 0x00b5, 0x00fe, 
    0x00de, 0x00da, 0x00db, 0x00d9, 0x00fd, 0x00dd, 0x00af, 0x00b4, 
    0x00ad, 0x00b1, 0x2017, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8, 
    0x00b0, 0x00a8, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0,
};

// Convert Unicode 0x0000 - 0x00FF into IBM850.
static unsigned char const cp850FromUnicode[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
    0x18, 0x19, 0x7f, 0x1b, 0x1a, 0x1d, 0x1e, 0x1f, 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x1c, 
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 
    0xff, 0xad, 0xbd, 0x9c, 0xcf, 0xbe, 0xdd, 0xf5, 
    0xf9, 0xb8, 0xa6, 0xae, 0xaa, 0xf0, 0xa9, 0xee, 
    0xf8, 0xf1, 0xfd, 0xfc, 0xef, 0xe6, 0xf4, 0xfa, 
    0xf7, 0xfb, 0xa7, 0xaf, 0xac, 0xab, 0xf3, 0xa8, 
    0xb7, 0xb5, 0xb6, 0xc7, 0x8e, 0x8f, 0x92, 0x80, 
    0xd4, 0x90, 0xd2, 0xd3, 0xde, 0xd6, 0xd7, 0xd8, 
    0xd1, 0xa5, 0xe3, 0xe0, 0xe2, 0xe5, 0x99, 0x9e, 
    0x9d, 0xeb, 0xe9, 0xea, 0x9a, 0xed, 0xe8, 0xe1, 
    0x85, 0xa0, 0x83, 0xc6, 0x84, 0x86, 0x91, 0x87, 
    0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b, 
    0xd0, 0xa4, 0x95, 0xa2, 0x93, 0xe4, 0x94, 0xf6, 
    0x9b, 0x97, 0xa3, 0x96, 0x81, 0xec, 0xe7, 0x98
};

// Compact mapping table for converting Unicode 0x0100 - 0xFFFF into IBM850.
static ushort const cp850MappingInput[] = {
    0x0110, 0x0131, 0x0192, 0x2017, 0x2022, 0x203c, 0x203e, 0x2190, 
    0x2191, 0x2192, 0x2193, 0x2194, 0x2195, 0x21a8, 0x221f, 0x2302, 
    0x2500, 0x2502, 0x250c, 0x2510, 0x2514, 0x2518, 0x251c, 0x2524, 
    0x252c, 0x2534, 0x253c, 0x2550, 0x2551, 0x2554, 0x2557, 0x255a, 
    0x255d, 0x2560, 0x2563, 0x2566, 0x2569, 0x256c, 0x2580, 0x2584, 
    0x2588, 0x2591, 0x2592, 0x2593, 0x25a0, 0x25ac, 0x25b2, 0x25ba, 
    0x25bc, 0x25c4, 0x25cb, 0x25d8, 0x25d9, 0x263a, 0x263b, 0x263c, 
    0x2640, 0x2642, 0x2660, 0x2663, 0x2665, 0x2666, 0x266a, 0x266c, 
    0xff01, 0xff02, 0xff03, 0xff04, 0xff05, 0xff06, 0xff07, 0xff08, 
    0xff09, 0xff0a, 0xff0b, 0xff0c, 0xff0d, 0xff0e, 0xff0f, 0xff10, 
    0xff11, 0xff12, 0xff13, 0xff14, 0xff15, 0xff16, 0xff17, 0xff18, 
    0xff19, 0xff1a, 0xff1b, 0xff1c, 0xff1d, 0xff1e, 0xff1f, 0xff20, 
    0xff21, 0xff22, 0xff23, 0xff24, 0xff25, 0xff26, 0xff27, 0xff28, 
    0xff29, 0xff2a, 0xff2b, 0xff2c, 0xff2d, 0xff2e, 0xff2f, 0xff30, 
    0xff31, 0xff32, 0xff33, 0xff34, 0xff35, 0xff36, 0xff37, 0xff38, 
    0xff39, 0xff3a, 0xff3b, 0xff3c, 0xff3d, 0xff3e, 0xff3f, 0xff40, 
    0xff41, 0xff42, 0xff43, 0xff44, 0xff45, 0xff46, 0xff47, 0xff48, 
    0xff49, 0xff4a, 0xff4b, 0xff4c, 0xff4d, 0xff4e, 0xff4f, 0xff50, 
    0xff51, 0xff52, 0xff53, 0xff54, 0xff55, 0xff56, 0xff57, 0xff58, 
    0xff59, 0xff5a, 0xff5b, 0xff5c, 0xff5d, 0xff5e, 0xffe8, 0xffe9, 
    0xffea, 0xffeb, 0xffec, 0xffed, 0xffee
};
static unsigned char const cp850MappingOutput[] = {
    0xd1, 0xd5, 0x9f, 0xf2, 0x07, 0x13, 0xee, 0x1b, 
    0x18, 0x1a, 0x19, 0x1d, 0x12, 0x17, 0x1c, 0x7f, 
    0xc4, 0xb3, 0xda, 0xbf, 0xc0, 0xd9, 0xc3, 0xb4, 
    0xc2, 0xc1, 0xc5, 0xcd, 0xba, 0xc9, 0xbb, 0xc8, 
    0xbc, 0xcc, 0xb9, 0xcb, 0xca, 0xce, 0xdf, 0xdc, 
    0xdb, 0xb0, 0xb1, 0xb2, 0xfe, 0x16, 0x1e, 0x10, 
    0x1f, 0x11, 0x09, 0x08, 0x0a, 0x01, 0x02, 0x0f, 
    0x0c, 0x0b, 0x06, 0x05, 0x03, 0x04, 0x0d, 0x0e, 
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 
    0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 
    0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0xb3, 0x1b, 
    0x18, 0x1a, 0x19, 0xfe, 0x09
};
#define cp850MappingSize ((int)sizeof(cp850MappingInput))

QCodePage850Codec::QCodePage850Codec()
{
}

QCodePage850Codec::~QCodePage850Codec()
{
}

QByteArray QCodePage850Codec::name() const
{
    return "IBM850";
}

QList<QByteArray> QCodePage850Codec::aliases() const
{
    QList<QByteArray> list;
    list << "CP850";
    return list;
}

int QCodePage850Codec::mibEnum() const
{
    return 2009;
}

QString QCodePage850Codec::convertToUnicode(const char *in, int length, ConverterState *) const
{
    QString str;
    while ( length-- > 0 )
        str += QChar((unsigned int)cp850ToUnicode[*in++ & 0xFF]);
    return str;
}

QByteArray QCodePage850Codec::convertFromUnicode(const QChar *in, int length, ConverterState *) const
{
    QByteArray result;
    unsigned int ch;
    char *out;
    result.resize( length );
    out = result.data();
    while ( length-- > 0 ) {
        ch = in->unicode();
        ++in;
        if ( ch < 0x0100 ) {
            *out++ = (char)cp850FromUnicode[ch];
        } else {
            // Perform a binary search on the sparse mapping table.
            int left = 0;
            int right = cp850MappingSize - 1;
            int middle;
            while ( left <= right ) {
                middle = (left + right) / 2;
                if ( ch < cp850MappingInput[middle] ) {
                    right = middle - 1;
                } else if ( ch > cp850MappingInput[middle] ) {
                    left = middle + 1;
                } else {
                    *out++ = (char)cp850MappingOutput[middle];
                    break;
                }
            }
            if ( left > right ) {
                *out++ = '?';
            }
        }
    }
    return result;
}

/*!
    Returns the text codec for the GSM character set identifier \a gsmCharset.
    The returned object should not be deleted.

    The following standard GSM character sets from 3GPP TS 27.007 are recognized:

    \table
        \row \o \c GSM \o 7-bit default GSM alphabet.  There may be some loss of information
                          where multiple Unicode characters map to the same 7-bit encoding.
        \row \o \c HEX \o Hex encoding of the 7-bit default GSM alphabet.
        \row \o \c UCS2 \o Hex encoding of UCS-2.
        \row \o \c IRA \o International reference alphabet (i.e. ASCII).
        \row \o \c PCCPxxx \o PC character set code page \c xxx.  Code pages 437 and 850
                        are guaranteed to be supported.  Other code pages depend upon
                        the codecs installed with QTextCodec.
        \row \o \c PCDN \o PC Danish/Norwegian character set (same as \c PCCP850).
        \row \o \c UTF-8 \o 8-bit encoding of Unicode.
        \row \o \c 8859-n \o ISO 8859 Latin-n character set.
        \row \o \c 8859-C \o ISO 8859 Latin/Cyrillic character set (same as \c 8859-5).
        \row \o \c 8859-A \o ISO 8859 Latin/Arabic character set (same as \c 8859-6).
        \row \o \c 8859-G \o ISO 8859 Latin/Greek character set (same as \c 8859-7).
        \row \o \c 8859-H \o ISO 8859 Latin/Hebrew character set (same as \c 8859-8).
    \endtable

    This implementation also supports the following character sets, beyond those
    mandated by 3GPP TS 27.007:

    \table
        \row \o \c gsm-noloss \o 7-bit default GSM alphabet, with no loss of information.
        \row \o \c xxx \o Any codec \c xxx that is supported by QTextCodec::codecForName().
    \endtable

    \sa QTextCodec, QGsmCodec
*/
QTextCodec *QAtUtils::codec( const QString& gsmCharset )
{
    QString cs = gsmCharset.toLower();
    QTextCodec *codec = 0;

    // Convert the name into an appropriate codec.
    if ( cs == "gsm" ) {
        // 7-bit GSM character set.
        static QTextCodec *gsm = 0;
        if ( !gsm )
            gsm = new QGsmCodec();
        codec = gsm;
    } else if ( cs == "gsm-noloss" ) {
        // 7-bit GSM character set, with no loss of quality.
        static QTextCodec *gsmNoLoss = 0;
        if ( !gsmNoLoss )
            gsmNoLoss = new QGsmCodec( true );
        codec = gsmNoLoss;
    } else if ( cs == "hex" ) {
        // Direct hex character set.  The underlying character set could
        // be anything according to the specification, but we need to pick
        // something.  We assume that it is 7-bit GSM, as that is the most
        // likely value.
        static QTextCodec *hex = 0;
        if ( !hex )
            hex = new QGsmHexCodec();
        codec = hex;
    } else if ( cs == "ucs2" ) {
        // Hex-encoded UCS2 character set.
        static QTextCodec *ucs2 = 0;
        if ( !ucs2 )
            ucs2 = new QUcs2HexCodec();
        codec = ucs2;
    } else if ( cs == "ira" ) {
        // International Reference Alphabet (i.e. ASCII).  Use Latin-1 codec.
        codec = QTextCodec::codecForName( "ISO-8859-1" );
    } else if ( cs == "pccp437" ) {
        // PC DOS code page 437, which isn't standard in Qt,
        // but which is the default charset for Wavecom modems.
        //
        // Wavecom also has a horrible hack in its PCCP437 implementation
        // to handle embedded UCS-2 character strings.  A hex UCS-2 string
        // will start with "80" and end with "FFFF".  If the string does
        // not have this format, it is interpreted as code page 437.
        static QTextCodec *cp437 = 0;
        if ( !cp437 )
            cp437 = new QCodePage437Codec();
        codec = cp437;
    } else if ( cs == "pcdn" || cs == "pccp850" ) {
        // PC Danish/Norwegian character set.  Map to PC code page 850.
        codec = QTextCodec::codecForName( "CP850" );
        if ( !codec ) {
            // Qt does not have CP850 (i.e. QT_NO_CODECS is defined),
            // so we need to provide our own implementation.  The next
            // time we are called, codecForName( "CP850" ) will return
            // this object, so we won't need to allocate again.
            codec = new QCodePage850Codec();
        }
    } else if ( cs.startsWith( "pccp" ) ) {
        // Some other PC DOS code page.
        codec = QTextCodec::codecForName( "CP" + cs.mid(4).toLatin1() );
    } else if ( cs == "8859-c" ) {
        // ISO 8859 Latin/Cyrillic - same as ISO-8859-5.
        codec = QTextCodec::codecForName( "ISO-8859-5" );
    } else if ( cs == "8859-a" ) {
        // ISO 8859 Latin/Arabic - same as ISO-8859-6.
        codec = QTextCodec::codecForName( "ISO-8859-6" );
    } else if ( cs == "8859-g" ) {
        // ISO 8859 Latin/Greek - same as ISO-8859-7.
        codec = QTextCodec::codecForName( "ISO-8859-7" );
    } else if ( cs == "8859-h" ) {
        // ISO 8859 Latin/Hebrew - same as ISO-8859-8.
        codec = QTextCodec::codecForName( "ISO-8859-8" );
    } else if ( cs.startsWith( "8859-" ) ) {
        // ISO-8859-n character set.
        codec = QTextCodec::codecForName( "ISO-" + cs.toLatin1() );
    } else {
        // Not one of the standard GSM charset specifiers.  Look it up as-is.
        codec = QTextCodec::codecForName( gsmCharset.toLatin1() );
    }

    // Return the codec that we found, or bail out with Latin1 if unknown.
    if ( codec )
        return codec;
    else
        return QTextCodec::codecForName( "ISO-8859-1" );
}

/*!
    Quote \a str so that it is suitable to be sent as an AT
    command argument and use \a codec to encode the string.
    The caller will need to add double quotes to the start and
    end of the return value to complete the AT command argument.
*/
QString QAtUtils::quote( const QString& str, QTextCodec *codec )
{
    // Convert the string into raw bytes.
    QByteArray bytes;
    if ( codec )
        bytes = codec->fromUnicode( str );
    else
        bytes = str.toLatin1();

    // Determine if we need to apply quoting to the string.
    if ( bytes.indexOf( '"' ) == -1 && bytes.indexOf( '\\' ) == -1 &&
         bytes.indexOf( '\r' ) == -1 && bytes.indexOf( '\n' ) == -1 ) {
        return QString::fromLatin1( bytes.data(), bytes.length() );
    }

    // Create a new string for the quoted form.  The result is suitable
    // for converting to bytes again with toLatin1 in QAtChat::writeLine().
    QString result = "";
    int posn = 0;
    int ch;
    while ( posn < bytes.length() ) {
        ch = bytes[posn++] & 0xFF;
        if ( ch == '"' || ch == '\\' || ch == '\r' || ch == '\n' ) {
            result += (QChar)'\\';
            result += (QChar)(hexchars[(ch >> 4) & 0x0F]);
            result += (QChar)(hexchars[ch & 0x0F]);
        } else {
            result += (QChar)ch;
        }
    }
    return result;
}

/*!
    Decode \a str according to \a codec.  The string is assumed to have
    been retrieved from an AT modem using the facilities in QAtChat
    and QAtResultParser.
*/
QString QAtUtils::decode( const QString& str, QTextCodec *codec )
{
    if ( codec )
        return codec->toUnicode( str.toLatin1() );
    else
        return str;
}

/*!
    Strip non-digit characters from \a number and normalize special
    characters.  The digit and special characters are normalized as follows:

    \table
        \header \o Input \o Normalized \o Meaning
        \row \o \c 0..9 \o \c 0..9 \o Numeric dialing digits
        \row \o \c ABCD \o \c ABCD \o Alphabetic dialing digits
        \row \o \c abcd \o \c ABCD \o Alphabetic dialing digits
        \row \o \c ,pPxX \o \c , \o Short pause or extension separator
        \row \o \c wW \o \c W \o Wait for dial tone
        \row \o \c ! \o \c ! \o Hook flash
        \row \o \c @ \o \c @ \o Wait for silence
    \endtable

    All other characters are stripped from \a number.

    \sa QPhoneNumber::resolveLetters()
*/
QString QAtUtils::stripNumber( const QString& number )
{
    QString n = "";
    int posn;
    uint ch;
    for ( posn = 0; posn < number.length(); ++posn ) {
        ch = number[posn].unicode();
        if ( ch >= '0' && ch <= '9' ) {
            n += (QChar)ch;
        } else if ( ch == '+' || ch == '#' || ch == '*' ) {
            n += (QChar)ch;
        } else if ( ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' ) {
            // ABCD can actually be digits!
            n += (QChar)ch;
        } else if ( ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' ) {
            n += (QChar)( ch - 'a' + 'A' );
        } else if ( ch == ',' || ch == 'p' || ch == 'P' || ch == 'X' || ch == 'x' ) {
            // Comma and 'p' mean a short pause.
            // 'x' means an extension, which for now is the same as a pause.
            n += (QChar)',';
        } else if ( ch == 'w' || ch == 'W' ) {
            // 'w' means wait for dial tone.
            n += (QChar)'W';
        } else if ( ch == '!' || ch == '@' ) {
            // '!' = hook flash, '@' = wait for silence.
            n += (QChar)ch;
        }
    }
    return n;
}

/*!
    Returns true if nextString() should parse backslash escape
    sequences in octal rather than the default of hexadecimal; otherwise returns false.

    \sa setOctalEscapes()
*/
bool QAtUtils::octalEscapes()
{
    return octalEscapesFlag;
}

/*!
    Indicate that nextString() should parse backslash escape
    sequences in octal if \a value is true, rather than the
    default of hexadecimal (\a value is false).

    \sa octalEscapes()
*/
void QAtUtils::setOctalEscapes( bool value )
{
    octalEscapesFlag = value;
}

enum QSMSDataCodingScheme {
    QSMS_Compressed      = 0x0020,
    QSMS_MessageClass    = 0x0010,
    QSMS_DefaultAlphabet = 0x0000,
    QSMS_8BitAlphabet    = 0x0004,
    QSMS_UCS2Alphabet    = 0x0008,
    QSMS_ReservedAlphabet= 0x000C
};

/*!
    Decodes \a value according to the cell broadcast data coding scheme \a dcs.
    The \a value is assumed to have been parsed by nextString() and be
    compliant with section 5 of 3GPP TS 23.038.

    \sa nextString()
    \since 4.3.3
*/
QString QAtUtils::decodeString( const QString& value, uint dcs )
{
    // Extract just the alphabet bits.
    QSMSDataCodingScheme scheme;
    if ((dcs & 0xC0) == 0)
        scheme = QSMS_DefaultAlphabet;  // Other bits indicate 7-bit GSM language.
    else
        scheme = (QSMSDataCodingScheme)(dcs & 0x0C);
    if ( scheme == QSMS_UCS2Alphabet ) {
        // The string is hex-encoded UCS-2.
        return codec("ucs2")->toUnicode( value.toLatin1() );
    } else if ( scheme == QSMS_8BitAlphabet ) {
        // The string is 8-bit encoded in the current locale.
        return QTextCodec::codecForLocale()->toUnicode( value.toLatin1() );
    } else {
        // Assume that everything else is in the default GSM alphabet.
        return codec("gsm")->toUnicode( value.toLatin1() );
    }
}
