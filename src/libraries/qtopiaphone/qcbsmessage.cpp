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

#include <qcbsmessage.h>
#include <qatutils.h>
#ifdef PHONESIM
#include "qsmsmessage_p.h"
#else
#include "qsmsmessage_p.h"
#endif
#ifdef Q_WS_QWS
#include <qtopialog.h>
#else
#include <qdebug.h>
#define qLog(dbgcat) if(1); else qDebug()
#endif
#include <QTextCodec>

class QCBSMessagePrivate
{
public:
    QCBSMessagePrivate()
    {
        mMessageCode = 0;
        mScope = QCBSMessage::CellWide;
        mUpdateNumber = 0;
        mChannel = 0;
        mLanguage = QCBSMessage::English;
        mPage = 1;
        mNumPages = 1;
    }

    ~QCBSMessagePrivate()
    {
    }

    template <typename Stream> void readFromStream( Stream &s )
    {
        int val;
        s >> mMessageCode;
        s >> val;
        mScope = (QCBSMessage::GeographicalScope)val;
        s >> mUpdateNumber;
        s >> mChannel;
        s >> val;
        mLanguage = (QCBSMessage::Language)val;
        s >> mPage;
        s >> mNumPages;
        s >> mText;
    }

    template <typename Stream> void writeToStream( Stream &s )
    {
        s << mMessageCode;
        s << (int)mScope;
        s << mUpdateNumber;
        s << mChannel;
        s << (int)mLanguage;
        s << mPage;
        s << mNumPages;
        s << mText;
    }

    uint mMessageCode;
    QCBSMessage::GeographicalScope mScope;
    uint mUpdateNumber;
    uint mChannel;
    QCBSMessage::Language mLanguage;
    uint mPage;
    uint mNumPages;
    QString mText;
};


/*!
    \class QCBSMessage
    \inpublicgroup QtTelephonyModule

    \brief The QCBSMessage class specifies the contents of a cell broadcast message.

    \ingroup telephony

    Cell broadcast messages arrive asynchronously from the network
    and are delivered to applications via the QCellBroadcast::broadcast()
    signal.  The most common use of cell broadcast messages is to
    display cell location information.

    Cell broadcast messages are uniquely identified by a combination of
    scope(), messageCode(), and updateNumber().  Together with the cell identifier,
    QNetworkRegistration::cellId(), and the location area code,
    QNetworkRegistration::locationAreaCode(), client applications can determine
    if a message is new, or a repetition of an existing message.

    \sa QCellBroadcast, QNetworkRegistration
*/

/*!
    \enum QCBSMessage::GeographicalScope smsmessage.h
    Describes the geographical scope of a cell broadcast message, which is used to
    help determine if a cell broadcast message is new.

    \value CellWide The message is specific to the current cell.  If a message
                    arrives with the same messageCode() and updateNumber() as an
                    existing message, but QNetworkRegistration::cellId() has changed,
                    then the message is new rather than a repetition.  Messages with
                    the \c CellWide scope should be displayed immediately if possible.
    \value PLMNWide The message is specific to the network operator, regardless of cell.
    \value LocationAreaWide The message is specific to the location area.  If a message
                    arrives with the same messageCode() and updateNumber() as an
                    existing message, but the QNetworkRegistration::locationAreaCode()
                    has changed, then the message is new rather than a repetition.
    \value CellWide2 Same as \c CellWide, but the message does not need to be
                    displayed immediately.
*/

/*!
    \enum QCBSMessage::Language smsmessage.h
    Describes the language that a cell broadcast message is written in.

    \value German The message is written in German.
    \value English The message is written in English.
    \value Italian The message is written in Italian.
    \value French The message is written in French.
    \value Spanish The message is written in Spanish.
    \value Dutch The message is written in Dutch.
    \value Swedish The message is written in Swedish.
    \value Danish The message is written in Danish.
    \value Portuguese The message is written in Portuguese.
    \value Finnish The message is written in Finnish.
    \value Norwegian The message is written in Norwegian.
    \value Greek The message is written in Greek.
    \value Turkish The message is written in Turkish.
    \value Hungarian The message is written in Hungarian.
    \value Polish The message is written in Polish.
    \value Unspecified The language that the message is written in is unspecified.
*/

/*!
    Constructs an empty QCBSMessage.
*/
QCBSMessage::QCBSMessage()
{
    d = new QCBSMessagePrivate();
}

/*!
    Constructs a copy of \a msg.
*/
QCBSMessage::QCBSMessage(const QCBSMessage &msg)
{
    d = 0;
    *this = msg;
}

/*!
    Destructs the QCBSMessage.
*/
QCBSMessage::~QCBSMessage()
{
    delete d;
}

/*!
    Assigns a copy of \a msg to this object.
*/
QCBSMessage& QCBSMessage::operator=(const QCBSMessage & msg)
{
    if ( this == &msg )
        return *this;

    if ( d )
        delete d;
    d = new QCBSMessagePrivate();
    *d = *msg.d;

    return *this;
}

/*!
    Returns the cell broadcast message code.

    \sa setMessageCode()
*/
uint QCBSMessage::messageCode() const
{
    return d->mMessageCode;
}


/*!
    Sets the cell broadcast message code to \a num.

    \sa messageCode()
*/
void QCBSMessage::setMessageCode( uint num )
{
    d->mMessageCode = num;
}

/*!
    Returns the geographical scope of the message.

    \sa setScope()
*/
QCBSMessage::GeographicalScope QCBSMessage::scope() const
{
    return d->mScope;
}

/*!
    Sets the geographical scope of the message to \a scope.

    \sa scope()
*/
void QCBSMessage::setScope( QCBSMessage::GeographicalScope scope )
{
    d->mScope = scope;
}

/*!
    Returns the update number for this cell broadcast message.

    \sa setUpdateNumber()
*/
uint QCBSMessage::updateNumber() const
{
    return d->mUpdateNumber;
}

/*!
    Sets the update number for this cell broadcast message to \a num.

    \sa updateNumber()
*/
void QCBSMessage::setUpdateNumber( uint num )
{
    d->mUpdateNumber = num;
}

/*!
    Returns the channel number for this cell broadcast message.
    The most common channel number is 50, indicating cell
    location information.

    \sa setChannel()
*/
uint QCBSMessage::channel() const
{
    return d->mChannel;
}

/*!
    Sets the channel number for this cell broadcast message to \a chan.

    \sa channel()
*/
void QCBSMessage::setChannel( uint chan )
{
    d->mChannel = chan;
}

/*!
    Returns the language that this cell broadcast message is expressed in.
    This can allow applications to distinguish between multiple copies
    of the same information in different languages.

    \sa setLanguage()
*/
QCBSMessage::Language QCBSMessage::language() const
{
    return d->mLanguage;
}

/*!
    Sets the language that this cell broadcast message is expressed
    in to \a lang.

    \sa language()
*/
void QCBSMessage::setLanguage( QCBSMessage::Language lang )
{
    d->mLanguage = lang;
}

/*!
    Returns the page number for this cell broadcast message if the
    information that it contains is split over multiple pages.

    \sa setPage()
*/
uint QCBSMessage::page() const
{
    return d->mPage;
}

/*!
    Sets the page number for this cell broadcast message to \a page.

    \sa page()
*/
void QCBSMessage::setPage( uint page )
{
    d->mPage = page;
}

/*!
    Returns the number of pages that make up this cell broadcast message.

    \sa setNumPages()
*/
uint QCBSMessage::numPages() const
{
    return d->mNumPages;
}

/*!
    Sets the number of pages in this cell broadcast message to \a npages.

    \sa numPages()
*/
void QCBSMessage::setNumPages( uint npages )
{
    d->mNumPages = npages;
}

/*!
    Sets the text that is contained in this cell broadcast message to \a str.

    \sa text()
*/
void QCBSMessage::setText(const QString & str)
{
    d->mText = str;
}

/*!
    Returns the text that is contained in this cell broadcast message.

    \sa setText()
*/
QString QCBSMessage::text() const
{
    return d->mText;
}

/*!
    \internal
    \fn void QCBSMessage::deserialize(Stream &stream)
*/
template <typename Stream> void QCBSMessage::deserialize(Stream &stream)
{
    d->readFromStream( stream );
}

/*!
    \internal
    \fn void QCBSMessage::serialize(Stream &stream) const
*/
template <typename Stream> void QCBSMessage::serialize(Stream &stream) const
{
    d->writeToStream( stream );
}

/*!
    Returns true if this cell broadcast message object is equal to \a other; otherwise returns false.
*/
bool QCBSMessage::operator==( const QCBSMessage& other ) const
{
    return ( d->mMessageCode == other.d->mMessageCode &&
             d->mScope == other.d->mScope &&
             d->mUpdateNumber == other.d->mUpdateNumber &&
             d->mChannel == other.d->mChannel &&
             d->mLanguage == other.d->mLanguage &&
             d->mPage == other.d->mPage &&
             d->mNumPages == other.d->mNumPages &&
             d->mText == other.d->mText );
}


/*!
    Returns true if this cell broadcast message object is equal to \a other; otherwise returns false.
*/
bool QCBSMessage::operator!=( const QCBSMessage& other ) const
{
    return !( *this == other );
}


/*! \internal
    Print the contents of this cell broadcast message to the
    debug message stream.  For internal debugging use only.

*/
void QCBSMessage::print()
{
    qLog(Modem) << "channel=" << channel()
                  << ", scope=" << (int)scope()
                  << ", messageCode=" << (int)messageCode()
                  << ", updateNumber=" << updateNumber()
                  << ", language=" << language()
                  << ", page=" << page()
                  << ", numPages=" << numPages()
                  << ", text=" << text();
}

static QSMSDataCodingScheme bestScheme( const QString& body )
{
    QTextCodec *codec = QAtUtils::codec( "gsm-noloss" );
    uint len = body.length();
    bool gsmSafe;

    // Encode zero-length bodies in the default alphabet.
    if ( len == 0 )
        return QSMS_DefaultAlphabet;

    // Check the body for non-GSM characters.
    gsmSafe = codec->canEncode( body );

    // Use the default alphabet if everything is GSM-compatible.
    if ( gsmSafe )
        return QSMS_DefaultAlphabet;

    // Default to the UCS-2 alphabet.
    return QSMS_UCS2Alphabet;
}

/*!
    Convert this CBS message into its binary PDU form, according to
    3GPP TS 03.41 and 3GPP TS 23.041.
*/
QByteArray QCBSMessage::toPdu() const
{
    QCBSDeliverMessage deliver;
    deliver.pack( *this, bestScheme( text() ) );
    return deliver.toByteArray();

}

/*!
    Convert a binary \a pdu into a CBS message, according to
    3GPP TS 03.41 and 3GPP TS 23.041.
*/
QCBSMessage QCBSMessage::fromPdu( const QByteArray& pdu )
{
    QCBSDeliverMessage deliver( pdu );
    return deliver.unpack();
}

Q_IMPLEMENT_USER_METATYPE(QCBSMessage)
