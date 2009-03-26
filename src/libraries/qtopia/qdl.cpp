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

// Local includes
#include "qdl.h"
#include "qdl_p.h"
#include "qdlclient.h"

// Qtopia includes
#include <qtopialog.h>

// Qt
#include <QObject>

// ============================================================================
//
// QDLPrivate namespace
//
// ============================================================================

/*!
    \internal
*/
QString QDLPrivate::linkAnchorText( const QString& clientName,
                                    const int linkId,
                                    const QDLLink& link,
                                    const bool noIcon )
{
    QString ahref = "<a href=\"";
    ahref += QDL::ANCHOR_HREF_PREFIX;
    ahref += "%1:%2\">%3%4</a>";

    // bug work around in QTextBrowser, which doesn't allow links which
    // are butted up against each other to be selected individually with
    // the keypad. See Bugzilla #5039 for details.
    // Works by splitting the text into seperate QTextFragments.
    ahref += "&nbsp;";

    // Add client name, link ID and description
    if ( link.isBroken() ) {
        ahref = ahref.arg( clientName ).arg( linkId );
        if ( !link.description().isEmpty() )
            ahref = ahref.arg( link.description() +QLatin1String(" ") +  QObject::tr( "(Broken Link)" ) );
        else
            ahref = ahref.arg( QObject::tr( "(Broken Link)" ) );
    } else {
        ahref = ahref.arg( clientName ).arg( linkId );
        ahref = ahref.arg( link.description() );
        }

    // Add icon
    QString iFileName = link.icon();
    if( iFileName.isEmpty() )
        iFileName = "pics/icons/22x22/qdllink"; // No tr

    if ( noIcon ) {
        ahref = ahref.arg( QString() );
    } else {
        QString img = "<img width=\"12\" height=\"12\" src=\"%1\">"; // No tr
        img = img.arg( iFileName );
        ahref = ahref.arg( img );
    }

    return ahref;
}

/*!
    \internal
*/
int QDLPrivate::indexOfQDLAnchor( const QString& text,
                                  const int startPos,
                                  QString& anchor )
{
    // Find the start of the first QDL href
    int startQDLHref = text.indexOf( QDL::ANCHOR_HREF_PREFIX, startPos );
    if ( startQDLHref == -1 ) {
        anchor = QString();
        return -1;
    }

    // Find the anchor start and stop tags
    QString anchorStart = "<a";
    QString anchorStop = "/a>";
    int anchorStartPos = text.lastIndexOf( anchorStart, startQDLHref );
    int anchorStopPos = text.indexOf( anchorStop, startQDLHref);

    if ( ( anchorStartPos == -1 ) ||
         ( anchorStopPos == -1 ) ||
         ( anchorStopPos <= anchorStartPos ) )
        {
        anchor = QString();
        return -1;
        }

    // Pull out the anchor text
    anchor = text.mid( anchorStartPos,
                       anchorStopPos - anchorStartPos + anchorStop.length() );
    return anchorStartPos;
}

/*!
    \internal
*/
int QDLPrivate::indexOfQDLAnchor( const QString& text,
                                  const QString& clientName,
                                  const int linkId,
                                  const int startPos,
                                  QString& anchor )
{
    // Build up search pattern
    QString pattern = QDL::ANCHOR_HREF_PREFIX;
    pattern += clientName;
    pattern += ":";
    pattern += QString::number( linkId );

    // Find the start of the first QDL href
    int startQDLHref = text.indexOf( pattern, startPos );
    if ( startQDLHref == -1 ) {
        anchor = QString();
        return -1;
    }

    // Find the anchor start and stop tags
    QString anchorStart = "<a";
    QString anchorStop = "/a>";
    int anchorStartPos = text.lastIndexOf( anchorStart, startQDLHref );
    int anchorStopPos = text.indexOf( anchorStop, startQDLHref);

    if ( ( anchorStartPos == -1 ) ||
         ( anchorStopPos == -1 ) ||
         ( anchorStopPos <= anchorStartPos ) )
    {
        anchor = QString();
        return -1;
    }

    // Pull out the anchor text
    anchor = text.mid( anchorStartPos,
                       anchorStopPos - anchorStartPos + anchorStop.length() );
    return anchorStartPos;
}

/*!
    \internal
*/
QString QDLPrivate::anchorToHref( const QString& anchor )
{
    // Find the start of the href
    int hrefStartPos = anchor.indexOf( QDL::ANCHOR_HREF_PREFIX );
    if ( hrefStartPos == -1 )
        return QString();

    // Find the end of the href
    QString hrefStop = "\">";
    int hrefStopPos = anchor.indexOf( hrefStop, hrefStartPos );
    if ( ( hrefStartPos == -1 ) ||
         ( hrefStopPos == -1 ) ||
         ( hrefStopPos <= hrefStartPos ) )
        return QString();

    // Pull out the href
    return anchor.mid( hrefStartPos, hrefStopPos - hrefStartPos );
}

/*!
    \internal
*/
bool QDLPrivate::decodeHref( const QString& href,
                             QString& clientName,
                             int& linkId )
{
    if ( !href.startsWith( QDL::ANCHOR_HREF_PREFIX ) ) {
        return false;
    }

    QString text = href.mid( QDL::ANCHOR_HREF_PREFIX.length() );
    QStringList data;
    data = text.split( ':' );
    if ( data.count() != 2 ) {
        qLog(DataLinking) << "QDL::decodeHref called with incorrectly "
                          << "formatted href";
        return false;
    }

    clientName = data[0];
    linkId = data[1].toInt();
    if ( clientName.isEmpty() || ( linkId == 0 ) ) {
        clientName = QString();
        linkId = 0;
        qLog(DataLinking) << "QDL::decodeHref detected an empty/broken link.";
        return false;
    }

    return true;
}

// ============================================================================
//
// QDL namespace
//
// ============================================================================

/*!
    \class QDL
    \inpublicgroup QtBaseModule

    \brief The QDL class provides general utility functions for use
        with Qt Extended Data Linking.

    Qt Extended Data Linking (QDL) allows a client to link to data in a source.
    A client may be a stand-alone object (QDLClient) or attached to a
    widget (QDLBrowserClient or QDLEditClient).
    A source can be any application which acts as a container for useful
    data and provides the QDL Service.

    This class provides useful functions for loading and saving links
    and dealing with general QDL usage.

    First availability: Qtopia 2.0

  \ingroup ipc
*/


const QString QDL::ANCHOR_HREF_PREFIX = "QDL://"; // No tr

const QString QDL::CLIENT_DATA_KEY = "qdl-private-client-data"; // No tr

const QString QDL::SOURCE_DATA_KEY = "qdl-private-source-data"; // No tr

/*!
    Returns all QDLClient objects that are children of \a parent, recursively.
    This is useful for automatically loading links into multiple clients, for
    example:

    \code
    QDL::loadLinks( rec, QDL::clients( this ) );
    \endcode
*/
QList<QDLClient *> QDL::clients( QObject *parent )
{
    QList<QDLClient *> cs;

    if ( !parent )
        return cs;

    QObjectList l = parent->children();
    for ( QObjectList::Iterator it = l.begin() ; it != l.end() ; ++it ) {
        if ( (*it)->inherits( "QDLClient" ) ) { // No tr
            cs.append( (QDLClient *)*it );
    }

        cs += QDL::clients( *it );
    }

    return cs;
}

/*!
    Loads the links stored in \a str into the clients in \a clientList.
    \a str is the base64 encoded binary data of the links created by
    QDL::saveLinks().
*/
void QDL::loadLinks( const QString &str, QList<QDLClient *> clientList )
{
    // extract all links out of the string
    if( str.isEmpty() )
        return;

    QByteArray data = QByteArray::fromBase64( str.toAscii() );
    QDataStream stream( data );
    stream >> clientList;
}

/*!
    Releases all the QDLLinks stored in \a str generated by QDL::saveLinks()
*/
void QDL::releaseLinks( const QString& str )
{
    QByteArray data = QByteArray::fromBase64( str.toAscii() );
    QDataStream stream( data );

    int numClients = 0;
    stream >> numClients;
    for ( int i=0; i<numClients; ++i ) {

        // Unpack the stream and load the links into a new QDLClient
        QString clientName;
        stream >> clientName;
        QDLClient client( 0, clientName );
        client.loadLinks( stream );

        // Clear the client, releasing all the links
        client.clear();
    }
}

/*!
  Saves the links from the clients contained in \a clientList into \a str.
  \a str will be the base64 encoded binary link data.
*/
void QDL::saveLinks( QString &str, QList<QDLClient *> clientList )
{
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << clientList;
    str = QString( data.toBase64() );
}

/*!
    Activates the QDL link specified by \a href, which should be in the form
    \c{QDL://<clientName>:<linkId>}. The \a clientList should contain the QDL
    client which holds the link, and is identified by \c clientName.
*/
void QDL::activateLink( const QString &href, const QList<QDLClient *> &clientList )
{
    // Grab the clientname and linkId from the href
    QString clientName;
    int linkId;
    if ( !QDLPrivate::decodeHref( href, clientName, linkId ) )
        return;

    // Find the client
    QDLClient *c = 0;
    for ( QList<QDLClient *>::ConstIterator it = clientList.begin();
          it != clientList.end();
          ++it ) {
        if( QString((*it)->objectName()) == clientName )
    {
            c = *it;
            break;
        }
    }

    if ( !c ) {
        qLog(DataLinking) << "QDL::activateLink - Can't activate link"
                          << "for unknown client"
                          << clientName.toLatin1().constData();
        return;
    }

    // Activate the link in the QDLClient
    c->activateLink( linkId );
}

/*!
   \relates QDL
   Reads \a clientList from the \a stream
*/
QDataStream &operator>>( QDataStream &stream, QList<QDLClient *>& clientList )
{
    int numClients = 0;
    stream >> numClients;
    for ( int i=0; i<numClients; ++i ) {

        // Unpack the stream
        QString clientName;
        stream >> clientName;
        if ( clientName.isEmpty() ) {
            qLog(DataLinking) << "Trying to load incorrectly formatted "
                              << "QDLLinks stream";
            return stream;
            }

        // Find the QDL client in the list
        QDLClient *c = 0;
        for( QList<QDLClient *>::Iterator cit = clientList.begin();
             cit != clientList.end();
             ++cit ) {
            if( (*cit)->objectName() == clientName ) {
                c = *cit;
                break;
    }
            }

        if ( c == 0 ) {
            // Stream the links into a empty QDLClient so we can ignore this error and
            // continue to load links from other clients
            QDLClient empty( 0, clientName );
            empty.loadLinks( stream );
        } else {
            // Load the links into the client
            c->loadLinks( stream );
        }
    }
    return stream;
}

/*!
   \relates QDL
   Writes \a clientList to \a stream
*/
QDataStream &operator<<( QDataStream &stream, const QList<QDLClient *> &clientList )
{
    stream << clientList.count();
    for( QList<QDLClient *>::ConstIterator cit = clientList.begin();
         cit != clientList.end();
         ++cit )
    {
        // verify links for this client
        (*cit)->verifyLinks();
        QString clientName = (*cit)->objectName();
        if ( !clientName.isEmpty() ) {
            stream << clientName;
            (*cit)->saveLinks( stream );
    }
    }

    return stream;
}
