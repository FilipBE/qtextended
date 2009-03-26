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
#include "qdl_p.h"
#include "qdleditclient.h"
#include "qdllink.h"
#include "qdlwidgetclient_p.h"

// Qtopia includes
#include <QSoftMenuBar>

// Qt includes
#include <QTextEdit>
#include <qtopialog.h>

// System includes
#include <sys/types.h>
#include <unistd.h>

// ============================================================================
//
// QDLEditClient
//
// ============================================================================

/*!
    \class QDLEditClient
    \inpublicgroup QtBaseModule

    \brief The QDLEditClient class is used to add QDLLinks to rich-text documents.

    QDLEditClient is a subclass of QDLClient which manages the QDLLinks in
    collaboration with the rich-text in QTextEdit. This allows the anchor text
    for QDLLinks to updated as links are added, set, or removed from the
    client object.

    \sa QDLClient, QDLBrowserClient

    \ingroup ipc
*/

/*!
    Constructs a QDLEditClient with \a edit as the parent widget. The QDLClient
    is identified by \a name, which should be unique within a group of
    QDLClients. \a name should only contain alpha-numeric characters,
    underscores and spaces.

    \sa isValid()
*/
QDLEditClient::QDLEditClient( QTextEdit *edit, const QString& name )
:   QDLClient( edit, name ),
    d( 0 )
{
    d = new QDLWidgetClientPrivate( edit );
}

/*!
    Destroys a QDL Edit Client.
*/
QDLEditClient::~QDLEditClient()
{
    delete d;
}

/*!
    Adds an "Insert Link" action item to \a context which connects to
    QDLEditClient::requestLinks().

    \sa requestLinks()
*/
QMenu* QDLEditClient::setupStandardContextMenu( QMenu *context )
{
    // Are we a valid widget?
    if( !isValid() ) {
        qLog(DataLinking) << "BUG : trying to setup a context "
                          << "menu on a null widget client";
        return 0;
    }

    QWidget *w = d->widget();
    if ( !context )
        context = QSoftMenuBar::menuFor( w, QSoftMenuBar::AnyFocus );

    QAction *insertLinkAction = new QAction( QIcon( ":icon/qdllink" ),
                                             tr( "Insert Link" ),
                                             w );

    QObject::connect( insertLinkAction,
                      SIGNAL(triggered(bool)),
                      this,
                      SLOT(requestLinks()) );

    context->addAction( insertLinkAction );

    QSoftMenuBar::addMenuTo( w, context, QSoftMenuBar::AnyFocus );
    return context;
}

/*!
    Returns true if the client object is valid; otherwise returns false. Calls to methods on an invalid
    client will fail.
*/
bool QDLEditClient::isValid() const
{
    return d->isValid();
}

/*!
    Returns the hint used when requesting links.

    \sa QDLClient::setHint()
*/
QString QDLEditClient::hint() const
{
    // Are we a valid widget?
    if( !isValid() ) {
        return QString();
    }

    // If the user has manually set a hint use that
    // otherwise determine a hint(selection)
    QString h = QDLClient::hint();
    if( !h.isEmpty() )
        return h;

    return d->determineHint();
}

/*!
    \reimp
*/
int QDLEditClient::addLink( QDSData& link )
{
    // Are we a valid widget?
    if( !isValid() )
        return 0;

    // Use the QDLClient to add the link
    int linkId = QDLClient::addLink( link );

    // Now add the link text to the widget
    d->insertText( QDLClient::linkAnchorText( linkId ) );

    // Return the link ID
    return linkId;
}

/*!
    \reimp
*/
void QDLEditClient::setLink( const int linkId, const QDLLink& link )
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    // Check that the link ID is valid
    if ( !QDLClient::validLinkId( linkId ) )
        return;

    QDLClient::setLink( linkId, link );
    QString t = d->text();

    QString anchor;
    int pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            0,
                                            anchor );
    int prevStopPos = -1;
    while ( pos != -1 ) {
        // Remove the old link
        t.remove( pos, anchor.length() );

        // Insert the new link, but only if there is no link directly before
        // this one. This handles when QTextEdit splits a link with an image
        QString anchorText = QDLClient::linkAnchorText( linkId );
        if ( pos != prevStopPos ) {
            t.insert( pos, anchorText );
            prevStopPos = pos + anchorText.length();
        }
        else {
            prevStopPos = -1;
        }

        // find the next anchor
        pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            pos + anchorText.length(),
                                            anchor );
    }

    // Set the new widget text
    d->setText( t );
}

/*!
    \reimp
*/
void QDLEditClient::removeLink( const int linkId )
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    // Check that the link ID is valid
    if ( !QDLClient::validLinkId( linkId ) )
        return;

    QString t = d->text();

    QString anchor;
    int pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            0,
                                            anchor );
    while ( pos != -1 ) {
        // Remove the old link
        t.remove( pos, anchor.length() );

        // find the next anchor
        pos = QDLPrivate::indexOfQDLAnchor( t,
                                            objectName(),
                                            linkId,
                                            pos,
                                            anchor );
    }

    // Set the new widget text and remove the old link from the QDLClient
    d->setText( t );
    QDLClient::removeLink( linkId );
}

/*!
    Requests QDLLinks from a selected QDL source. The user selects the
    source from a list of available QDL sources. Returned links
    from the source are automatically added to the client using
    QDLEditClient::addLink().

    \sa addLink()
*/
void QDLEditClient::requestLinks()
{
    // Are we a valid widget?
    if( !isValid() )
        return;

    QDLClient::requestLinks( d->widget() );
}

/*!
    \reimp
*/
void QDLEditClient::verifyLinks()
{
    d->verifyLinks( this );
}

