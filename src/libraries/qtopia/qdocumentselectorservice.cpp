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
#include <QDocumentSelectorService>
#include "qdocumentserverchannel_p.h"
#include "qdocumentselectorsocketserver_p.h"
#include <unistd.h>
#include <QFile>
#include <QEventLoop>
#include <QContentSortCriteria>

class QFdFile : public QFile
{
public:
    QFdFile( QObject *parent = 0 );

    virtual bool open( OpenMode mode );
    virtual void close();
};

QFdFile::QFdFile( QObject *parent )
    : QFile( parent )
{
}

bool QFdFile::open( OpenMode mode )
{
    Q_UNUSED( mode );

    return false;
}

void QFdFile::close()
{
    flush();

    ::close( handle() );

    QFile::close();
}


class QDocumentSelectorServicePrivate : public QDocumentServerClient
{
    Q_OBJECT
public:
    QDocumentSelectorServicePrivate( QObject *parent = 0 )
        : QDocumentServerClient( "QDocumentSelectorServer", parent )
        , filter( QContent::Document )
        , sortCriteria( QContentSortCriteria::Name, Qt::AscendingOrder )
        , eventLoop( 0 )
    {
    }

    QContentFilter filter;
    QContentSortCriteria sortCriteria;

    QContent document;
    QFdFile data;

    QEventLoop *eventLoop;

signals:
    void documentOpened( const QContent &document, QIODevice *data );

protected slots:
    virtual void invokeSignal( const QDocumentServerMessage &message )
    {
        const QByteArray signature = message.signature();
        const QVariantList arguments = message.arguments();

        if( signature == "documentOpened(QContent,QIODevice::OpenMode)" )
        {
            Q_ASSERT( arguments.count() == 2 && message.rights().count() == 1 );

            if( data.isOpen() )
                data.close();

            int fd = message.rights()[ 0 ].dupFd();

            if( data.QFile::open( fd, qvariant_cast< QIODevice::OpenMode >( arguments[ 1 ] ) ) )
            {
                document = qvariant_cast< QContent >( arguments[ 0 ] );

                emit documentOpened( document, &data );

                eventLoop->exit( 0 );
            }
            else
            {
                ::close( fd );

                Q_ASSERT( false );

                eventLoop->exit( 1 );
            }
        }
        else if( signature == "cancelled()" )
        {
            Q_ASSERT( arguments.count() == 0 );

            eventLoop->exit( 1 );
        }
    }
};


/*!
    \class QDocumentSelectorService
    \inpublicgroup QtBaseModule
    \brief The QDocumentSelectorService class provides an interface for low-privilege applications to open to user selected documents.

    Applications with insufficient permissions to access the Qt Extended Document System directly may use the document selector service to
    request that a server application open files on their behalf.  Rather that opening a document specified by the application, the server
    displays a document selector for the user to select a document which is then opened and read access granted to the application.

    Documents may also write to files by requesting permission to either save an already open document or create a new document.  In
    both cases the user must confirm they wish to save the document before the server will open it in write-only mode.

    Only one document may be opened by the service at a time. The currently opened document may be retrieved using the
    selectedDocument() method and the selectedDocumentData() method returns a QIODevice through the the document may be read from or
    written to.

    \ingroup documentselection
    \sa QDocumentSelector
*/

/*!
    Contructs a new document selector service with the given \a parent.
*/
QDocumentSelectorService::QDocumentSelectorService( QObject *parent )
    : QObject( parent )
    , d( new QDocumentSelectorServicePrivate )
{
    connect( d, SIGNAL(documentOpened(QContent,QIODevice*)), this, SIGNAL(documentOpened(QContent,QIODevice*)) );

    d->connect();
}

/*!
    Destroys the document selector service.
*/
QDocumentSelectorService::~QDocumentSelectorService()
{
    delete d;
}

/*!
    Returns the services document selector's content filter.
*/
QContentFilter QDocumentSelectorService::filter() const
{
    return d->filter;
}

/*!
    Sets the service document selector's content \a filter.
*/
void QDocumentSelectorService::setFilter( const QContentFilter &filter )
{
    d->filter = filter;
}

/*!
    Returns the sort mode of the service document selector.
*/
QContentSortCriteria QDocumentSelectorService::sortCriteria() const
{
    return d->sortCriteria;
}

/*!
    Sets the \a sort criteria of the service document selector.
*/
void QDocumentSelectorService::setSortCriteria( const QContentSortCriteria &sort )
{
    d->sortCriteria = sort;
}

/*!
    Returns the last selected document.
*/
QContent QDocumentSelectorService::selectedDocument() const
{
    return d->document;
}

/*!
    Returns a pointer to the data of the selected document.

    The service retains ownership of the returned QIODevice.

    \sa selectedDocument()
*/
QIODevice *QDocumentSelectorService::selectedDocumentData()
{
    return &d->data;
}

/*!
    Attempts to create a new document.

    The user will be prompted with a suggested \a name and the \a type to save the document as.  Returns true if the user accepts and the
    document was succesfully created; false otherwise.

    If successful the documentOpened() signal will be emitted with the document QContent and a pointer to the document QIODevice opened in
    write-only mode.  These can also be obtained from the selectedDocument() and selectedDocumentData() methods respectively upon return.

    The dialog displayed will parented off the given \a widget.

    \sa selectedDocument(), selectedDocumentData()
*/
bool QDocumentSelectorService::newDocument( const QString &name, const QString &type, QWidget *widget )
{
    Q_UNUSED(widget);

    if( d->eventLoop && d->eventLoop->isRunning() )
        return false;

    QStringList types;

    types << type;

    d->callSlotWithArgumentList( "newDocument(QString,QStringList)", QVariantList() << name << types );

    if( !d->eventLoop )
        d->eventLoop = new QEventLoop( this );

    return d->eventLoop->exec() == 0;
}

/*!
    Attempts to create a new document.

    The user will be prompted with a suggested \a name and possible mime \a types to save the document as.  Returns true if the user 
    accepts and the document was succesfully created; false otherwise.

    If successful the documentOpened() signal will be emitted with the document QContent and a pointer to the document QIODevice opened in
    write-only mode.  These can also be obtained from the selectedDocument() and selectedDocumentData() methods respectively upon return.

    The dialog displayed will parented off the given \a widget.

    \sa selectedDocument(), selectedDocumentData()
*/
bool QDocumentSelectorService::newDocument( const QString &name, const QStringList &types, QWidget *widget )
{
    Q_UNUSED(widget);

    if( d->eventLoop && d->eventLoop->isRunning() )
        return false;

    d->callSlotWithArgumentList( "newDocument(QString,QStringList)", QVariantList() << name << types );

    if( !d->eventLoop )
        d->eventLoop = new QEventLoop( this );

    return d->eventLoop->exec() == 0;
}


/*!
    Prompts the user to select a document to open in read-only mode.  Returns true if the user selects a document and it is successfully
    opened; false otherwise.

    The content displayed in the selector may be altered by setting the content filter with setFilter() and the sort order with sortOrder().

    If successful the documentOpened() signal will be emitted with the document QContent and a pointer to the document QIODevice opened in
    read-only mode.  These can also be obtained from the selectedDocument() and selectedDocumentData() methods respectively upon return.

    The dialog displayed will parented off the given \a widget.

    \sa selectedDocument(), selectedDocumentData()
*/
bool QDocumentSelectorService::openDocument( QWidget *widget )
{
    Q_UNUSED(widget);

    if( d->eventLoop && d->eventLoop->isRunning() )
        return false;

    d->callSlotWithArgumentList( "openDocument(QContentFilter,QContentSortCriteria)",
            QVariantList() << QVariant::fromValue( d->filter ) << QVariant::fromValue( d->sortCriteria ) );

    if( !d->eventLoop )
        d->eventLoop = new QEventLoop( this );

    return d->eventLoop->exec() == 0;
}

/*!
    Tries to open the last opened document in write only mode so changes can be saved.

    The user will be prompted before the document is opened.  Returns true if the user accepts and the document was
    succesfully opened; false otherwise.

    If successful the documentOpened() signal will be emitted with the document QContent and a pointer to the document QIODevice opened in
    write-only mode.  These can also be obtained from the selectedDocument() and selectedDocumentData() methods respectively upon return.

    The dialog displayed will parented off the given \a widget.

    \sa selectedDocument(), selectedDocumentData()
*/
bool QDocumentSelectorService::saveDocument( QWidget *widget )
{
    Q_UNUSED(widget);

    if( d->document.isNull() || d->eventLoop && d->eventLoop->isRunning() )
        return false;

    d->callSlotWithArgumentList( "saveDocument()", QVariantList() );

    if( !d->eventLoop )
        d->eventLoop = new QEventLoop( this );

    return d->eventLoop->exec() == 0;
}

/*!
    Closes the selected document QIODevice and clears the selected document.

    If the service is displaying a dialog that will also be closed.

    To close the document without clearing the selection close the QIODevice returned by selectedDocumentData() instead.
*/
void QDocumentSelectorService::close()
{
    if( !d->document.isNull() )
    {
        d->data.close();
        d->document = QContent();

        d->callSlotWithArgumentList( "close()", QVariantList() );
    }

    if( d->eventLoop && d->eventLoop->isRunning() )
        d->eventLoop->exit( 1 );
}

/*!
    \fn void QDocumentSelectorService::documentOpened( const QContent &document, QIODevice *data )

    Signals that a \a document has been opened and it may be read from or written to using \a data.

    After this signal has been emitted the values of \a document and \a data may be acquired from the selectedDocument() and
    selectedDocumentData() methods.
*/

#include "qdocumentselectorservice.moc"

