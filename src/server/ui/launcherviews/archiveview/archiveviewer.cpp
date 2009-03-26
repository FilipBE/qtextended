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

#include "archiveviewer.h"
#include <qdocumentproperties.h>
#include <qtopiaapplication.h>
#include <QSoftMenuBar>
#include <QtDebug>
#include <QAction>
#include <QMenu>
#include <QHideEvent>
#include <QShowEvent>
#include "applicationlauncher.h"
#include "uifactory.h"

/*!
    \class ArchiveViewer
    \inpublicgroup QtDrmModule
    \brief The ArchiveViewer widget displays the contents of an archive file format.
  
    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Constructs a new archive viewer with parent \a parent and flags \a flags.
*/
ArchiveViewer::ArchiveViewer( QWidget* parent, Qt::WFlags flags )
    : LauncherView( parent, flags )
    , propDlg( 0 )
{
    setObjectName( "archives" );

    QMenu *softMenu = QSoftMenuBar::menuFor( this );

    setFocusPolicy( Qt::StrongFocus );

    propertiesAction = new QAction( QIcon(":icon/info"), tr("Properties..."), this );

    QObject::connect( propertiesAction, SIGNAL(triggered()), this, SLOT(showProperties()) );
    QObject::connect( this, SIGNAL(clicked(QContent)), this, SLOT(executeContent(QContent)) );

    softMenu->addAction( propertiesAction );
}

/*!
    Displays all files in the directory \a document.
*/
void ArchiveViewer::setDocument( const QString &document )
{
    QContentSet::scan( document );

    QContent content( document );

    setWindowTitle( content.name() );
    setFilter( QContentFilter( QContentFilter::Location, document ) );
}

void ArchiveViewer::executeContent( const QContent &content )
{
    if( content.type() == "application/vnd.oma.drm.dcf" )
    {
        filterStack.push( m_contentSet->filter() );

        setFilter( QContentFilter( QContentFilter::Location, content.fileName() ) );
    }
    else if( !content.executableName().isEmpty() )
    {
        content.execute();
    }
    else
    {
        if( !warningBox )
        {
            warningBox = QAbstractMessageBox::messageBox(
                    this,
                    tr("No application"),
                    tr("<qt>No application is defined for this document.</qt>"),
                    QAbstractMessageBox::Information );
            warningBox->setAttribute(Qt::WA_DeleteOnClose); // It's a QPointer<> so safe.
        }

        QtopiaApplication::showDialog( warningBox );
    }
}

void ArchiveViewer::showProperties()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        propLnk = doc;
        if (propDlg)
            delete propDlg;
        propDlg = new QDocumentPropertiesDialog(propLnk, this);
        propDlg->setObjectName("document-properties");
        QtopiaApplication::showDialog(propDlg);
        propDlg->setWindowState(propDlg->windowState() | Qt::WindowMaximized);
    }
}

/*!
    \reimp
 */
void ArchiveViewer::showEvent( QShowEvent *event )
{
    if( !m_contentSet->filter().isValid() )
        setFilter(
                QContentFilter( QContent::Document ) &
                QContentFilter( QContentFilter::MimeType, "application/vnd.oma.drm.dcf" ) );

    LauncherView::showEvent( event );
}

/*!
    \reimp
*/
void ArchiveViewer::keyPressEvent( QKeyEvent *event )
{
    if( event->key() == Qt::Key_Back && !filterStack.isEmpty() )
    {
        setFilter( filterStack.top() );

        filterStack.pop();

        event->accept();
    }
    else
        LauncherView::keyPressEvent( event );
}

/*!
    \reimp
*/
void ArchiveViewer::hideEvent( QHideEvent *event )
{
    setFilter( QContentFilter() );

    LauncherView::hideEvent( event );
}

static QWidget *archives()
{
    return new ArchiveViewer;
}
QTOPIA_SIMPLE_BUILTIN(archives,archives);
UIFACTORY_REGISTER_WIDGET(ArchiveViewer);
