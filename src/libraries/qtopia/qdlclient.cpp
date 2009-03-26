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
#include "qdlclient_p.h"
#include "qdllink.h"
#include "qdlsourceselector_p.h"

// Qtopia includes
#include <QUniqueId>
#include <QDSAction>
#include <QDSData>
#include <QMimeType>
#include <qtopialog.h>

// Qt includes
#include <QByteArray>

// ============================================================================
//
// Functions
//
// ============================================================================

static QString encodeAhref( const QString &ahref )
{
    QString encAhref = ahref;
    return encAhref.replace( QRegExp(":"), "%3A" );
}

// ============================================================================
//
// QDLClientPrivate
//
// ============================================================================

class QDLClientPrivate
{
public:
    int nextLinkId();

    QString mHint;
    QMap<int, QUniqueId> mLinkKeys;
};

int QDLClientPrivate::nextLinkId()
{
    uint numLids = mLinkKeys.count();

    for( uint i = 1 ; i <= numLids ; ++i )
        if( !( mLinkKeys.contains( i ) ) )
            return i;

    return numLids+1;
}

// ============================================================================
//
// QDLClient
//
// ============================================================================

/*!
    \class QDLClient
    \inpublicgroup QtBaseModule

    \brief The QDLClient class manages a set of QDLLinks for a client object.

    The QDLClient class stores a set of QDLLinks for a client object. QDLClient
    manages the stored QDLLinks, allows links to be requested from QDL data
    sources and to activate a link on a QDL source.

    The links managed by the client object can be saved using
    QDLClient::saveLinks() and can be restored using QDLClient::loadLinks().
    Alternatively if the client object belongs to a group,
    QDLClients::saveLinks() and QDL::loadLinks() should be used for convenience.

    This class operates independently of any other object. The subclasses
    QDLBrowserClient and QDLEditClient extends this class to operate on a widget
    and its text dependently.

    \sa QDLBrowserClient, QDLEditClient

    \ingroup ipc
*/

/*!
    Constructs a QDLClient, \a parent is passed on to QObject to establish the
    parent child relationship. The QDLClient is identified by \a name, which
    should be unique within a group of QDLClients. \a name should only contain
    alpha-numeric characters, underscores and spaces.
*/
QDLClient::QDLClient( QObject *parent, const QString& name )
:   QObject( parent ),
    d( 0 )
{
    QObject::setObjectName( encodeAhref( name ) );
    d = new QDLClientPrivate();
}

/*!
    Destroys a QDL Client.
*/
QDLClient::~QDLClient()
{
    delete d;
}

/*!
    Saves the stored links to \a stream.

    \sa loadLinks()
*/
void QDLClient::saveLinks( QDataStream& stream ) const
{
    stream << d->mLinkKeys;
}

/*!
    Retrieves a copy of the stored QDLLink identified by \a linkId. If the
    copy of the QDLLink is modified, QDLClient::setLink() should be used to
    update the stored QDLLink.

    If \a linkId is invalid a null QDLLink will be returned.

    \sa setLink(), linkIds()
*/
QDLLink QDLClient::link( const int linkId ) const
{
   // Check that we have a valid linkId
   if ( validLinkId( linkId ) )
        return QDLLink( QDSData( d->mLinkKeys[linkId] ) );
   else
        return QDLLink();
}

/*!
    Returns a list of all the stored Link IDs.

    \sa link()
*/
QList<int> QDLClient::linkIds() const
{
    return d->mLinkKeys.keys();
}

/*!
    Returns the hint used when requesting QDLLinks

    \sa setHint()
*/
QString QDLClient::hint() const
{
    return d->mHint;
}

/*!
    Returns the rich-text anchor for the QDLLink identified by \a linkId.
    The anchor will not contain the QDLLink icon if \a noIcon is true.
*/
QString QDLClient::linkAnchorText( const int linkId, const bool noIcon ) const
{
    // Check that we have a valid linkId
    if ( !validLinkId( linkId ) )
        return QString();

    return QDLPrivate::linkAnchorText( objectName(),
                                       linkId,
                                       link( linkId ),
                                       noIcon );
}

/*!
    Returns true if \a linkId identifies a stored QDLLink; otherwise returns false.

    \sa linkIds()
*/
bool QDLClient::validLinkId( const int linkId ) const
{
    return d->mLinkKeys.contains( linkId );
}

/*!
    Sets the \a hint used when requesting QDLLinks

    \sa hint()
*/
void QDLClient::setHint( const QString& hint )
{
    d->mHint = hint;
}

/*!
    Loads the links in \a stream into the client object. \a stream is
    generated by QDLClient::saveLinks().

    \sa saveLinks()
*/
void QDLClient::loadLinks( QDataStream& stream )
{
    d->mLinkKeys.clear();
    stream >> d->mLinkKeys;
}

/*!
    Adds the link stored in \a link to the client object.
    The link Id is returned if the link is added correctly,
    otherwise 0 is returned.

    QDL sources create \a link, which is returned during
    QDLEditClient::requestLinks(). Therefore in normal usage this method isn't
    called directly by a client.

    \sa requestLinks(), setLink(), removeLink()
*/
int QDLClient::addLink( QDSData& link )
{
    // Increment the reference to the data in the QDSDataStore and
    // add the key to the map
    int linkId = d->nextLinkId();
    d->mLinkKeys[ linkId ] = link.store();

    return linkId;
}

/*!
    Updates the link stored identified by \a linkId with \a link.

    \sa addLink(), removeLink()
*/
void QDLClient::setLink( const int linkId, const QDLLink &link )
{
    // Check that we have a valid linkId
    if ( !validLinkId( linkId ) )
        return;

    // Grab the QDSData object for the link
    QDSData linkData( d->mLinkKeys[linkId] );
    if ( linkData.type().id() != QDLLink::mimeType().id() ) {
        qWarning( "QDLClient::setLink() - QDSData was of invalid type" );
        return;
    }

    // Modify the QDS data object
    linkData.modify( link.toQDSData().data() );
}

/*!
    Removes the link stored identified by \a linkId, and updates the
    anchor text for the link in the parent widget's text.

    \sa addLink(), setLink()
*/
void QDLClient::removeLink( const int linkId )
{
    // Check that we have a valid linkId
    if ( !validLinkId( linkId ) )
        return;

    // Remove the reference for the link from the QDSDataStore
    QDSData( d->mLinkKeys[linkId] ).remove();

    // Remove the link keys map
    d->mLinkKeys.remove( linkId );
}

/*!
    Sets the broken state of the stored QDLLink identified by \a linkId to
    \a broken, which is used to indicate that the data item at the source
    is no longer available.

    \sa removeLink()
*/
void QDLClient::breakLink( const int linkId, const bool broken )
{
    // Check that we have a valid linkId
    if ( !validLinkId( linkId ) )
        return;

    // Get the link from the QDSDataStore and check that we want to change
    // it's broken state
    QDSData linkData = QDSData( d->mLinkKeys[linkId] );
    QDLLink link( linkData );
    if ( link.isBroken() == broken )
        return;

    // Break/unbreak the link
    link.setBroken( broken );

    // Update the link data in the QDSDataStore
    linkData.modify( link.toQDSData().data() );
}


/*!
    Removes all links from the client object.

    \sa requestLinks()
*/
void QDLClient::clear()
{
    QList<int> ids = linkIds();
    foreach ( int id, ids )
        removeLink( id );
}

/*!
    Requests QDL links from a source. The user selects the
    desired source from a list of available QDL sources, the list
    is a modal dialog connected to \a parent.

    \sa clear()
*/
void QDLClient::requestLinks( QWidget* parent )
{
    // Use a QDLSourceSelector to get the user to select a QDL source
    QDLSourceSelector *s
        = new QDLSourceSelector( QDLLink::listMimeType(), parent );

    if ( s->exec() == QDialog::Accepted )
    {
        QList<QDSServiceInfo> qdlServices = s->selected();
        QList<QDSServiceInfo>::ConstIterator cit;
        for( cit = qdlServices.begin(); cit != qdlServices.end() ; ++cit )
            requestLinks( *cit );
    }

    // Cleanup
    delete s;
}

/*!
    Requests QDL links from the QDL source described in \a qdlService.
    This method can be used when the QDL source is known.

    \sa clear()
*/
void QDLClient::requestLinks( const QDSServiceInfo& qdlService )
{
    // Check that the service has the right types
    if ( !qdlService.supportsRequestDataType( QMimeType(QDLCLIENT_HINT_MIMETYPE) ) ) {
        qWarning( "QDLClient::requestLinks() - "\
                  " QDS service has invalid request data type" );
        return;
    }

    if ( !qdlService.supportsResponseDataType( QDLLink::listMimeType() ) ) {
        qWarning( "QDLClient::requestLinks() -"\
                  " QDS service has invalid response data type" );
        return;
    }

    // Create the hint string data object
    QByteArray stringDataArray;
    {
        QDataStream ds( &stringDataArray, QIODevice::WriteOnly );
        ds << hint();
    }
    QDSData stringData( stringDataArray, QMimeType(QDLCLIENT_HINT_MIMETYPE) );

    // Create the action and make the request
    QDSAction action( qdlService );
    int resCode = action.exec( stringData );
    if ( resCode == QDSAction::Complete ) {
        qLog(DataLinking) << "QDLClient::requestLinks() - "
                          << "QDSAction responded with no data";
        return;
    } else if ( resCode == QDSAction::Error ) {
        qLog(DataLinking) << "QDLClient::requestLinks() - "
                          << "QDSAction responded with an error:"
                          << action.errorMessage().toLatin1().constData();
        return;
    }

    // Pullout the link data objects
    QByteArray linksData = action.responseData().data();
    QList<QDSData> links;
    QDataStream ds( &linksData, QIODevice::ReadOnly );
    ds >> links;

    // Add the new links to this client
    foreach ( QDSData linkData, links ) {
        addLink( linkData );
    }
}

/*!
    Activates the stored QDLLink identified by \a linkId on the QDL data
    source.
*/
void QDLClient::activateLink( const int linkId )
{
    // Check that we have a valid linkId
    if ( !validLinkId( linkId ) ) {
        qLog(DataLinking) << "QDLClient::activateLink - Link with ID"
                          << linkId
                          << "does not exist";
        return;
    }

    // Activate the link
    link( linkId ).activate();
}

/*!
    Verifies the correctness of the links stored by the client object. This
    method determines if QDLLinks are broken, and ensures that all
    stored links have are properly configured.

    \sa addLink(), setLink(), removeLink()
*/
void QDLClient::verifyLinks()
{
    // Default implementation, do nothing
}
