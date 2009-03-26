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
#include "qmapi.h"

#include <trace.h>

#include <QFile>
#include <QString>
#include <QVariant>
#include <qdebug.h>

#define INITGUID
#define USES_IID_IMessage
#include <mapix.h>
#include <mapiutil.h>

#include <windows.h>
#include <unknwn.h>

//#define QMAPI_DEBUG
QD_LOG_OPTION(QMAPI)

// =====================================================================

using namespace QMAPI;

#ifdef QMAPI_DEBUG
#define CLEANUP_LOG(message) WARNING() << __FILE__ << __LINE__ << message << hrtext(hr);
#else
#define CLEANUP_LOG(message) WARNING() << __FILE__ << __LINE__ << message;
#endif

#define CLEANUP(test,message)\
    do {\
        if ( test ) {\
            CLEANUP_LOG(message)\
            goto cleanup;\
        }\
    } while ( 0 )

#undef DEFINE_GUID
#undef DEFINE_OLEGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define DEFINE_OLEGUID(name, l, w1, w2) DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)
DEFINE_OLEGUID(OUTLOOK_GUID, 0x00062004, 0, 0); // {00062004-0000-0000-C000-000000000046}
DEFINE_OLEGUID(BIRTHDAY_GUID, 0x00062008, 0, 0); // {00062008-0000-0000-C000-000000000046}

#ifdef QMAPI_DEBUG
#include "mapidebug.cpp"
#endif

// =====================================================================

static QVariant value( LPSPropValue pv )
{
    QVariant ret;

    switch ( PROP_TYPE(pv->ulPropTag) ) {
        case PT_BOOLEAN:
            ret = pv->Value.b;
            break;
        case PT_I2:
            ret = pv->Value.i;
            break;
        case PT_LONG:
            ret = pv->Value.l;
            break;
        case PT_DOUBLE:
            ret = pv->Value.dbl;
            break;
        case PT_STRING8:
            ret = QString::fromLocal8Bit(pv->Value.lpszA);
            break;
        case PT_UNICODE:
            ret = QString::fromUtf16(pv->Value.lpszW);
            break;
        default:
#ifdef QMAPI_DEBUG
            ret = propname(PROP_TYPE(pv->ulPropTag));
#else
            ret = QVariant();
#endif
            break;
    }

    return ret;
}

// =====================================================================

namespace QMAPI {

    class SessionPrivate
    {
    public:
        SessionPrivate( Session *q );
        ~SessionPrivate();

        Session *q;
        LPMAPINAMEID emailNames[3];
        bool emailNamesInit;
        LPMAPINAMEID birthdayNames[1];
        bool birthdayNamesInit;
    };

};

// =====================================================================

namespace QMAPI {

    class ContactPrivate
    {
    public:
        QString Body;
        QString Email1Address;
        QString Email2Address;
        QString Email3Address;
    };

    class AppointmentPrivate
    {
    public:
        QString Body;
        bool IsBirthday;
    };

    class TaskPrivate
    {
    public:
        QString Body;
    };

};

// =====================================================================

SessionPrivate::SessionPrivate( Session *_q )
    : q( _q ), emailNamesInit( true ), birthdayNamesInit( true )
{
}

SessionPrivate::~SessionPrivate()
{
}

// =====================================================================

// Connect to the MAPI subsystem
Session::Session( QObject *parent )
    : QObject( parent ), d( 0 )
{
    TRACE(QMAPI) << "Session::Session";

    // Grab the MAPI subsystem
    HRESULT hr = MAPIInitialize(0);
    if ( hr != S_OK ) goto cleanup;
    CLEANUP(( hr != S_OK ), "Can't initialize MAPI");

    d = new SessionPrivate( this );

cleanup:
    ; // NOP
}

// Disconnect from the MAPI subsystem
Session::~Session()
{
    TRACE(QMAPI) << "Session::~Session";

    delete d;

    // Release the MAPI subsystem
    MAPIUninitialize();
}

bool Session::connected() const
{
    return (d != 0);
}

QMAPI::Contact *Session::openContact( IUnknown *obj )
{
    TRACE(QMAPI) << "Session::openContact";
    if ( !obj ) return 0;

    QMAPI::Contact *ret = 0;

    HRESULT hr;
    LPMESSAGE lpMsg = 0;
    LPSPropTagArray lpProps = 0;
    ULONG ulSz;
    LPSPropValue lpValues = 0;
    SizedSPropTagArray(5,props);
    int propCount;
    QString messageClass;
    bool messageClassOk;

    hr = obj->QueryInterface(IID_IMessage, (void**)&lpMsg);
    CLEANUP(( FAILED(hr) || !lpMsg ), "Didn't get an IID_IMessage");

    if ( d->emailNamesInit ) {
        d->emailNamesInit = false;

        for ( int i = 0; i < 3; i++ ) {
            d->emailNames[i] = new MAPINAMEID;
            d->emailNames[i]->lpguid = (LPGUID)&OUTLOOK_GUID;
            d->emailNames[i]->ulKind = MNID_ID;
        }
        d->emailNames[0]->Kind.lID = 0x8084; // Email1Address
        d->emailNames[1]->Kind.lID = 0x8094; // Email2Address
        d->emailNames[2]->Kind.lID = 0x80A4; // Email3Address
    }

    hr = lpMsg->GetIDsFromNames( 3, d->emailNames, 0, &lpProps );
    CLEANUP(( FAILED(hr) || !lpProps || lpProps->cValues != 3 ), "Can't get properties");

    propCount = 5;
    for ( int i = 0; i < 3; i++ ) {
        ULONG tag = lpProps->aulPropTag[i];
        if ( PROP_TYPE(tag) == PT_ERROR ) {
            // FIXME user notification?
            WARNING() << "Can't resolve the email address named properties!";
            propCount = 2;
            break;
        }
    }

    props.cValues = propCount;
    props.aulPropTag[0] = PR_MESSAGE_CLASS;
    props.aulPropTag[1] = PR_BODY;
    props.aulPropTag[2] = lpProps->aulPropTag[0];
    props.aulPropTag[3] = lpProps->aulPropTag[1];
    props.aulPropTag[4] = lpProps->aulPropTag[2];

    // Get the properties
    hr = lpMsg->GetProps( (LPSPropTagArray)&props, MAPI_UNICODE, &ulSz, &lpValues );
    CLEANUP(( FAILED(hr) || !lpValues ), "Can't get properties");
    CLEANUP(( ulSz != propCount ), "Not enough properties");

    messageClass = value(&lpValues[0]).toString();
    messageClassOk = false;
    if ( messageClass == "IPM.Contact" )
        messageClassOk = true;
    if ( !messageClassOk ) {
        WARNING() << "Unknown PR_MESSAGE_CLASS" << messageClass;
        goto cleanup;
    }

    ret = new QMAPI::Contact( this );
    if ( PROP_TYPE(lpValues[1].ulPropTag) != PT_ERROR )
        ret->d->Body = value(&lpValues[1]).toString();
    if ( PROP_TYPE(lpValues[2].ulPropTag) != PT_ERROR )
        ret->d->Email1Address = value(&lpValues[2]).toString();
    if ( PROP_TYPE(lpValues[3].ulPropTag) != PT_ERROR )
        ret->d->Email2Address = value(&lpValues[3]).toString();
    if ( PROP_TYPE(lpValues[4].ulPropTag) != PT_ERROR )
        ret->d->Email3Address = value(&lpValues[4]).toString();

cleanup:
    if ( lpValues ) MAPIFreeBuffer( lpValues );
    if ( lpProps ) MAPIFreeBuffer( lpProps );
    if ( lpMsg ) lpMsg->Release();

    return ret;
}

QMAPI::Appointment *Session::openAppointment( IUnknown *obj, bool isException )
{
    TRACE(QMAPI) << "Session::openAppointment";
    if ( !obj ) return 0;

    QMAPI::Appointment *ret = 0;

    HRESULT hr;
    LPMESSAGE lpMsg = 0;
    LPSPropTagArray lpProps = 0;
    ULONG ulSz;
    LPSPropValue lpValues = 0;
    SizedSPropTagArray(3,props);
    int propCount;
    QString messageClass;
    bool messageClassOk;

    hr = obj->QueryInterface(IID_IMessage, (void**)&lpMsg);
    CLEANUP(( FAILED(hr) || !lpMsg ), "Didn't get an IID_IMessage");

    if ( d->birthdayNamesInit ) {
        d->birthdayNamesInit = false;

        for ( int i = 0; i < 1; i++ ) {
            d->birthdayNames[i] = new MAPINAMEID;
            d->birthdayNames[i]->lpguid = (LPGUID)&BIRTHDAY_GUID;
            d->birthdayNames[i]->ulKind = MNID_ID;
        }
        d->birthdayNames[0]->Kind.lID = 0x8586; // Contact name (for birthdays and anniversaries)
    }

    hr = lpMsg->GetIDsFromNames( 1, d->birthdayNames, 0, &lpProps );
    CLEANUP(( FAILED(hr) || !lpProps || lpProps->cValues != 1 ), "Can't get properties");

    propCount = 3;
    for ( int i = 0; i < 1; i++ ) {
        ULONG tag = lpProps->aulPropTag[i];
        if ( PROP_TYPE(tag) == PT_ERROR ) {
            // FIXME user notification?
            WARNING() << "Can't resolve the birthday named property!";
            propCount = 2;
            break;
        }
    }

    props.cValues = propCount;
    props.aulPropTag[0] = PR_MESSAGE_CLASS;
    props.aulPropTag[1] = PR_BODY;
    props.aulPropTag[2] = lpProps->aulPropTag[0];

    // Get the properties
    hr = lpMsg->GetProps( (LPSPropTagArray)&props, MAPI_UNICODE, &ulSz, &lpValues );
    CLEANUP(( FAILED(hr) || !lpValues ), "Can't get properties");
    CLEANUP(( ulSz != propCount ), "Not enough properties");

    messageClass = value(&lpValues[0]).toString();
    messageClassOk = false;
    if ( isException && messageClass == "IPM.OLE.CLASS.{00061055-0000-0000-C000-000000000046}" )
        messageClassOk = true;
    if ( !isException && messageClass == "IPM.Appointment" )
        messageClassOk = true;
    if ( !messageClassOk ) {
        WARNING() << "Unknown PR_MESSAGE_CLASS" << messageClass;
        goto cleanup;
    }

    ret = new QMAPI::Appointment( this );
    if ( PROP_TYPE(lpValues[1].ulPropTag) != PT_ERROR )
        ret->d->Body = value(&lpValues[1]).toString();
    if ( PROP_TYPE(lpValues[2].ulPropTag) != PT_ERROR )
        ret->d->IsBirthday = true;
    else
        ret->d->IsBirthday = false;

cleanup:
    if ( lpValues ) MAPIFreeBuffer( lpValues );
    if ( lpProps ) MAPIFreeBuffer( lpProps );
    if ( lpMsg ) lpMsg->Release();

    return ret;
}

QMAPI::Task *Session::openTask( IUnknown *obj )
{
    TRACE(QMAPI) << "Session::openTask";
    if ( !obj ) return 0;

    QMAPI::Task *ret = 0;

    HRESULT hr;
    LPMESSAGE lpMsg = 0;
    ULONG ulSz;
    LPSPropValue lpValues = 0;
    SizedSPropTagArray(2,props);
    int propCount = 2;
    QString messageClass;

    hr = obj->QueryInterface(IID_IMessage, (void**)&lpMsg);
    CLEANUP(( FAILED(hr) || !lpMsg ), "Didn't get an IID_IMessage");

    props.cValues = propCount;
    props.aulPropTag[0] = PR_MESSAGE_CLASS;
    props.aulPropTag[1] = PR_BODY;

    // Get the properties
    hr = lpMsg->GetProps( (LPSPropTagArray)&props, MAPI_UNICODE, &ulSz, &lpValues );
    CLEANUP(( FAILED(hr) || !lpValues ), "Can't get properties");
    CLEANUP(( ulSz != propCount ), "Not enough properties");

    messageClass = value(&lpValues[0]).toString();
    if ( messageClass != "IPM.Task" ) {
        WARNING() << "Unknown PR_MESSAGE_CLASS" << messageClass;
        goto cleanup;
    }


    ret = new QMAPI::Task( this );
    if ( PROP_TYPE(lpValues[1].ulPropTag) != PT_ERROR )
        ret->d->Body = value(&lpValues[1]).toString();

cleanup:
    if ( lpValues ) MAPIFreeBuffer( lpValues );
    if ( lpMsg ) lpMsg->Release();

    return ret;
}

// =====================================================================

Contact::Contact( QObject *parent )
    : QObject( parent ), d( 0 )
{
    d = new ContactPrivate;
}

Contact::~Contact()
{
    delete d;
}

QString Contact::Body()
{
    return d->Body;
}

QString Contact::Email1Address()
{
    return d->Email1Address;
}

QString Contact::Email2Address()
{
    return d->Email2Address;
}

QString Contact::Email3Address()
{
    return d->Email3Address;
}

// =====================================================================

Appointment::Appointment( QObject *parent )
    : QObject( parent ), d( 0 )
{
    d = new AppointmentPrivate;
}

Appointment::~Appointment()
{
    delete d;
}

QString Appointment::Body()
{
    return d->Body;
}

bool Appointment::IsBirthday()
{
    return d->IsBirthday;
}

// =====================================================================

Task::Task( QObject *parent )
    : QObject( parent ), d( 0 )
{
    d = new TaskPrivate;
}

Task::~Task()
{
    delete d;
}

QString Task::Body()
{
    return d->Body;
}

