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

#include <QUuid>
#include <QQueue>

#include <qmediasessionrequest.h>


// {{{ QMediaSessionRequestPrivate
class QMediaSessionRequestPrivate
{
public:
    QUuid               id;
    QString             domain;
    QString             type;
    QQueue<QVariant>    requestData;
};
// }}}


/*!
    \class QMediaSessionRequest
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaSessionRequest class provides an object that is passed by
    clients to the Media Server to start a new media session.

    A QMediaSessionRequest is not usually created directly. The Qt Extended Media
    API will create a session in response to a call to one of its functions to
    play media content. A Media Engine that provides a QMediaSessionBuilder of
    the same type as the request may be asked to fulfill the request.  Each
    request carries
        - a session identifier, valid for the lifetime of the session.
        - a domain, the media domain in which this session should be created, this
          effects management of the media content.
        - a type, which is used to identify the type of the request.

    \sa QMediaSessionBuilder
*/


/*!
    Construct an empty QMediaSessionRequest.
*/

QMediaSessionRequest::QMediaSessionRequest():
    d(new QMediaSessionRequestPrivate)
{
}


/*!
    Construct a QMediaSessionRequest in the Media \a domain and of \a type.
*/

QMediaSessionRequest::QMediaSessionRequest
(
 QString const& domain,
 QString const& type
):
    d(new QMediaSessionRequestPrivate)
{
    d->id = QUuid::createUuid();
    d->domain = domain;
    d->type = type;
}

/*!
    Construct QMediaSessionRequest taking a \a copy of another.
*/

QMediaSessionRequest::QMediaSessionRequest(QMediaSessionRequest const& copy):
    d(new QMediaSessionRequestPrivate)
{
    *d = *copy.d;
}

/*!
    Destruct a QMediaSessionRequest
*/

QMediaSessionRequest::~QMediaSessionRequest()
{
    delete d;
}

/*!
    Returns the session id of this request. The session id is automatically
    generated, and is valid for the life time of a session.
*/

QUuid const& QMediaSessionRequest::id() const
{
    return d->id;
}

/*!
    Returns the domain that this session will be allocated in.
*/

QString const& QMediaSessionRequest::domain() const
{
    return d->domain;
}

/*!
    Returns the type of the session.
*/

QString const& QMediaSessionRequest::type() const
{
    return d->type;
}

/*!
    Assignment operator for QMediaSessionRequest, taking its value from \a rhs.
*/

QMediaSessionRequest& QMediaSessionRequest::operator=(QMediaSessionRequest const& rhs)
{
    *d = *rhs.d;
    return *this;
}

/*!
    \fn QMediaSessionRequest& QMediaSessionRequest::operator<<(DataType const& data)

    The input-to operator for QMediaSessionRequest. This operator is used to
    add any extra arguments for the request. The \a data value is added to an
    internal queue of arguments.
*/

/*!
    \fn QMediaSessionRequest& QMediaSessionRequest::operator>>(DataType& data)

    The output-to operator for the QMediaSessionRequest. This operator is used to
    extract arguments from the request. The \a data value is removed from an
    internal queue of arguments.
*/

// {{{ Serialization
/*!
    \fn void QMediaSessionRequest::serialize(Stream &stream) const
    \internal
*/
template <typename Stream>
void QMediaSessionRequest::serialize(Stream &stream) const
{
    stream << d->id;
    stream << d->domain;
    stream << d->type;
    stream << d->requestData;
}

/*!
    \fn void QMediaSessionRequest::deserialize(Stream &stream)
    \internal
*/
template <typename Stream>
void QMediaSessionRequest::deserialize(Stream &stream)
{
    stream >> d->id;
    stream >> d->domain;
    stream >> d->type;
    stream >> d->requestData;
}
// }}}

//private:
void QMediaSessionRequest::enqueueRequestData(QVariant const& data)
{
    d->requestData.enqueue(data);
}

QVariant QMediaSessionRequest::dequeueRequestData()
{
    return d->requestData.dequeue();
}


Q_IMPLEMENT_USER_METATYPE(QMediaSessionRequest);
