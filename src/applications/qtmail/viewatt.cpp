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


#include "viewatt.h"
#include <qcontent.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qtablewidget.h>
#include <qheaderview.h>
#include <qevent.h>
#include <QDebug>
#include <QMailMessage>
#include <qtopiaapplication.h>

//#define ENACT_INSTALLATION_IMMEDIATELY

ViewAtt::ViewAtt(QMailMessage* mailIn, bool _inbox, QWidget *parent, Qt::WFlags f)
    : QDialog(parent, f)
{
    mail = mailIn;
    inbox = _inbox;

    setWindowTitle( tr( "Attachments" ) );
    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setSpacing( 6 );

    label = new QLabel( this );
    label->setWordWrap( true );
    vb->addWidget( label );
    listView = new QTableWidget( this );
    listView->installEventFilter( this );
    listView->horizontalHeader()->setFocusPolicy( Qt::NoFocus );
    listView->verticalHeader()->setFocusPolicy( Qt::NoFocus );
    vb->addWidget( listView );
    listView->setFocus();
    QStringList columns;
    columns << tr( "Attachment" );
    columns << tr( "Type" );
    listView->setColumnCount( columns.count() );
    listView->setHorizontalHeaderLabels( columns );
    listView->verticalHeader()->hide();

    init();
}

void ViewAtt::accept()
{
#ifndef ENACT_INSTALLATION_IMMEDIATELY
    int i;
    for (i = 0; i < listView->rowCount(); ++i) {
        QTableWidgetItem *item = listView->item( i, 0 );
        setInstall( item );
    }
#endif
    QDialog::accept();
}

void ViewAtt::reject()
{
#ifdef ENACT_INSTALLATION_IMMEDIATELY
    // reset to initial state
    QListIterator<QTableWidgetItem*> it = on->entryIterator();
    while ( it.hasNext() ) {
        QTableWidgetItem *item = it.next();
        if ( item->checkState() != Qt::Checked ) {
            item->setChecked( Qt::Checked );
            setInstall( item );
        }
    }
    QListIterator<QTableWidgetItem*> it = off->entryIterator();
    while ( it.hasNext() ) {
        QTableWidgetItem *item = it.next();
        if ( item->checkState() == Qt::Checked ) {
            item->setChecked( Qt::Unchecked );
            setInstall( item );
        }
    }
#endif
    QDialog::reject();
}


void ViewAtt::init()
{
    if (inbox) {
        label->setText( tr("<p>Check attachments to add to Documents") );
#ifdef ENACT_INSTALLATION_IMMEDIATELY
        connect( listView, SIGNAL(clicked(QTableWidgetItem*)),
                 this, SLOT(setInstall(QTableWidgetItem*)) );
#endif
    } else {
        label->setText(tr("<p>These are the attachments in this mail"));
    }

    for ( uint i = 0; i < mail->partCount(); i++ ) {
        QMailMessagePart &part = mail->partAt( i );

        bool isFilePart = part.hasBody() && !part.contentType().name().isEmpty();
        bool hasContentLocation = !part.contentLocation().isEmpty();

        if(!isFilePart && !hasContentLocation)
            continue;

        QString attachmentName;

        if(!isFilePart)
            attachmentName = part.contentLocation();
        else 
            attachmentName = part.displayName();

        if ( inbox ) {
            QTableWidgetItem *item = new QTableWidgetItem( attachmentName );
            attachmentMap.insert(item,i);
            if(part.attachmentPath().isEmpty())
            {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                item->setCheckState(!part.attachmentPath().isEmpty() ? Qt::Checked : Qt::Unchecked);
            }
            listView->setRowCount( listView->rowCount() + 1 );
            listView->setItem( listView->rowCount() - 1, 0, item );
            item = new QTableWidgetItem( QString( part.contentType().content() ) );
            item->setFlags( Qt::ItemIsEnabled );
            listView->setItem( listView->rowCount() - 1, 1, item );
#ifdef ENACT_INSTALLATION_IMMEDIATELY
            if ( ins )
                on.append(item);
            else
                off.append(item);
#endif
        } else {
            QTableWidgetItem *item = new QTableWidgetItem( attachmentName);
            listView->setRowCount( listView->rowCount() + 1 );
            listView->setItem( listView->rowCount() - 1, 0, item );
            item = new QTableWidgetItem( QString( part.contentType().content() ) );
            listView->setItem( listView->rowCount() - 1, 1, item );
        }
    }
    if(!inbox)
        listView->setSelectionMode(QAbstractItemView::NoSelection);
    if(listView->rowCount() > 0)
        listView->selectRow(0);
}

void ViewAtt::setInstall(QTableWidgetItem* i)
{
    bool res = false;
    if ( !inbox )
        return;

    if ( i ) {
        int row = listView->row( i );
        QTableWidgetItem *item = listView->item( row, 0 );
        QString filename  = item->text(); // unique?
        bool checked = item->checkState() == Qt::Checked;

        if(!checked)
            return;

        int partIndex = attachmentMap.value(i);

        QMailMessagePart& part = mail->partAt(partIndex);

        if(!part.attachmentPath().isEmpty())
            return;

        //detach the attachment from the mail

        if(part.detachAttachment(Qtopia::documentDir()))
        {
            res = true;
            QContent d( part.attachmentPath());

            if( part.hasBody() )
            {
                QMailMessageContentType type(part.contentType());

                if( d.drmState() == QContent::Unprotected )
                    d.setType( type.content() );

                QString name = type.name();
                d.setName( !name.isEmpty() ? name : part.contentLocation() );
            }
            d.commit();
        }
    }

    if (!res) {
        QString title( tr("Attachment error") );
        QString msg( "<qt>" + tr("Storage for documents is full.<br><br>"
                                 "Some attachments could not be saved.") +
                     "</qt>" );
        QMessageBox::warning(qApp->activeWindow(), title, msg, tr("OK") );
    }

}

bool ViewAtt::eventFilter( QObject *o, QEvent *e )
{
    if ((o == listView) && (e->type() == QEvent::KeyPress)) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        switch( keyEvent->key() ) {
        case Qt::Key_Select:
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!inbox)
                return false;
            if(!listView->hasEditFocus())
                listView->setEditFocus(true);

            QTableWidgetItem* i = listView->currentItem();
            if (i) {
                int row = listView->row( i );
                QTableWidgetItem *item = listView->item( row, 0 );
                bool checked = item->checkState() == Qt::Checked;
                item->setCheckState( checked ? Qt::Unchecked : Qt::Checked );
                return true;
            }
            return false;
        }
        break;
        default:
            return false;
        }
    }
    return QDialog::eventFilter( o, e );
}
