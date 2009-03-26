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

#include "qmediasessionbuilder.h"


/*!
    \class QMediaSessionBuilder
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaSessionBuilder class is a base class to be used by Media Engines
            which need to create sessions within the Media Server.

    A Media Engine should make available at least one QMediaSessionBuilder to be used
    in the construction of media sessions.

    QMediaSessionBuilder is not instantiated directly, it should be inherited
    and its methods overridden to provide the required functionality.

    \code
    Example
    {
        class UrlSessionBuilder : public QMediaSessionBuilder
        {
        public:
            UriSessionBuilder()
            {
                // create m_attributes, perhaps from args
            }

            ~UriSessionBuilder() {}

            QString type() { return "com.trolltech.qtopia.uri"; }
            Attributes const& attributes() { return m_attributes; }

            QMediaServerSession* createSession(QMediaSessionRequest request)
            {
                QUrl    url;
                request >> url;
                // do something
                return session;
            }

            void destroySession(QMediaServerSession* session)
            {
                delete session;
            }

        private:
            Attributes      m_attributes;
        };
    }
    \endcode

    \sa QMediaEngine
    \sa QMediaSessionRequest
    \sa QMediaServerSession

    \ingroup multimedia
*/

/*!
    \typedef QMediaSessionBuilder::Attributes

    A QMap of QString to QVariant. Attributes are generic name value pairs for
    use in communicating static information about the builder.
*/

/*!
    Destruct a QMediaSessionBuilder.
*/

QMediaSessionBuilder::~QMediaSessionBuilder()
{
}

/*!
    \fn QString QMediaSessionBuilder::type() const;

    Returns a string used as an identifier for the type of builder. The string
    should be of the form reverse-dns.local-type. Example:
    com.trolltech.qtopia.uri.
*/

/*!
    \fn Attributes const& QMediaSessionBuilder::attributes() const;

    Returns a QMap of Attribute Name and Value pairs. This may be used for
    communication of some builder type information to a builder type manager,
    or for custom builder types may just be information that is useful to be
    presented into the Qt Extended Value-space.

    \sa QValueSpaceItem
*/

/*!
    \fn QMediaServerSession* QMediaSessionBuilder::createSession(QMediaSessionRequest sessionRequest);

    This function should create a QMediaServerSession, if able, based upon the
    information in the session request. This function will be called if the
    type of the QMediaSessionRequest \a sessionRequest matches the type of the
    builder. Additionally, if the builder is one of the common builder types it
    will only be called if it matches extra criteria, as defined by the builder
    type, for example, the mime type of the content.

    \sa QMediaSessionRequest
    \sa QMediaServerSession
*/

/*!
    \fn void QMediaSessionBuilder::destroySession(QMediaServerSession* serverSession);

    This function is called when the session \a serverSession is no longer
    necessary. The builder will only be asked to destroy sessions that it
    created. The builder must destroy the session at this time.
*/

/*!
    \typedef QMediaSessionBuilderList

    A QList of QMediaSessionBuilder. A convenience for dealing with standard
    collections of builders.
*/

