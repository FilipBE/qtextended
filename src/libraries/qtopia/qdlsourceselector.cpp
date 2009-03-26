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
#include "qdlsourceselector_p.h"
#include "qdlclient_p.h"

// Qtopia includes
#include <QDSServices>
#include <QtopiaApplication>
#include <QMimeType>
#include <qtopialog.h>
#include <QtopiaItemDelegate>

// Qt includes
#include <QVBoxLayout>
#include <QListWidget>
#include <QDesktopWidget>

// ============================================================================
//
// QDLSourceSelectorPrivate
//
// ============================================================================

class QDLSourceSelectorPrivate
{
public:
    QDLSourceSelectorPrivate( const QMimeType& responseDataType )
    :   serviceList( 0 ),
        services( 0 ),
        itemToServiceIndex()
    {
        QStringList attributes;
        attributes.append( "QDL" ); // No tr
        attributes.append( "request" ); // No tr
        services = new QDSServices( QDLCLIENT_HINT_MIMETYPE,
                                    responseDataType.id(),
                                    attributes );
    }

    ~QDLSourceSelectorPrivate() { delete services; };

    QListWidget* serviceList;
    QDSServices* services;
    QMap<QListWidgetItem *, int> itemToServiceIndex;
};


// ============================================================================
//
// QDLSourceSelector
//
// ============================================================================

QDLSourceSelector::QDLSourceSelector( const QMimeType& responseDataType,
                                      QWidget *parent,
                                      Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    d = new QDLSourceSelectorPrivate( responseDataType );

    // Setup the window layout
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);
    d->serviceList = new QListWidget( this );
    d->serviceList->setFrameStyle(QFrame::NoFrame);
    d->serviceList->setItemDelegate(new QtopiaItemDelegate(d->serviceList));
    l->addWidget( d->serviceList );
    d->serviceList->setFocus();

    setWindowTitle( tr("Select Source") );

    // Add QDL services to the list widget
    const int len = d->services->count();
    for( int i=0; i < len; ++i ) {
        QString name = d->services->operator[](i).serviceName();
        qLog(DataLinking) << "QDLSourceSelector appending service" << name;
        QListWidgetItem *newItem
            = new QListWidgetItem( name );
        QString iconname = d->services->operator[]( i ).icon();
        if ( iconname.isEmpty() ) {
            iconname = "qdllink"; // No tr
        }
        QIcon is( QPixmap( ":icon/" + iconname ) );
        newItem->setIcon( is );
        d->serviceList->insertItem( i, newItem );
        d->itemToServiceIndex[ d->serviceList->item( i ) ] = i;
    }

    d->serviceList->sortItems();
    d->serviceList->setCurrentRow(0);

    // Connect the signals
    connect( d->serviceList,
             SIGNAL(itemActivated(QListWidgetItem*)),
             this,
             SLOT(accept()) );

    connect( d->serviceList,
             SIGNAL(itemPressed(QListWidgetItem*)),
             this,
             SLOT(accept()) );

    if ( len && !d->serviceList->selectedItems().count() == 0 )
        d->serviceList->setItemSelected( d->serviceList->item( 0 ), true );

    QDialog::setModal( true );
    QDialog::showMaximized();
    QtopiaApplication::setMenuLike( this, true );
}

QDLSourceSelector::~QDLSourceSelector()
{
    delete d;
}

QList<QDSServiceInfo> QDLSourceSelector::selected() const
{
    QList<QDSServiceInfo> selections;

    int numApps = d->serviceList->count();
    for( int i=0; i<numApps ; ++i ) {
        if( d->serviceList->isItemSelected( d->serviceList->item( i ) ) ) {
            int j = d->itemToServiceIndex[ d->serviceList->item( i ) ];
            selections.append( d->services->operator[]( j ) );
        }
    }

    return selections;
}

QSize QDLSourceSelector::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();
    return QSize(width(),
                 desktop->availableGeometry( desktop->screenNumber( this ) ).height() );
}

void QDLSourceSelector::accept()
{
    QDialog::accept();
    emit selected( selected() );
}
