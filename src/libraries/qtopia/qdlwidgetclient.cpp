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
#include "qdlclient.h"
#include "qdlwidgetclient_p.h"
#include "qdllink.h"

// Qtopia includes
#include <qtopialog.h>

// Qt includes
#include <QTextEdit>
#include <QTextBrowser>

// ============================================================================
//
// QDLWidgetClientPrivate
//
// ============================================================================

QDLWidgetClientPrivate::QDLWidgetClientPrivate( QWidget *widget )
:   mValid( false ),
    mWidget(),
    mWidgetType()
{
    setWidget( widget );
}

QDLWidgetClientPrivate::~QDLWidgetClientPrivate()
{
}

bool QDLWidgetClientPrivate::isValid() const
{
    return mValid;
}

void QDLWidgetClientPrivate::setText( const QString &text )
{
    QString wt = widgetType();
    if( wt == "QTextEdit" ) // No tr
        mWidget.te->setHtml( text );
    if( wt == "QTextBrowser" ) // No tr
        mWidget.tb->setHtml( text );
}

void QDLWidgetClientPrivate::verifyLinks( QDLClient* client )
{
    // Are we a valid widget?
    if ( client == 0 )
        return;
    if ( !isValid() )
        return;

    // Check for broken links
    {
        QList<int> ids = client->linkIds();
        foreach ( int id, ids ) {
            if ( client->link( id ).isBroken() )
                breakLink( client, id );
        }
    }

#ifdef QDL_STRICT_LINK_CHECKS
    // Disable this stuff due to performance, complete, but expensive tests

    // Check for links which don't exist in the widget text
    {
        QList<int> invalidLids;
        QList<int> ids = client->linkIds();
        QString t = text();
        foreach ( int id, ids ) {
            QString anchor;
            int pos = QDLPrivate::indexOfQDLAnchor( t,
                                                    client->objectName(),
                                                    id,
                                                    0,
                                                    anchor );
            if ( pos == -1 ) {
                invalidLids.append( id );
            }
        }

        // Remove the invalid links
        foreach ( int id, invalidLids )
            client->removeLink( id );
    }

    // Check for links in the text that don't exist in QDLClient and remove
    // them
    {
        QList<int> ids = client->linkIds();
        QString t = text();
        QString anchor;
        bool changed = false;
        int pos = QDLPrivate::indexOfQDLAnchor( t, 0, anchor );
        while ( pos != -1 ) {
            QString clientName;
            int linkId;
            if ( QDLPrivate::decodeHref( QDLPrivate::anchorToHref( anchor ),
                                         clientName,
                                         linkId ) )
            {
                if ( ( clientName == client->objectName() ) &&
                     !ids.contains( linkId ) )
                {
                    t.remove( pos, anchor.length() );
                    pos = QDLPrivate::indexOfQDLAnchor( t, pos, anchor );
                    changed = true;
                    continue;
                }
            }

            pos = QDLPrivate::indexOfQDLAnchor( t,
                                                pos + anchor.length(),
                                                anchor );
        }

        if (changed)
            setText( t );
    }
#endif
}

void QDLWidgetClientPrivate::breakLink( QDLClient* client, const int linkId )
{
    // Are we a valid widget?
    if ( client == 0 )
        return;
    if ( !isValid() )
        return;

    // Check that the link ID is valid
    if ( !client->validLinkId( linkId ) )
        return;

    // Break the link
    client->breakLink( linkId, true );

    // Adjust the link text in the widget
    QString t = text();

    QString anchor;
    int prevStopPos = -1;
    int pos = QDLPrivate::indexOfQDLAnchor( t,
                                            client->objectName(),
                                            linkId,
                                            0,
                                            anchor );
    while ( pos != -1 ) {
        // Remove the old link
        t.remove( pos, anchor.length() );

        // Insert the new link, but only if there is no link directly before
        // this one. This handles when QTextEdit splits a link with an image
        if ( pos != prevStopPos ) {
            QString anchorText = client->linkAnchorText( linkId );
            t.insert( pos, anchorText );
            prevStopPos = pos + anchorText.length();
        }
        else {
            prevStopPos = -1;
        }

        // find the next anchor
        pos = QDLPrivate::indexOfQDLAnchor( t,
                                            client->objectName(),
                                            linkId,
                                            pos + anchor.length(),
                                            anchor );
    }

    // Set the new widget text
    setText( t );
}

void QDLWidgetClientPrivate::setWidget( QWidget *widget )
{
    mValid = false;

    if( widget == 0 ) {
        setWidgetType( QString() );
        mWidget.te = 0;
        mWidget.tb = 0;
        mValid = false;
    } else if( widget->inherits( "QTextBrowser" ) ) { // No tr
        mWidget.tb = qobject_cast<QTextBrowser*>( widget );
        setWidgetType( "QTextBrowser" ); // No tr
        mValid = true;
    } else if( widget->inherits( "QTextEdit" ) ) { // No tr
        mWidget.te = qobject_cast<QTextEdit*>( widget );
        setWidgetType( "QTextEdit" ); // No tr
        mValid = true;
    }
}

QWidget* QDLWidgetClientPrivate::widget() const
{
    QString wt = widgetType();

    if( wt == "QTextEdit" ) // No tr
        return mWidget.te;
    else if( wt == "QTextBrowser" ) // No tr
        return mWidget.tb;
    else
        return 0;
}

void QDLWidgetClientPrivate::setWidgetType( const QString& type )
{
    mWidgetType = type;
}

QString QDLWidgetClientPrivate::widgetType() const
{
    return mWidgetType;
}

void QDLWidgetClientPrivate::insertText( const QString& text )
{
    QString wt = widgetType();

    if ( wt == "QTextEdit" ) // No tr
        mWidget.te->insertHtml( text );
    //else if ( wt == "QTextBrowser" )
        // don't allow text to be inserted into a QTextBrowser, should use setText()
}

QString QDLWidgetClientPrivate::text() const
{
    QString wt = widgetType();

    if( wt == "QTextEdit" ) // No tr
        return mWidget.te->toHtml();
    else if( wt == "QTextBrowser" ) // No tr
        return mWidget.tb->toHtml();
    else
        return QString();
}

QString QDLWidgetClientPrivate::determineHint() const
{
    QString wt = widgetType();
    if ( wt == "QTextEdit" ) // No tr
        return mWidget.te->textCursor().selectedText();
    if ( wt == "QTextBrowser" ) // No tr
        return mWidget.tb->textCursor().selectedText();
    else
        return QString();
}




