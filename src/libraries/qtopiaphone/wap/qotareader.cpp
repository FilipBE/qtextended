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

#include <qotareader.h>

#include <qbuffer.h>
#include <qstack.h>


class QOtaContentHandler : public QXmlDefaultHandler
{
public:
    QOtaContentHandler();
    virtual ~QOtaContentHandler();

    QOtaCharacteristicList *list();

    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );

private:
    QOtaCharacteristicList *clist;
    QOtaCharacteristic *current;
    QStack<QOtaCharacteristic*> stack;
};


QOtaContentHandler::QOtaContentHandler()
{
    clist = new QOtaCharacteristicList();
    current = 0;
}

QOtaContentHandler::~QOtaContentHandler()
{
    if ( clist )
        delete clist;
    if ( current )
        delete current;
}

QOtaCharacteristicList *QOtaContentHandler::list()
{
    QOtaCharacteristicList *l = clist;
    clist = 0;
    return l;
}

bool QOtaContentHandler::startElement( const QString& , const QString& localName, const QString& , const QXmlAttributes& atts )
{
    if ( localName == "characteristic" ) {

        // Push the current characteristic onto the context stack.
        if ( current )
            stack.push( current );

        // Create a new characteristic and set its type.
        current = new QOtaCharacteristic();
        current->setType( atts.value( "type" ) );

        // If there is a "value" attribute, then add it as the default param.
        if ( ! atts.value( "value" ).isEmpty() ) {
            current->addParm( "", atts.value( "value" ) );
        }

    } else if ( localName == "parm" && current ) {

        // Add a parameter to the current characteristic.
        current->addParm( atts.value( "name" ), atts.value( "value" ) );

    }
    return true;
}

bool QOtaContentHandler::endElement( const QString& , const QString& localName, const QString& )
{
    if ( localName == "characteristic" && current ) {

        // Pop the current characteristic level from the context stack.
        QOtaCharacteristic *temp = current;
        current = ( stack.isEmpty() ? 0 : stack.pop() );

        // Add the characteristic to its surrounding context.
        if ( current ) {
            current->addChild( *temp );
        } else if ( clist->contains( temp->type() ) ) {
            (*clist)[temp->type()].append( *temp );
        } else {
            QList<QOtaCharacteristic> list;
            list.append( *temp );
            (*clist).insert( temp->type(), list );
        }

        // The characteristic has been copied: delete the original.
        delete temp;

    }
    return true;
}

class QOtaCharacteristicPrivate
{
public:
    QOtaCharacteristicPrivate()
    {
        children = 0;
    }
    ~QOtaCharacteristicPrivate()
    {
        delete children;
    }

    QString type;
    QOtaParameters parms;
    QOtaCharacteristicList *children;
};

/*!
    \class QOtaCharacteristic
    \inpublicgroup QtTelephonyModule

    \brief The QOtaCharacteristic class encapsulates characteristics from an Over-The-Air (OTA) network configuration message

    The QOtaCharacteristic class describes a network configuration
    characteristic consisting of a type, zero or more named
    parameters, and set of child characteristics.  It logically
    forms a tree of configuration values, nested one inside the other.

    Normally the programmer will not use this class, but rather access
    characteristic values via QOtaCharacteristicList::parameter() and
    QOtaCharacteristicList::appParameter().

    \ingroup telephony
    \sa QOtaReader, QOtaCharacteristicList
*/

/*!
    Construct an empty characteristic definition.
*/
QOtaCharacteristic::QOtaCharacteristic()
{
    d = new QOtaCharacteristicPrivate();
}

/*!
    Construct a copy of \a c.
*/
QOtaCharacteristic::QOtaCharacteristic( const QOtaCharacteristic& c )
{
    d = new QOtaCharacteristicPrivate();
    d->type = c.d->type;
    d->parms = c.d->parms;
    if ( d->children != c.d->children ) {
        if ( d->children )
            delete d->children;
        if ( c.d->children ) {
            d->children = new QOtaCharacteristicList();
            *(d->children) = *(c.d->children);
        } else {
            d->children = 0;
        }
    }
}

/*!
    Destruct a characteristic definition.
*/
QOtaCharacteristic::~QOtaCharacteristic()
{
    delete d;
}

/*!
    Clear this characteristic definition.
*/
void QOtaCharacteristic::clear()
{
    d->type = QString();
    d->parms.clear();
    if ( d->children )
        d->children->clear();
}

/*!
    Get the type of this characteristic definition.

    \sa setType()
*/
const QString& QOtaCharacteristic::type() const
{
    return d->type;
}

/*!
    Set the \a type of this charactieristic definition.

    \sa type()
*/
void QOtaCharacteristic::setType( const QString& type )
{
    d->type = type;
}

/*!
    Get a list of all parameters to this characteristic definition.

    \sa addParm()
*/
const QOtaParameters& QOtaCharacteristic::parms() const
{
    return d->parms;
}

/*!
    Add a parameter called \a name to this characteristic definition
    with \a value.

    \sa parms()
*/
void QOtaCharacteristic::addParm( const QString& name, const QString& value )
{
    if ( !d->parms.contains( name ) ) {
        d->parms.insert( name, value );
    } else {
        // The "DOMAIN" parameter may have multiple values.
        d->parms[name] = d->parms[name] + "," + value;
    }
}

/*!
    Get the children of this characteristic definition.

    \sa addChild()
*/
const QOtaCharacteristicList& QOtaCharacteristic::children() const
{
    if ( !(d->children) )
        d->children = new QOtaCharacteristicList();
    return *(d->children);
}

/*!
    Add a \a child to this characteristic definition.

    \sa children()
*/
void QOtaCharacteristic::addChild( QOtaCharacteristic& child )
{
    if ( !(d->children) )
        d->children = new QOtaCharacteristicList();
    if ( d->children->contains( child.type() ) ) {
        (*(d->children))[child.type()].append( child );
    } else {
        QList<QOtaCharacteristic> list;
        list.append( child );
        d->children->insert( child.type(), list );
    }
}

/*!
    Make a copy of \a c.
*/
QOtaCharacteristic& QOtaCharacteristic::operator=( const QOtaCharacteristic& c )
{
    if ( d != c.d ) {
        d->type = c.d->type;
        d->parms = c.d->parms;
        if ( d->children )
            delete d->children;
        if ( c.d->children ) {
            d->children = new QOtaCharacteristicList();
            *(d->children) = *(c.d->children);
        } else {
            d->children = 0;
        }
    }
    return *this;
}

/*!
    \class QOtaCharacteristicList
    \inpublicgroup QtTelephonyModule

    \brief The QOtaCharacteristicList class encapsulates a list of characteristics from an OTA network configuration message

    The QOtaCharacteristicList class contains a list of network configuration
    characteristics that resulted from parsing an Over-The-Air (OTA)
    network configuration message with QOtaReader::parse().

    \ingroup telephony
    \sa QOtaReader
*/

/*!
    \fn QOtaCharacteristicList::QOtaCharacteristicList()

    Construct an empty characteristics list.
*/

/*!
    \fn QOtaCharacteristicList::QOtaCharacteristicList( const QOtaCharacteristicList& list )

    Construct a copy of a specified characteristics \a list.
*/

/*!
    \fn QOtaCharacteristicList::~QOtaCharacteristicList()

    Destruct a characteristics list.
*/

/*!
    Convert an Over-The-Air (OTA) network configuration characteristics
    list into a NetworkInterfaceProperties object suitable for use
    by the Qt Extended network configuration system.
*/
QtopiaNetworkProperties QOtaCharacteristicList::toConfig() const
{
    QtopiaNetworkProperties cfg;
    QString value;

    value = parameter( "NAPDEF", "NAME" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "ISP_NAME" );
    add( cfg, "Info/Name", value );

    value = parameter( "NAPDEF", "BEARER" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "BEARER" );
    add( cfg, "Wap/Bearer", value );

    QString save = parameter("NAPDEF", "NAP-ADDRTYPE");
    if ( save.isEmpty() ) {
        //NOKIA style
        save = parameter("ADDRESS", "BEARER");
        if ( save  == "GSM/CSD" )
            add( cfg, "Serial/Phone", parameter("ADDRESS", "CSD_DIALSTRING") );
        else if ( save == "GPRS" )
            add( cfg, "Serial/APN", parameter("ADDRESS", "GPRS_ACCESSPOINTNAME") );
    } else {
        //OMA style
        value = parameter( "NAPDEF", "NAP-ADDRESS" );
        if ( save == "E164" )
            add( cfg, "Serial/Phone", value );
        else if ( save == "APN" )
            add( cfg, "Serial/APN", value );
    }

    value = parameter( "NAPDEF", "NAPAUTHINFO", "AUTHTYPE" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PPP_AUTHTYPE" );
    add( cfg, "Wap/AuthType", value );

    value = parameter( "NAPDEF", "NAPAUTHINFO", "AUTHNAME" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PPP_AUTHNAME" );
    add( cfg, "Properties/UserName", value );

    value = parameter( "NAPDEF", "NAPAUTHINFO", "AUTHSECRET" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PPP_AUTHSECRET" );
    add( cfg, "Properties/Password", value );

    value = parameter( "PXLOGICAL", "PXPHYSICAL", "PXADDR" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PROXY" );
    add( cfg, "Wap/Gateway", value );

    value = parameter( "PXLOGICAL", "PXPHYSICAL", "PXADDRTYPE" );
    add( cfg, "Wap/AddressType", value );

    value = parameter( "ADDRESS", "PROXY_LOGINTYPE" );
    add( cfg, "Wap/LoginType", value );

    value = parameter( "PXLOGICAL", "PXAUTHINFO", "PXAUTH-ID" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PROXY_AUTHNAME" );
    add( cfg, "Wap/UserName", value );

    value = parameter( "PXLOGICAL", "PXAUTHINFO", "PXAUTH-PW" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PROXY_AUTHSECRET" );
    add( cfg, "Wap/Password", value );

    value = parameter( "PXLOGICAL", "PXPHYSICAL", "PORT", "PORTNBR" );
    if ( value.isEmpty() )
        value = parameter( "ADDRESS", "PORT" );
    add( cfg, "Wap/Port", value );

    value = parameter( "PXLOGICAL", "PXPHYSICAL", "PORT", "SERVICE" );
    add( cfg, "Wap/ProxyService", value );

    value = parameter( "PXLOGICAL", "STARTPAGE" );
    if ( value.isEmpty() )
        value = appParameter( "w2", "RESOURCE", "URI" );
    if ( value.isEmpty() )
        value = parameter( "URL" );
    if ( value.isEmpty() )
        value = parameter( "BOOKMARK", "NAME" );
    add( cfg, "Wap/HomePage", value );

    value = appParameter( "w4", "ADDR" );
    if ( value.isEmpty() )
        value = parameter( "MMSURL" );
    add( cfg, "MMS/Server", value );



    return cfg;
}

/*!
    Return the value of a top-level parameter called \a name1.

    \sa appParameter()
*/
QString QOtaCharacteristicList::parameter( const QString& name1 ) const
{
    const QOtaParameters *params = section( name1 );
    if ( params && params->contains( "" ) )
        return (*params)[""];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name2 under the
    top-level section \a name1.

    \sa appParameter()
*/
QString QOtaCharacteristicList::parameter
        ( const QString& name1, const QString& name2 ) const
{
    const QOtaParameters *params = section( name1 );
    if ( params && params->contains( name2 ) )
        return (*params)[name2];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name3 under the
    sub-section \a name2 under the top-level section \a name1.

    \sa appParameter()
*/
QString QOtaCharacteristicList::parameter
    ( const QString& name1, const QString& name2, const QString& name3 ) const
{
    const QOtaParameters *params = section( name1, name2 );
    if ( params && params->contains( name3 ) )
        return (*params)[name3];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name4 under the
    sub-section \a name3 under sub-section \a name2 under the
    top-level section \a name1.

    \sa appParameter()
*/
QString QOtaCharacteristicList::parameter
        ( const QString& name1, const QString& name2,
          const QString& name3, const QString& name4 ) const
{
    const QOtaParameters *params = section( name1, name2, name3 );
    if ( params && params->contains( name4 ) )
        return (*params)[name4];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name1 under the
    application-specific data for \a app.

    \sa parameter()
*/
QString QOtaCharacteristicList::appParameter
        ( const QString& app, const QString& name1 ) const
{
    const QOtaCharacteristic *appChars = appSection( app );
    if ( appChars && appChars->parms().contains( name1 ) )
        return appChars->parms()[name1];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name2 under the
    section \a name1 within the application-specific data for \a app.

    \sa parameter()
*/
QString QOtaCharacteristicList::appParameter
    ( const QString& app, const QString& name1, const QString& name2 ) const
{
    const QOtaParameters *params = appSection( app, name1 );
    if ( params && params->contains( name2 ) )
        return (*params)[name2];
    else
        return QString();
}

/*!
    Return the value of a parameter called \a name3 under the
    sub-section \a name2 and section \a name1 within the
    application-specific data for \a app.

    \sa parameter()
*/
QString QOtaCharacteristicList::appParameter
        ( const QString& app, const QString& name1,
          const QString& name2, const QString& name3 ) const
{
    const QOtaParameters *params = appSection( app, name1, name2 );
    if ( params && params->contains( name3 ) )
        return (*params)[name3];
    else
        return QString();
}

/*!
    Return the parmeters in the top-level section \a name1.

    \sa appSection()
*/
const QOtaParameters *QOtaCharacteristicList::section
        ( const QString& name1 ) const
{
    if ( contains( name1 ) ) {
        const QList<QOtaCharacteristic>& value = (*this)[name1];
        if ( value.count() > 0 ) {
            return &(value[0].parms());
        }
    }
    return 0;
}

/*!
    Return the section called \a name2 under the top-level section \a name1.

    \sa appSection()
*/
const QOtaParameters *QOtaCharacteristicList::section
        ( const QString& name1, const QString& name2 ) const
{
    if ( contains( name1 ) ) {
        const QList<QOtaCharacteristic>& value = (*this)[name1];
        if ( value.count() > 0 ) {
            if ( value[0].children().contains( name2 ) ) {
                const QList<QOtaCharacteristic>& value2
                    = value[0].children()[name2];
                if ( value2.count() > 0 ) {
                    return &(value2[0].parms());
                }
            }
        }
    }
    return 0;
}

/*!
    Return the section called \a name3 under the sub-section \a name2
    under the top-level section \a name1.

    \sa appSection()
*/
const QOtaParameters *QOtaCharacteristicList::section
    ( const QString& name1, const QString& name2, const QString& name3 ) const
{
    if ( contains( name1 ) ) {
        const QList<QOtaCharacteristic>& value = (*this)[name1];
        if ( value.count() > 0 ) {
            if ( value[0].children().contains( name2 ) ) {
                const QList<QOtaCharacteristic>& value2
                    = value[0].children()[name2];
                if ( value2.count() > 0 ) {
                    if ( value2[0].children().contains( name3 ) ) {
                        const QList<QOtaCharacteristic>& value3
                            = value2[0].children()[name3];
                        if ( value3.count() > 0 ) {
                            return &(value3[0].parms());
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*!
    Return the parameters under the application-specific data for \a app.

    \sa section()
*/
const QOtaCharacteristic *QOtaCharacteristicList::appSection
        ( const QString& app ) const
{
    if ( contains( "APPLICATION" ) ) {
        const QList<QOtaCharacteristic>& value = (*this)["APPLICATION"];
        for ( int posn = 0; posn < value.count(); ++posn ) {
            if ( value[posn].parms().contains( "APPID" ) &&
                 value[posn].parms()["APPID"] == app ) {
                return &(value[posn]);
            }
        }
    }
    return 0;
}

/*!
    Return the section called \a name1 under the application-specific
    data for \a app.

    \sa section()
*/
const QOtaParameters *QOtaCharacteristicList::appSection
        ( const QString& app, const QString& name1 ) const
{
    const QOtaCharacteristic *appChars = appSection( app );
    if ( appChars && appChars->children().contains( name1 ) ) {
        const QList<QOtaCharacteristic>& value
            = appChars->children()[name1];
        if ( value.count() > 0 ) {
            return &(value[0].parms());
        }
    }
    return 0;
}

/*!
    Return the section called \a name2 under the section \a name1 within
    the application-specific data for \a app.

    \sa section()
*/
const QOtaParameters *QOtaCharacteristicList::appSection
    ( const QString& app, const QString& name1, const QString& name2 ) const
{
    const QOtaCharacteristic *appChars = appSection( app );
    if ( appChars && appChars->children().contains( name1 ) ) {
        const QList<QOtaCharacteristic>& value
            = appChars->children()[name1];
        if ( value.count() > 0 ) {
            if ( value[0].children().contains( name2 ) ) {
                const QList<QOtaCharacteristic>& value2
                    = value[0].children()[name2];
                if ( value2.count() > 0 ) {
                    return &(value2[0].parms());
                }
            }
        }
    }
    return 0;
}

void QOtaCharacteristicList::add
    ( QtopiaNetworkProperties& cfg, const QString& name, const QString& value )
{
    if ( !value.isEmpty() )
        cfg.insert( name, value );
}

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    \class QOtaReader
    \inpublicgroup QtTelephonyModule

    \brief The QOtaReader class provides support for parsing Over-The-Air (OTA) network configuration messages

    The QOtaReader class provides support for reading Over-The-Air (OTA)
    network configuration messages.  Two distinct formats are supported:

    \list
      \o \c{QOtaReader::Nokia} - Nokia Over The Air Settings Specification 7.0.
      \o \c{QOtaReader::Wap} - WAP/OMA standards \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-183-provcont-20010724-a.pdf}{wap-183-provcont-20010724-a.pdf}
         and \l{http://www.openmobilealliance.org/release_program/docs/ClientProv/V1_1-20050428-C/OMA-WAP-ProvCont-v1_1-20050428-C.pdf}{OMA-WAP-ProvCont-v1_1-20050428-C.pdf}.
    \endlist

    \ingroup telephony
    \sa QWbXmlReader, QOtaCharacteristicList
*/

/*!
    \enum QOtaReader::QOtaType
    Defines the format for Over-The-Air (OTA) network configuration messages.

    \value Nokia Nokia Over The Air Settings Specification 7.0.
    \value Wap WAP/OMA standards \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-183-provcont-20010724-a.pdf}{wap-183-provcont-20010724-a.pdf}
           and \l{http://www.openmobilealliance.org/release_program/docs/ClientProv/V1_1-20050428-C/OMA-WAP-ProvCont-v1_1-20050428-C.pdf}{OMA-WAP-ProvCont-v1_1-20050428-C.pdf}.
*/

/*!
    Construct a new Over-The-Air (OTA) network configuration reader
    of the specified \a type.
*/
QOtaReader::QOtaReader( QOtaType type )
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    if ( type == QOtaReader::Wap ) {

        // Tag and attribute lists based on "wap-183-provcont-20010724-a.pdf"
        // and "OMA-WAP-ProvCont-V1_1-20021112-C.pdf".

        set(tags, 0x05, "wap-provisioningdoc");
        set(tags, 0x06, "characteristic");
        set(tags, 0x07, "parm");
        set(tags, 0x46, "characteristic");
        set(tags, 0x47, "parm");

        set(attrs, 0x05, "name");
        set(attrs, 0x06, "value");
        set(attrs, 0x07, "name=NAME");
        set(attrs, 0x08, "name=NAP-ADDRESS");
        set(attrs, 0x09, "name=NAP-ADDRTYPE");
        set(attrs, 0x0A, "name=CALLTYPE");
        set(attrs, 0x0B, "name=VALIDUNTIL");
        set(attrs, 0x0C, "name=AUTHTYPE");
        set(attrs, 0x0D, "name=AUTHNAME");
        set(attrs, 0x0E, "name=AUTHSECRET");
        set(attrs, 0x0F, "name=LINGER");
        set(attrs, 0x10, "name=BEARER");
        set(attrs, 0x11, "name=NAPID");
        set(attrs, 0x12, "name=COUNTRY");
        set(attrs, 0x13, "name=NETWORK");
        set(attrs, 0x14, "name=INTERNET");
        set(attrs, 0x15, "name=PROXY-ID");
        set(attrs, 0x16, "name=PROXY-PROVIDER-ID");
        set(attrs, 0x17, "name=DOMAIN");
        set(attrs, 0x18, "name=PROVURL");
        set(attrs, 0x19, "name=PXAUTH-TYPE");
        set(attrs, 0x1A, "name=PXAUTH-ID");
        set(attrs, 0x1B, "name=PXAUTH-PW");
        set(attrs, 0x1C, "name=STARTPAGE");
        set(attrs, 0x1D, "name=BASAUTH-ID");
        set(attrs, 0x1E, "name=BASAUTH-PW");
        set(attrs, 0x1F, "name=PUSHENABLED");
        set(attrs, 0x20, "name=PXADDR");
        set(attrs, 0x21, "name=PXADDRTYPE");
        set(attrs, 0x22, "name=TO-NAPID");
        set(attrs, 0x23, "name=PORTNBR");
        set(attrs, 0x24, "name=SERVICE");
        set(attrs, 0x25, "name=LINKSPEED");
        set(attrs, 0x26, "name=DNLINKSPEED");
        set(attrs, 0x27, "name=LOCAL-ADDR");
        set(attrs, 0x28, "name=LOCAL-ADDRTYPE");
        set(attrs, 0x29, "name=CONTEXT-ALLOW");
        set(attrs, 0x2A, "name=TRUST");
        set(attrs, 0x2B, "name=MASTER");
        set(attrs, 0x2C, "name=SID");
        set(attrs, 0x2D, "name=SOC");
        set(attrs, 0x2E, "name=WSP-VERSION");
        set(attrs, 0x2F, "name=PHYSICAL-PROXY-ID");
        set(attrs, 0x30, "name=CLIENT-ID");
        set(attrs, 0x31, "name=DELIVERY-ERR-SDU");
        set(attrs, 0x32, "name=DELIVERY-ORDER");
        set(attrs, 0x33, "name=TRAFFIC-CLASS");
        set(attrs, 0x34, "name=MAX-SDU-SIZE");
        set(attrs, 0x35, "name=MAX-BITRATE-UPLINK");
        set(attrs, 0x36, "name=MAX-BITRATE-DNLINK");
        set(attrs, 0x37, "name=RESIDUAL-BER");
        set(attrs, 0x38, "name=SDU-ERROR-RATIO");
        set(attrs, 0x39, "name=TRAFFIC-HANDL-PRIO");
        set(attrs, 0x3A, "name=TRANSFER-DELAY");
        set(attrs, 0x3B, "name=GUARANTEED-BITRATE-UPLINK");
        set(attrs, 0x3C, "name=GUARANTEED-BITRATE-DNLINK");
        set(attrs, 0x3D, "name=PXADDR-FQDN");
        set(attrs, 0x3E, "name=PROXY-PW");
        set(attrs, 0x3F, "name=PPGAUTH-TYPE");
        set(attrs, 0x45, "version");
        set(attrs, 0x46, "version=1.0");
        set(attrs, 0x47, "name=PULLENABLED");
        set(attrs, 0x48, "name=DNS-ADDR");
        set(attrs, 0x49, "name=MAX-NUM-RETRY");
        set(attrs, 0x4A, "name=FIRST-RETRY-TIMEOUT");
        set(attrs, 0x4B, "name=REREG-THRESHOLD");
        set(attrs, 0x4C, "name=T-BIT");
        set(attrs, 0x4E, "name=AUTH-ENTITY");
        set(attrs, 0x4F, "name=SPI");
        set(attrs, 0x50, "type");
        set(attrs, 0x51, "type=PXLOGICAL");
        set(attrs, 0x52, "type=PXPHYSICAL");
        set(attrs, 0x53, "type=PORT");
        set(attrs, 0x54, "type=VALIDITY");
        set(attrs, 0x55, "type=NAPDEF");
        set(attrs, 0x56, "type=BOOTSTRAP");
        set(attrs, 0x57, "type=VENDORCONFIG");
        set(attrs, 0x58, "type=CLIENTIDENTITY");
        set(attrs, 0x59, "type=PXAUTHINFO");
        set(attrs, 0x5A, "type=NAPAUTHINFO");
        set(attrs, 0x5B, "type=ACCESS");
        set(attrs, 0x85, "IPV4");
        set(attrs, 0x86, "IPV6");
        set(attrs, 0x87, "E164");
        set(attrs, 0x88, "ALPHA");
        set(attrs, 0x89, "APN");
        set(attrs, 0x8A, "SCODE");
        set(attrs, 0x8B, "TETRA-ITSI");
        set(attrs, 0x8C, "MAN");
        set(attrs, 0x90, "ANALOG-MODEM");
        set(attrs, 0x91, "V.120");
        set(attrs, 0x92, "V.110");
        set(attrs, 0x93, "X.31");
        set(attrs, 0x94, "BIT-TRANSPARENT");
        set(attrs, 0x95, "DIRECT-ASYNCHRONOUS-DATA-SERVICE");
        set(attrs, 0x9A, "PAP");
        set(attrs, 0x9B, "CHAP");
        set(attrs, 0x9C, "HTTP-BASIC");
        set(attrs, 0x9D, "HTTP-DIGEST");
        set(attrs, 0x9E, "WTLS-SS");
        set(attrs, 0x9F, "MD5");
        set(attrs, 0xA2, "GSM-USSD");
        set(attrs, 0xA3, "GSM-SMS");
        set(attrs, 0xA4, "ANSI-136-GUTS");
        set(attrs, 0xA5, "IS-95-CDMA-SMS");
        set(attrs, 0xA6, "IS-95-CDMA-CSD");
        set(attrs, 0xA7, "IS-95-CDMA-PACKET");
        set(attrs, 0xA8, "ANSI-136-CSD");
        set(attrs, 0xA9, "ANSI-136-GPRS");
        set(attrs, 0xAA, "GSM-CSD");
        set(attrs, 0xAB, "GSM-GPRS");
        set(attrs, 0xAC, "AMPS-CDPD");
        set(attrs, 0xAD, "PDC-CSD");
        set(attrs, 0xAE, "PDC-PACKET");
        set(attrs, 0xAF, "IDEN-SMS");
        set(attrs, 0xB0, "IDEN-CSD");
        set(attrs, 0xB1, "IDEN-PACKET");
        set(attrs, 0xB2, "FLEX/REFLEX");
        set(attrs, 0xB3, "PHS-SMS");
        set(attrs, 0xB4, "PHS-CSD");
        set(attrs, 0xB5, "TETRA-SDS");
        set(attrs, 0xB6, "TETRA-PACKET");
        set(attrs, 0xB7, "ANSI-136-GHOST");
        set(attrs, 0xB8, "MOBITEX-MPAK");
        set(attrs, 0xB9, "CDMA2000-1X-SIMPLE-IP");
        set(attrs, 0xBA, "CDMA2000-1X-MOBILE-IP");
        set(attrs, 0xC5, "AUTOBAUDING");
        set(attrs, 0xCA, "CL-WSP");
        set(attrs, 0xCB, "CO-WSP");
        set(attrs, 0xCC, "CL-SEC-WSP");
        set(attrs, 0xCD, "CO-SEC-WSP");
        set(attrs, 0xCE, "CL-SEC-WTA");
        set(attrs, 0xCF, "CO-SEC-WTA");
        set(attrs, 0xD0, "OTA-HTTP-TO");
        set(attrs, 0xD1, "OTA-HTTP-TLS-TO");
        set(attrs, 0xD2, "OTA-HTTP-PO");
        set(attrs, 0xD3, "OTA-HTTP-TLS-PO");
        set(attrs, 0xE0, "AAA");
        set(attrs, 0xE1, "HA");
        set(attrs, 0x105, "name");
        set(attrs, 0x106, "value");
        set(attrs, 0x107, "name=NAME");
        set(attrs, 0x114, "name=INTERNET");
        set(attrs, 0x11C, "name=STARTPAGE");
        set(attrs, 0x122, "name=TO-NAPID");
        set(attrs, 0x123, "name=PORTNBR");
        set(attrs, 0x124, "name=SERVICE");
        set(attrs, 0x12E, "name=AACCEPT");
        set(attrs, 0x12F, "name=AAUTHDATA");
        set(attrs, 0x130, "name=AAUTHLEVEL");
        set(attrs, 0x131, "name=AAUTHNAME");
        set(attrs, 0x132, "name=AAUTHSECRET");
        set(attrs, 0x133, "name=AAUTHTYPE");
        set(attrs, 0x134, "name=ADDR");
        set(attrs, 0x135, "name=ADDRTYPE");
        set(attrs, 0x136, "name=APPID");
        set(attrs, 0x137, "name=APROTOCOL");
        set(attrs, 0x138, "name=PROVIDER-ID");
        set(attrs, 0x139, "name=TO-PROXY");
        set(attrs, 0x13A, "name=URI");
        set(attrs, 0x13B, "name=RULE");
        set(attrs, 0x150, "type");
        set(attrs, 0x153, "type=PORT");
        set(attrs, 0x155, "type=APPLICATION");
        set(attrs, 0x156, "type=APPADDR");
        set(attrs, 0x157, "type=APPAUTH");
        set(attrs, 0x158, "type=CLIENTIDENTITY");
        set(attrs, 0x159, "type=RESOURCE");
        set(attrs, 0x180, ",");         // OMA-WAP-ProvCont-V1_1-20021112-C.pdf
        set(attrs, 0x181, "HTTP-");     // OMA-WAP-ProvCont-V1_1-20021112-C.pdf
        set(attrs, 0x182, "BASIC");     // OMA-WAP-ProvCont-V1_1-20021112-C.pdf
        set(attrs, 0x183, "DIGEST");    // OMA-WAP-ProvCont-V1_1-20021112-C.pdf
        set(attrs, 0x186, "IPV6");
        set(attrs, 0x187, "E164");
        set(attrs, 0x188, "ALPHA");
        set(attrs, 0x18D, "APPSRV");
        set(attrs, 0x18E, "OBEX");
        set(attrs, 0x190, ",");         // OMA-WAP-ProvCont-v1_1-20050428-C.pdf
        set(attrs, 0x191, "HTTP-");     // OMA-WAP-ProvCont-v1_1-20050428-C.pdf
        set(attrs, 0x192, "BASIC");     // OMA-WAP-ProvCont-v1_1-20050428-C.pdf
        set(attrs, 0x193, "DIGEST");    // OMA-WAP-ProvCont-v1_1-20050428-C.pdf

    } else {

        // Nokia Over The Air Settings Specification 7.0.

        set(tags, 0x05, "characteristic-list");
        set(tags, 0x06, "characteristic");
        set(tags, 0x07, "parm");

        set(attrs, 0x06, "type=ADDRESS");
        set(attrs, 0x07, "type=URL");
        set(attrs, 0x08, "type=NAME");
        set(attrs, 0x10, "name");
        set(attrs, 0x11, "value");
        set(attrs, 0x12, "name=BEARER");
        set(attrs, 0x13, "name=PROXY");
        set(attrs, 0x14, "name=PORT");
        set(attrs, 0x15, "name=NAME");
        set(attrs, 0x16, "name=PROXY_TYPE");
        set(attrs, 0x17, "name=URL");
        set(attrs, 0x18, "name=PROXY_AUTHNAME");
        set(attrs, 0x19, "name=PROXY_AUTHSECRET");
        set(attrs, 0x1A, "name=SMS_SMSC_ADDRESS");
        set(attrs, 0x1B, "name=USSD_SERVICE_CODE");
        set(attrs, 0x1C, "name=GPRS_ACCESSPOINTNAME");
        set(attrs, 0x1D, "name=PPP_LOGINTYPE");
        set(attrs, 0x1E, "name=PROXY_LOGINTYPE");
        set(attrs, 0x21, "name=CSD_DIALSTRING");
        set(attrs, 0x22, "name=PPP_AUTHTYPE");
        set(attrs, 0x23, "name=PPP_AUTHNAME");
        set(attrs, 0x24, "name=PPP_AUTHSECRET");
        set(attrs, 0x28, "name=CSD_CALLTYPE");
        set(attrs, 0x29, "name=CSD_CALLSPEED");
        set(attrs, 0x45, "value=GSM/CSD");
        set(attrs, 0x46, "value=GSM/SMS");
        set(attrs, 0x47, "value=GSM/USSD");
        set(attrs, 0x48, "value=IS-136/CSD");
        set(attrs, 0x49, "value=GPRS");
        set(attrs, 0x60, "value=9200");
        set(attrs, 0x61, "value=9201");
        set(attrs, 0x62, "value=9202");
        set(attrs, 0x63, "value=9203");
        set(attrs, 0x64, "value=AUTOMATIC");
        set(attrs, 0x65, "value=MANUAL");
        set(attrs, 0x6A, "value=AUTO");
        set(attrs, 0x6B, "value=9600");
        set(attrs, 0x6C, "value=14400");
        set(attrs, 0x6D, "value=19200");
        set(attrs, 0x6E, "value=28880");
        set(attrs, 0x6F, "value=38400");
        set(attrs, 0x70, "value=PAP");
        set(attrs, 0x71, "value=CHAP");
        set(attrs, 0x72, "value=ANALOGUE");
        set(attrs, 0x73, "value=ISDN");
        set(attrs, 0x74, "value=43200");
        set(attrs, 0x75, "value=57600");
        set(attrs, 0x76, "value=MSISDN_NO");
        set(attrs, 0x77, "value=IPV4");
        set(attrs, 0x78, "value=MS_CHAP");
        set(attrs, 0x7C, "type=MMSURL");
        set(attrs, 0x7D, "type=ID");
        set(attrs, 0x7E, "name=ISP_NAME");
        set(attrs, 0x7F, "type=BOOKMARK");
    }

    setTagSets( tags, attrs );
}

/*!
    Destruct an OTA reader.
*/
QOtaReader::~QOtaReader()
{
}

/*!
    Parse a WBXML-encoded OTA \a input message and return a characteristics list
    that describes its contents.
*/
QOtaCharacteristicList *QOtaReader::parseCharacteristics( const QByteArray& input )
{
    QByteArray data( input );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    return parseCharacteristics( buffer );
}

/*!
    Parse a WBXML-encoded OTA \a input message and return a characteristics list
    that describes its contents.
*/
QOtaCharacteristicList *QOtaReader::parseCharacteristics( QIODevice& input )
{
    QXmlContentHandler *oldHandler = contentHandler();
    QOtaContentHandler handler;
    setContentHandler( &handler );
    bool result = parse( input );
    setContentHandler( oldHandler );
    if ( result )
        return handler.list();
    else
        return 0;
}

/*!
    Parse a plain-text XML OTA \a input message with \a reader and return a
    characteristics list that describes its contents.  This function is
    intended for non-binary versions of network configuration messages.
*/
QOtaCharacteristicList *QOtaReader::parseCharacteristics
    ( QXmlReader *reader, const QXmlInputSource& input )
{
    QXmlContentHandler *oldHandler = reader->contentHandler();
    QOtaContentHandler handler;
    reader->setContentHandler( &handler );
    bool result = reader->parse( input );
    reader->setContentHandler( oldHandler );
    if ( result )
        return handler.list();
    else
        return 0;
}

