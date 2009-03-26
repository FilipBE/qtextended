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

//Qtopia
#include <QSoftMenuBar>
#include <QDebug>
#include <QtopiaApplication>
#include <QtopiaAbstractService>
#include <qtopiaservices.h>
#include <QDSAction>
#include <QDSData>
#include <QDSServices>

//Qt
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QMenu>
#include <QAction>

#include "imagebrowser.h"

class ImageBrowserPrivate :  public QObject
{
    Q_OBJECT
public:
    ImageBrowserPrivate() {}
    ~ImageBrowserPrivate() {}

    bool dirty;
    int centerIndex;
    QMenu* contextMenu;

    QAction* a_del,
           * a_edit,
           * a_contact,
           * a_send;

    QString picfile;
    PictureFlowView *view;
    QContentSetModel *model;
    QContent currentContent;
};

ImageBrowser::ImageBrowser(QContentSetModel* model, QObject *parent):
    QObject(parent),
    d(new ImageBrowserPrivate)
{
    d->dirty = true;

    d->view = new PictureFlowView;
    connect(d->view, SIGNAL(currentChanged(const QModelIndex&)), this,SLOT(currentChanged(const QModelIndex&)));
    connect(d->view, SIGNAL(activated(const QModelIndex&)), this,SLOT(currentChanged(const QModelIndex&)));

    d->view->setModel( d->model = model);
    d->view->setModelRole(QContentSetModel::ThumbnailRole);
    d->currentContent = d->model->content(d->view->currentModelIndex());


    d->contextMenu = QSoftMenuBar::menuFor( d->view );
    d->a_del = new QAction( QIcon(), tr("Delete"), this);
    d->a_edit = new QAction( QIcon(), tr("Edit"), this);
    d->a_contact = new QAction( QIcon(":image/addressbook/AddressBook"), tr("Save to contact..."), this);
    d->a_send = new QAction( QIcon(":icon/beam"), tr("Send to contact..."), this);

    connect(d->a_del, SIGNAL(triggered()), this, SLOT(deleteCurrentSlide()));
    connect(d->a_edit, SIGNAL(triggered()), this, SLOT(editCurrentSlide()));
    connect(d->a_contact, SIGNAL(triggered()), this, SLOT(addCurrentSlideToContact()));
    connect(d->a_send, SIGNAL(triggered()), this, SLOT(sendCurrentSlideToContact()));

    d->contextMenu->addAction(d->a_del);
    d->contextMenu->addAction(d->a_edit);
    d->contextMenu->addAction(d->a_contact);
    d->contextMenu->addAction(d->a_send);


    d->picfile = Qtopia::tempDir() + "image.jpg";
}

ImageBrowser::~ImageBrowser()
{
    delete d;
}

QWidget *ImageBrowser::pictureflowWidget()
{
    return d->view;
}

void ImageBrowser::show()
{
    d->view->showFullScreen();
    d->view->show();
}

void ImageBrowser::currentChanged(const QModelIndex& index)
{
    d->currentContent =  d->model->content(index);
}

void ImageBrowser::editCurrentSlide()
{
    if (d->currentContent.isNull())
        return;
    d->currentContent.execute();
}

void ImageBrowser::addCurrentSlideToContact()
{
    if ( !d->currentContent.isNull() )
    {
        // Find a suitable QDS service
        QDSServices services( QString( "image/jpeg" ) );

        // Select the first service to create the action (assuming there
        // is only be one address book type application on the device)
        QDSAction action( services.findFirst( "setContactImage" ) );
        if ( !action.isValid() ) {
            qWarning( "Camera found no service to set the contact image" );
            return;
        }

        QFile pixFile(d->currentContent.fileName());
        QDSData pixData(pixFile, QMimeType( "image/jpeg" ) );

        if ( action.exec( pixData ) != QDSAction::Complete ) {
            qWarning( "Camera unable to set contact image" );
            return;
        }
    }
}

void ImageBrowser::sendCurrentSlideToContact()
{
    //copy file
    if(d->currentContent.isNull())
        return;
    QFile input(d->currentContent.fileName());
    if(!input.open(QIODevice::ReadOnly)){
        return; //error
    }
    QFile output(d->picfile);
    if(!output.open(QIODevice::WriteOnly)){
        return;
    }

    const int BUFFER_SIZE = 1024;
    qint8 buffer[BUFFER_SIZE];

    QDataStream srcStr(&input);
    QDataStream destStr(&output);

    while(!srcStr.atEnd()) {
        int i = 0;
        while(!srcStr.atEnd() && i < BUFFER_SIZE){
             srcStr >> buffer[i];
             i++;
        }
        for(int k = 0; k < i; k++) {
             destStr << buffer[k];
        }
    }

    QtopiaServiceRequest e("Email","writeMessage(QString,QString,QStringList,QStringList)");
    e << QString() << QString() << QStringList() << QStringList( QString( d->picfile ) );
    e.send();
}


void ImageBrowser::deleteCurrentSlide()
{
    if (d->currentContent.isNull())
        return;
    switch(QMessageBox::warning(0, tr("Confirmation"),
            tr("<qt>Delete '%1'?</qt>", "%1 = file name").arg(d->currentContent.name()),
            QMessageBox::Yes,
            QMessageBox::No))
    {
        case QMessageBox::Yes:
            d->currentContent.removeFiles();
            break;
        default:
            //nothing
            break;
    }
}
#include "imagebrowser.moc"
