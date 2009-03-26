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
#include <qcontentset.h>

#include <qtopiasql.h>
#include <qstorage.h>
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include "qcontent_p.h"
#include "qcontentstore_p.h"
#include "qcontentsetengine_p.h"

class NullQContentSetEngine : public QContentSetEngine
{
public:
    NullQContentSetEngine()
        : QContentSetEngine( QContentFilter(), QContentSortCriteria(), QContentSet::Synchronous )
    {
    }

    virtual bool isEmpty() const{ return true; }

    virtual void insertContent( const QContent & ){}
    virtual void removeContent( const QContent & ){}

    virtual void clear(){}

    virtual bool contains( const QContent & ) const{ return false; }

protected:
    virtual void commitChanges(){}
    virtual void filterChanged( const QContentFilter & ){}
    virtual void sortCriteriaChanged( const QContentSortCriteria & ){}

    virtual QContentList values( int, int ){ return QContentList(); }
    virtual int valueCount() const{ return 0; }
};

Q_GLOBAL_STATIC(NullQContentSetEngine,nullQContentSetEngine);

////////////////////////////////////////////////////////////////
//
// QContentSet implementation

/*!
    \relates QContentSet
    \typedef QContentList

    Synonym for QList<QContent>.
*/

/*!
    \class QContentSet
    \inpublicgroup QtBaseModule

    \brief The QContentSet class represents a filtered view of all content on a device.

    The content that appears in a QContentSet is defined by a applying a filtering
    criteria to the set with setCriteria().  Any content in the backing store that
    passes the filtering criteria is included in the set, if no filtering criteria
    is applied the set is empty.  By default content in a set is sorted by name, an
    alternative sort order can be specified with setSortCriteria().

    \section1 Asynchronous and Synchronous updates

    QContentSets are synchronized with the backing store; if content is added, removed,
    or modified in a way affecting its inclusion in a QContentSet the QContentSet will
    be updated to reflect that change.  A QContentSet can be made to update its content
    asynchronously or synchronously, the content is updated the same whether caused by
    external events or changing the filtering or sorting criteria.

    Updates to an asynchronous content set are performed in an background thread which
    is started from the event loop when a change to the filtering or sorting criteria
    has been identified or a change notification has been received.  As the update is
    performed pairs of contentAboutToBeRemoved()/contentRemoved() and
    contentAboutToBeInserted()/contentInserted() signals are emitted to indicate
    where changes to the content set have occurred, these signals are synchronized
    to the event loop of the QContentSet's thread so within an event the count() of
    a QContentSet will not change.

    Synchronous QContentSets simply reset the contents of the set when there is a
    possible change in the content.  Changing the filtering or sorting criteria
    will trigger a deferred update of the content set which occur either when control
    returns to the event loop or count() is called.  This allows a number of changes
    to be accumulated before updating the set.

    Generally for persisted QContentSets such those used in document selectors the
    asynchronous update mode should be preferred, while the synchronous mode is
    more suited for one off queries.

    \section1 Explicit content

    In addition to the filtered content a QContentSet has an internal list of explicitly
    maintained content.  Content added to a set explicitly is included in the visible
    set irregardless of whether it passes the filtering criteria and does not have to be
    committed to the backing store.  The contents of this internal list are managed with
    the add() and remove() methods.

    \section1 Example: Iterating over a set of user audio recordings sorted by most recently modified.

    \code
    QContentSet contentSet;

    // Filter for documents with the mime type 'audio/wav' in the 'Recordings' category.
    contentSet.setCriteria( QContentFilter( Document ) );
    contentSet.addCriteria( QContentFilter::mimeType( "audio/wav" ), QContentFilter::And );
    contentSet.addCriteria( QContentFilter::category( "Recordings" ), QContentFilter::And );

    // Sort by modified date in descending order.
    contentSet.setSortCriteria( QContentSortCriteria(
            QContentSortCriteria::LastModified,
            Qt::DescendingOrder ) );

    for( int i = 0; i < contentSet.count(); i++ )
    {
        QContent content = contentSet.content( i );
        ...
    }
    \endcode

    \section1 Tutorials

    For an example of setting the filtering criteria of a QContentSet see the \l {Tutorial: Content
    Filtering} {Content Filtering } tutorial and for an example of listening for changes in a
    QContentSet see the \l { Tutorial: Listening for Content Changes} {Change Listener} tutorial.

  \ingroup content
*/

/*!
    \enum QContentSet::Priority

    This enum specifies the priority to use when scanning a directory.

    \value LowPriority use low priority
    \value NormalPriority use normal priority
    \value HighPriority use low priority - directory will be scanned before
        lower priority jobs.
*/

/*!
    \enum QContentSet::UpdateMode

    Indicates whether the contents of a content set should be updated synchronously or asynchronously.

    \value Synchronous Update the content set in the current thread of execution.
    \value Asynchronous Update the content set in a background thread.
*/

/*!
    Constructs a new unfiltered QContentSet with the specified \a parent.

    The QContentSet can be populated with content from the backing store by specifying a
    filtering criteria with setCriteria() or addCriteria().

    \sa setCriteria(), addCriteria(), setSortCriteria()
*/
QContentSet::QContentSet( QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( QContentFilter(), QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), Synchronous );

    init();
}

/*!
    Constructs a new QContentSet with the specified \a parent containing all content
    from the backing store which matches the filtering \a criteria.

    \sa addCriteria(), setSortCriteria()
 */
QContentSet::QContentSet( const QContentFilter &criteria, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( criteria, QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), Synchronous );

    init();
}


/*!
    Constructs a new QContentSet with the specified \a parent containing all content
    from the backing store which matches the filtering \a criteria and is sorted by
    \a sortOrder.

    \deprecated

    \bold {Note:} The use of a QStringList to specify the sort order is deprecated, use a
    QContentSortCriteria instead.

    \sa addCriteria(), setSortOrder()
 */
QContentSet::QContentSet( const QContentFilter &criteria, const QStringList &sortOrder, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( criteria, QContentSetEngine::convertSortOrder( sortOrder ), Synchronous );

    init();
}


/*!
    Constructs a new QContentSet with the specified \a parent, containing all content
    from \a original.
*/
QContentSet::QContentSet( const QContentSet &original, QObject *parent)
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( original.filter(), QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), Synchronous );

    init();
}

/*!
    Constructs a new QContentSet with the specified \a parent containing all content
    from the backing store which matches the \a tag filtering criteria \a filter.

    \sa addCriteria(), setSortCriteria()
 */
QContentSet::QContentSet( QContentFilter::FilterType tag, const QString& filter, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( QContentFilter( tag, filter ), QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), Synchronous );

    init();
}


/*!
    Constructs a new QContentSet with the specified \a parent containing all content
    from the backing store which matches the \a tag filtering criteria \a filter and
    is sorted by \a sortOrder.

    \bold Example: Construct a QContentSet containing wave files sorted by most recently modified.
    \code
    QContentSet contentSet(
        QContentFilter::MimeType, "audio/wav",
        "time desc" );
    \endcode

    \bold {Note:} The use of a QStringList to specify the sort order is deprecated, use a
    QContentSortCriteria instead.

    \deprecated

    \sa addCriteria(), setSortOrder()
*/
QContentSet::QContentSet( QContentFilter::FilterType tag, const QString& filter, const QStringList &sortOrder, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( QContentFilter( tag, filter ), QContentSetEngine::convertSortOrder( sortOrder ), Synchronous );

    init();
}

/*!
    Constructs an unfiltered QContentSet with the specified \a parent and update \a mode.

    The QContentSet can be populated with content from the backing store by specifying a
    filtering criteria with setCriteria() or addCriteria().

    \sa setCriteria(), addCriteria(), setSortCriteria()
*/
QContentSet::QContentSet( UpdateMode mode, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( QContentFilter(), QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), mode );

    init();
}

/*!
    Constructs a QContentSet with the specified \a parent and update \a mode containing all
    content from the backing store which matches the filtering \a criteria.

    \sa addCriteria(), setSortCriteria()
*/
QContentSet::QContentSet( const QContentFilter &criteria, UpdateMode mode, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( criteria, QContentSortCriteria(QContentSortCriteria::Name, Qt::AscendingOrder), mode );

    init();
}

/*!
    Constructs a QContentSet with the specified \a parent and update \a mode containing all
    content from the backing store which matches the filtering \a criteria and is sorted by
    \a sort.

    \bold Example: Construct an asynchronous QContentSet containing wave files sorted by most recently modified.
    \code
    QContentSet contentSet(
        QContentFilter::mimeType( "audio/wav" )
        QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::DescendingOrder )
        QContentSet::Asynchronous );
    \endcode

    \sa addCriteria()
*/
QContentSet::QContentSet( const QContentFilter &criteria, const QContentSortCriteria &sort, UpdateMode mode, QObject *parent )
    : QObject( parent )
{
    d = QContentStore::instance()->contentSet( criteria, sort, mode );

    init();
}

/*!
    \internal
    offload common initialization functionality.
*/
void QContentSet::init()
{
    if( d )
    {
        d->setParent( this );

        connect( d  , SIGNAL(contentAboutToBeRemoved(int,int)),
                this, SIGNAL(contentAboutToBeRemoved(int,int)) );
        connect( d  , SIGNAL(contentAboutToBeInserted(int,int)),
                this, SIGNAL(contentAboutToBeInserted(int,int)) );
        connect( d  , SIGNAL(contentChanged(int,int)),
                this, SIGNAL(contentChanged(int,int)) );
        connect( d  , SIGNAL(contentRemoved()),
                this, SIGNAL(contentRemoved()) );
        connect( d  , SIGNAL(contentInserted()),
                this, SIGNAL(contentInserted()) );
        connect( d  , SIGNAL(contentChanged()),
                this, SIGNAL(changed()) );

#ifndef QTOPIA_CONTENT_INSTALLER
    connect(qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
            d, SLOT(update()));
    connect(qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
            this, SIGNAL(changed(QContentIdList,QContent::ChangeType)) );
    connect(QContentUpdateManager::instance(), SIGNAL(refreshRequested()),
            d, SLOT(update()));
#endif

        d->update();
    }
    else
    {
        qWarning() << "No content set engine constructed";

        d = nullQContentSetEngine();
    }
}

/*!
  Destroys the QContentSet.
*/
QContentSet::~QContentSet()
{
    delete d;
}

/*!
    Sets the sort \a criteria used to order the set.
*/
void QContentSet::setSortCriteria( const QContentSortCriteria &criteria )
{
    d->setSortCriteria( criteria );
    d->update();
}

/*!
    Returns the sort criteria used to order the set.
*/
QContentSortCriteria QContentSet::sortCriteria() const
{
    return d->sortCriteria();
}

/*!
    Initiates a document scan of a \a path and its sub-directories looking for documents
    that have been added, removed, or modified to bring the backing store's view of the
    path up to date with the file system.

    Scans are performed threaded in a server process, the scan \a priority sets the run
    priority of the scanner threads.  A higher priority scan will finish sooner but may
    reduce the device responsiveness.
*/
void QContentSet::scan( const QString &path, Priority priority )
{
    QtopiaIpcAdaptor qo(QLatin1String("QPE/DocAPI"));
    qo.send(SIGNAL(scanPath(QString,int)), path, priority);
}

/*!
  Finds all documents in the system's document directories which
  match the filter \a mimefilter, and appends the resulting QContent objects to \a folder.
*/
void QContentSet::findDocuments(QContentSet* folder, const QString &mimefilter)
{
    QContentSet d;
    if (!mimefilter.isEmpty())
        d.addCriteria(QContentFilter::MimeType, mimefilter, QContentFilter::Or);
    d.addCriteria(QContentFilter::Role, QLatin1String("Document"), QContentFilter::And);
    folder->appendFrom(d);
    QStorageMetaInfo *storage=QStorageMetaInfo::instance();
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    fsf.removable = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, storage->fileSystems(&fsf) ) {
        QContentSet ide;
        ide.setCriteria(QContentFilter::Location, fs->path());
        ide.addCriteria(QContentFilter::MimeType, mimefilter, QContentFilter::And );
        folder->appendFrom(ide);
    }
}

/*!
  Returns true if the set contains the object \a content, that is:
  \list
  \o if it has been explicitly added
  \o if it matches the filter expression.
  \endlist

  \sa add(), remove(), clear()
*/
bool QContentSet::contains( const QContent &content ) const
{
    return d->contains( content );
}

/*!
    Adds \a content to an explicitly maintained internal list of content.  Items in
    this list appear in the content set irregardless of whether they match the filtering
    criteria.

  \sa remove(), contains(), clear()
*/

void QContentSet::add( const QContent &content )
{
    d->insertContent( content );
    d->update();
}

/*!
    Removes \a content from an explicitly maintained internal list of content.  Items in
    this list appear in the content set irregardless of whether they match the filtering
    criteria.

  \sa add(), contains(), clear()
*/
void QContentSet::remove( const QContent &content )
{
    d->removeContent( content );
    d->update();
}

/*!
    Removes the filtering criteria, and all explicitly added content from the set.

    This will remove all content from the set.

    \sa add(), remove(), contains(), setCriteria(), addCriteria(), clearFilter()
*/
void QContentSet::clear()
{
    d->clear();
    d->update();
}

/*!
  This method does nothing and is only retained for binary compatibility.

  \sa uninstallContent()

  \internal
*/
void QContentSet::installContent()
{
}

/*!
  This method does nothing and is only retained for binary compatibility.

  \sa installContent()

  \internal
*/
void QContentSet::uninstallContent()
{
}

/*!
    Return a QContentList of items in this set.

    This is a relatively expensive operation, and generally should not be used unless it is
    known that only a few items will be returned.

    It is also a snapshot of the currently known items in the list,
    which has a possibility of going out of date immediately after it is obtained.

    Instead of using this method consider iterating over the set directly instead.

    \code
    for( int i = 0; i < contentSet.count(); i++ )
    {
        QContent content = contentSet.content( i );
        ...
    }
    \endcode

    \sa QContentSetModel, itemIds()
*/
QContentList QContentSet::items() const
{
    QContentList l;
    for( int i = 0; i < count(); ++i )
        l.append( content( i ) );
    return l;
}

/*!
    Return a QContentIdList of content IDs in this set.

    This is a relatively expensive operation, and generally should not be used unless it is
    known that only a few items will be returned.

    It is also a snapshot of the currently known items in the list,
    which has a possibility of going out of date immediately after it is obtained.

    Instead of using this method consider iterating over the set directly instead.

    \code
    for( int i = 0; i < contentSet.count(); i++ )
    {
        QContentId contentId = contentSet.contentId( i );
        ...
    }
    \endcode

    \sa QContentSetModel, items()
*/
QContentIdList QContentSet::itemIds() const
{
    QContentIdList l;
    for( int i = 0; i < count(); ++i )
        l.append( contentId( i ) );
    return l;
}

/*!
    Returns the QContent at \a index in a set.
*/
QContent QContentSet::content( int index ) const
{
    return d->content( index );
}

/*!
    Returns the ID of the QContent at \a index in a set.
*/
QContentId QContentSet::contentId( int index ) const
{
    return d->contentId( index );
}

/*!
  \fn void QContentSet::changed(const QContentIdList &idList, QContent::ChangeType type)

  This signal is emitted when QContent included in the filter expression for this
  QContentSet are changed by another application, or by removable media being inserted
  or removed.

  \a idList contains a list of the Ids of QContent items that have changed.
  
  \a type specifies the type of change that is being signalled.
*/

/*!
  \fn void QContentSet::changed()

  This signal is emitted when a large number of QContent objects included in the filter
  expression for this QContentSet are changed by another application or by removable media
  being inserted or removed.
*/

/*!
    \fn QContentSet::aboutToSort()

    This signal is emitted when this QContentSet is about to be sorted.
*/

/*!
    \fn QContentSet::sorted()

    This signal is emitted when this QContentSet has been sorted.
 */

/*!
    \fn QContentSet::contentChanged(int start, int end)

    This signal is emitted when content items between the \a start and \a end indexes
    have changed.
*/

/*!
    \fn QContentSet::contentAboutToBeRemoved(int start, int end)

    This signal is emitted when content items between the \a start and \a end indexes
    are about to be removed.

    Content may be removed from a set as a result of the filtering criteria changing, a QContent
    being explicitly removed from the set, a QContent being deleted from the device, or attributes
    of a QContent changing so that it no longer matches the filtering criteria.

    \sa contentRemoved()
*/

/*!
    \fn QContentSet::contentRemoved()

    This signal is emitted when the content removal indicated by contentAboutToBeRemoved()
    has been completed.

    \sa contentAboutToBeRemoved()
*/

/*!
    \fn QContentSet::contentAboutToBeInserted(int start, int end)

    This signal is emitted when content items are about to be inserted between the \a start
    and \a end indexes.

    Content may be inserted into a set a result of the filtering criteria changing, a QContent
    being explicitly added to the set, a new QContent being created on the device, or attributes
    of a QContent changing so that it matches the filtering criteria.

    \sa contentInserted()
*/

/*!
    \fn QContentSet::contentInserted()

    This signal is emitted when the content insertion indicated by contentAboutToBeInserted()
    has been completed.

    \sa contentAboutToBeInserted()
*/

/*!
    Returns true if this set is empty.
*/
bool QContentSet::isEmpty() const
{
    if (d->updatePending() && d->updateMode() == Synchronous)
        const_cast<QContentSetEngine *>(d)->flush();

    return d->isEmpty();
}

/*!
    Appends the contents of \a other to this QContentSet.
    Currently it appends them as explicit items to the current QContentSet, in the future,
    it will concatenate the two filter sets to create a new aggregate filter set.
*/
void QContentSet::appendFrom( QContentSet& other )
{
    // QTOPIA_DOCAPI_TODO when moving to combinatorial filtering, add "other"s filter set or'd with the overall original filter set
    for (int i=0; i< other.count(); i++)
    {
        add(other.content(i));
    }
    d->update();
}

/*!
    Return the number of QContent objects in this set.
*/
int QContentSet::count() const
{
    if (d->updatePending() && d->updateMode() == Synchronous)
        const_cast<QContentSetEngine *>(d)->flush();

    return d->count();
}

/*!
    Find a QContent object for the executable \a exec in the current QContentSet.
    Returns an empty/invalid QContent if unsuccessful.
    \sa QContent::InvalidId
*/
QContent QContentSet::findExecutable( const QString& exec ) const
{
    for (int i=0; i< count(); i++)
        if (content(i).executableName() == exec)
            return content(i);
    return QContent();
}

/*!
    Find a QContent object for the \a filename in the current QContentSet.
    Returns an empty/invalid QContent if unsuccessful.

    Paths are not acceptable in \a filename, ie the \a filename must
    not contain any "/" characters.

    Note that if more than one item with the \a filename exists in the QContentSet
    no guarantee is provided as to which one is returned.

    This method is typically used with filters such that only one \a filename item
    exists in the filtered set.

    \sa QContent::InvalidId
*/
QContent QContentSet::findFileName( const QString& filename ) const
{
    Q_ASSERT( !filename.contains( QDir::separator() ));

    QString fn = filename;
    fn.prepend( QDir::separator() );
    for (int i=0; i< count(); i++)
        if (content(i).fileName().endsWith( fn ))
            return content(i);
    return QContent();
}

/*!
    Returns the list of mime-types contained in this set.

    For applications, games and settings the type is application/x-executable.

    For documents the type is the document's MIME type, or application/octet-stream
    if the file type is unknown.
*/
QStringList QContentSet::types() const
{
    QStringList result;
    for (int i=0; i<count(); i++) {
        const QContent item=content(i);
        if (!result.contains(item.type()))
            result.append(item.type());
    }
    return result;
}


/*!
    Assigns the given \a contentset to this QContentSet and returns a reference
    to this QContentSet.
*/
QContentSet &QContentSet::operator=(const QContentSet& contentset)
{
    d->setFilter( contentset.filter() );
    d->setSortCriteria( contentset.sortCriteria() );
    d->update();

    return *this;
}

/*!
    Returns the update mode of the content set; either Synchronous or Asynchronous.
*/
QContentSet::UpdateMode QContentSet::updateMode() const
{
    return d->updateMode();
}

/*!
  Clears the content set's current filtering criteria.  This will remove all filtered content from the
  set but leave content that was explicitly added.

  \sa filter(), addCriteria()
 */
void QContentSet::clearFilter()
{
    d->setFilter( QContentFilter() );
    d->update();
}

/*!
    Appends a \a kind filter matching the value \a filter to the existing filtering criteria using
    the given \a operand.

    \bold Example: Filter for documents with the mime type \c image/jpeg or \c {image/png}.
    \code
    QContentSet contentSet;
    contentSet.addCriteria( QContentFilter::MimeType, "image/jpeg", QContentFilter::Or );
    contentSet.addCriteria( QContentFilter::MimeType, "image/png", QContentFilter::Or );
    contentSet.addCriteria( QContentFilter::Role", "Document", QContentFilter::And );
    \endcode
*/
void QContentSet::addCriteria( QContentFilter::FilterType kind, const QString &filter, QContentFilter::Operand operand )
{
    if( operand == QContentFilter::And )
    {
        d->setFilter( d->filter() & QContentFilter( kind, filter ) );
    }
    else if( operand == QContentFilter::Or )
    {
        d->setFilter( d->filter() | QContentFilter( kind, filter ) );
    }
    d->update();
}

/*!
    Appends \a filter to the existing filtering criteria using the given \a operand.

    \bold Example: Filter for documents with the mime type \c image/jpeg or \c {image/png}.
    \code
    QContentSet contentSet( QContentFilter( QContent::Document ) );
    contentSet.addCriteria(
        QContentFilter::mimeType( "image/jpeg" ) | QContentFilter::mimeType( "image/png" ),
        QContentFilter::And );
    \endcode
 */
void QContentSet::addCriteria(const QContentFilter& filter, QContentFilter::Operand operand )
{
    if( operand == QContentFilter::And )
    {
        d->setFilter( d->filter() & filter );
    }
    else if( operand == QContentFilter::Or )
    {
        d->setFilter( d->filter() | filter );
    }
    d->update();
}

/*!
    Sets the filtering criteria of a QContentSet to a \a kind filter matching the value \a filter.

    This will replace any existing filtering criteria.
 */
void QContentSet::setCriteria(QContentFilter::FilterType kind, const QString &filter)
{
    d->setFilter( QContentFilter( kind, filter ) );
    d->update();
}

/*!
    Sets the filtering criteria of a QContentSet to \a filter.

    This will replace any existing filtering criteria.

    \bold Example: Filter for documents with the mime type \c image/jpeg or \c {image/png}.
    \code
    QContentSet contentSet;
    contentSet.setCriteria( QContentFilter( QContent::Document )
            & ( QContentFilter::mimeType( "image/jpeg" )
            | QContentFilter::mimeType( "image/png" ) ) );
    \endcode
 */
void QContentSet::setCriteria(const QContentFilter& filter)
{
    d->setFilter( filter );
    d->update();
}

/*!
    Returns the current filtering criteria of the QContentSet.
*/
QContentFilter QContentSet::filter() const
{
    return d->filter();
}

/*!
    Sets the attributes that content in this QContentSet is ordered by to \a sortOrder.

    Valid sort attributes are:
    \list
        \o \c {name}: The user visible name of the content.
        \o \c {type}: The MIME type of the content.
        \o \c {time}: The date-time the content was last modified.
        \o \c {synthetic/[group]/[key]}: A content property with the given property \c group and \c key.
    \endlist

    To specify whether to sort in ascending or descending order append \c asc or \c desc preceded by a space
    to the end of the attribute name.

    \bold Example: Sorting a QContent by last modified date in descending order.

    \code
    contentSet.setSortOrder( QStringList << "time desc" );
    \endcode

    This method has been deprecated, use setSortCriteria() instead.

    \deprecated

    \sa setSortCriteria()
 */
void QContentSet::setSortOrder( const QStringList &sortOrder )
{
    d->setSortOrder( sortOrder );
    d->update();
}

/*!
    Returns the attributes the content in this QContentSet is ordered by.

    This method has been deprecated, use sortCriteria() instead.

   \deprecated

   \sa sortCriteria()
 */
QStringList QContentSet::sortOrder() const
{
    return d->sortOrder();
}

/*!
  \reimp
*/
void QContentSet::timerEvent(QTimerEvent *e)
{
    QObject::timerEvent( e );
}

/*!
    Returns the number of \l {QContent}s in the database that match a content \a filter.
*/
int QContentSet::count( const QContentFilter &filter )
{
    return QContentStore::instance()->contentCount( filter );
}

////////////////////////////////////////////////////////////////
//
// ContentSetModel implementation

class QContentSetModelPrivate
{
public:
    QDrmRights::Permission selectPermission;
    QDrmRights::Permissions mandatoryPermissions;
};

/*!
  \class QContentSetModel
    \inpublicgroup QtBaseModule


  \brief The QContentSetModel class provides a data model to represent the items in a QContentSet.

  This class provides access to the content system information, providing extra functions for
  retrieving information relevant to the \l{QContentSet}{content set} passed in the
  constructor.

  Example usage:
  \code
    QContentSet contentset( QContentFilter::Directory, "/Documents" );
    QContentSetModel model( &contentset );

    QListView listview( this );
    listview->setModel( &model );
  \endcode


   For a complete example that includes the use of \c QContentSetModel, see
   the \l {Tutorial: Content Filtering} {Content Filtering} tutorial, and the
   \l {Model/View Programming} documentation.

  \ingroup content
*/

/*!
  Constructs a new model based on the QContentSet \a cls with parent \a parent.
*/
QContentSetModel::QContentSetModel( const QContentSet *cls, QObject *parent )
    : QAbstractListModel(parent), contentSet(cls), d(0)
{
    d = new QContentSetModelPrivate;

    d->selectPermission = QDrmRights::InvalidPermission;
    d->mandatoryPermissions = QDrmRights::NoPermissions;

    connect( cls, SIGNAL(contentAboutToBeInserted(int,int)),
             this, SLOT(beginInsertContent(int,int)) );
    connect( cls, SIGNAL(contentInserted()),
             this, SLOT(endInsertContent()) );
    connect( cls, SIGNAL(contentAboutToBeRemoved(int,int)),
             this, SLOT(beginRemoveContent(int,int)) );
    connect( cls, SIGNAL(contentRemoved()),
             this, SLOT(endRemoveContent()) );
    connect( cls, SIGNAL(contentChanged(int,int)), this, SLOT(contentChanged(int,int)) );
    connect( cls, SIGNAL(aboutToSort()), this, SLOT(emitLayoutAboutToBeChanged()) );
    connect( cls, SIGNAL(sorted()), this, SLOT(emitLayoutChanged()) );
    connect( cls->d, SIGNAL(reset()), this, SLOT(doReset()) );
    connect( cls->d, SIGNAL(updateStarted()), this, SIGNAL(updateStarted()) );
    connect( cls->d, SIGNAL(updateFinished()), this, SIGNAL(updateFinished()) );
    connect( cls, SIGNAL(destroyed()), this, SLOT(clearContentSet()) );
}

/*!
  Destroys the QContentSetModel object.
 */

QContentSetModel::~QContentSetModel()
{
    delete d;
}

/*!
  Returns the number of rows in the model. Since this is a flat list the
  \a parent argument will always be the default null index, indicating a
  top level item.  The result is the count of the items in the backing
  store which match the filter expression plus any explicitly added
  non-matching items.
*/
int QContentSetModel::rowCount( const QModelIndex & /* parent */ ) const
{
    if (!contentSet)
        return 0;
    return contentSet->count();
}

/*!
  \enum QContentSetModel::ItemDataRole

  Each item in the model has a set of data elements associated with it,
  each with its own role. The roles are used by the view to indicate to
  the model which type of data it needs.

  QContentSetModel adds the following roles:

  \value FilenameRole The filename of the data item to be displayed.
  \value ContentRole The QContent object of the data item to be displayed.
  \value ThumbnailRole The thumbnail if available of the data item to be displayed.

  \sa Qt::ItemDataRole, Qtopia::ItemDataRole
*/

/*!
  Returns the appropriate QVariant data from the model for the given \a index.
  Depending upon the \a role, the QVariant will contain the name of the QContent
  object at that index, its icon, or relevant tooltip text.  The "ToolTip"
  text is shown when the user hovers the cursor over the item in the
  model view.  The tooltip text will display the comment() field from the
  QContent object, or if error() is true, the error text.  Additionally
  the DRM rights in summary form will be shown.
*/
QVariant QContentSetModel::data( const QModelIndex & index, int role ) const
{
    if (!contentSet)
        return QVariant();

    switch ( role )
    {
        case Qt::DecorationRole:
            return content( index ).icon();
        case Qt::DisplayRole:
            return content( index ).name();
        case Qt::ToolTipRole:
            return content( index ).error()
                ? content( index ).errorString()
                : content( index ).comment();
        case FilenameRole:
            return content( index ).fileName();
        case ContentRole:
            return QVariant::fromValue( content( index ) );
        case ThumbnailRole:
            return content( index ).thumbnail();
        case Qtopia::AdditionalDecorationRole:
            {
                QContent c = content( index );

                if ( c.drmState() == QContent::Protected ) {
                    static const QIcon lockIcon(":icon/drm/Drm_lock");
                    static const QIcon invalidLockIcon(":icon/drm/Drm_lock_invalid");
                    QDrmRights::Permission permission = d->selectPermission == QDrmRights::InvalidPermission
                            ? QMimeType::fromId(c.type()).permission()
                            : d->selectPermission;

                    QIcon icon = (content(index).permissions( false ) & permission) == permission
                                ? lockIcon
                                : invalidLockIcon;
                    return QVariant(icon);
                } else if( c.role() == QContent::Application && c.categories().contains( QLatin1String( "Packages" ) ) ) {
                    static const QIcon packagesIcon( ":icon/packagemanager/PackageManager" );
                    return packagesIcon;
                }
            }
    }
    return QVariant();
}

/*!
    \reimp
*/
Qt::ItemFlags QContentSetModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags( index );

    if( d->mandatoryPermissions != QDrmRights::NoPermissions &&
        (content( index ).permissions() & d->mandatoryPermissions) != d->mandatoryPermissions )
        flags &= ~(Qt::ItemIsEnabled );

    return flags;
}

/*!
  Returns the QContent for the requested \a row.
*/
QContent QContentSetModel::content( uint row ) const
{
    if (!contentSet)
        return QContent();

    return contentSet->d->content( row );
}

/*!
  Returns the QContent for the requested \a index.
 */

QContent QContentSetModel::content( const QModelIndex & index ) const
{
    return content(index.row());
}

/*!
  Returns the QContentId for the requested \a index.
 */

QContentId QContentSetModel::contentId( const QModelIndex & index ) const
{
    return contentId( index.row() );
}

/*!
  Returns the QContentId for the requested \a row. The value returned may be
  QContent::InvalidId if the QContent has not been committed to the backing store, or there
  is no associated QContentSet for the model.
*/
QContentId QContentSetModel::contentId( uint row ) const
{
    if (!contentSet)
        return QContent::InvalidId;

    return contentSet->d->contentId( row );
}

/*!
    Sets the \a permissions a QContent must have to in order to be selectable in the content
    model.

    If a row in the model doesn't have all the mandatory permissions the
    Qt::ItemIsEnabled and Qt::ItemIsSelectable flags will be cleared.
    \sa QDrmRights, mandatoryPermissions()
*/
void QContentSetModel::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->mandatoryPermissions = permissions;

    emit dataChanged( QModelIndex(), QModelIndex() );
}

/*!
    Returns the permissions a QContent must have in order to be selectable in the content
    model.
    \sa QDrmRights, setMandatoryPermissions()
*/
QDrmRights::Permissions QContentSetModel::mandatoryPermissions() const
{
    return d->mandatoryPermissions;
}

/*!
    Sets the \a permission which indicates the intended usage of the content in the model.
    Content in the model which doesn't have the selected permission will be displayed with
    an invalid rights icon.

    If the permission is QDrmRights::InvalidPermission the default permission for the
    content is used.
    \sa QDrmRights, selectPermission()
*/
void QContentSetModel::setSelectPermission( QDrmRights::Permission permission )
{
    d->selectPermission = permission;

    emit dataChanged( QModelIndex(), QModelIndex() );
}

/*!
    Returns the permission which indicates the intended usage of the content in the model.
    \sa QDrmRights, setSelectPermission()
*/
QDrmRights::Permission QContentSetModel::selectPermission() const
{
    return d->selectPermission;
}

void QContentSetModel::doReset()
{
    reset();
}

void QContentSetModel::beginInsertContent( int start, int end )
{
    beginInsertRows( QModelIndex(), start, end );
}

void QContentSetModel::endInsertContent()
{
    endInsertRows();
}

void QContentSetModel::beginRemoveContent( int start, int end )
{
    beginRemoveRows( QModelIndex(), start, end );
}

void QContentSetModel::endRemoveContent()
{
    endRemoveRows();
}

void QContentSetModel::contentChanged( int start, int end )
{
    emit dataChanged( index( start, 0 ), index( end, 0 ) );
}

void QContentSetModel::emitLayoutAboutToBeChanged()
{
    emit layoutAboutToBeChanged();
}

void QContentSetModel::emitLayoutChanged()
{
    emit layoutChanged();
}


void QContentSetModel::clearContentSet()
{
    contentSet = 0;
    reset();
}

/*!
    \fn QContentSetModel::updateStarted()

    Signals that an update of the contents of the set has started.

    This only applies to asynchronous content sets.

    \sa updateInProgress(), updateFinished()
*/

/*!
    \fn QContentSetModel::updateFinished()

    Signals that an update of the contents of the set has started.

    This only applies to asynchronous content sets.

    \sa updateInProgress(), updateStarted()
*/

/*!
    Returns true if the content of the set are being updated.

    This only applies to asynchronous content sets.

    \sa updateStarted(), updateFinished()
*/
bool QContentSetModel::updateInProgress() const
{
    return !contentSet || contentSet->d->updateInProgress();
}

/*!
    \fn QContentSet::serialize(Stream &stream) const
    \internal
*/
template <typename Stream> void QContentSet::serialize(Stream &stream) const
{
    stream << filter();

    QContentIdList explicitIds;

    stream << explicitIds;
    stream << sortOrder();
}

/*!
    \fn QContentSet::deserialize(Stream &stream)
    \internal
*/
template <typename Stream> void QContentSet::deserialize(Stream &stream)
{
    QContentFilter filter;
    QContentIdList explicitIds;
    QStringList sortOrder;

    stream >> filter;
    stream >> explicitIds;
    stream >> sortOrder;

    clear();
    setSortOrder( sortOrder );
    setCriteria( filter );

    foreach( const QContentId &id, explicitIds )
        add( QContent( id ) );
}

/*!
    \fn QDataStream &operator << ( QDataStream &ds, const QContentSet &set )
    \internal
*/
/*!
    \fn QDataStream &operator >> ( QDataStream &ds, QContentSet &set )
    \internal
*/

Q_IMPLEMENT_USER_METATYPE(QContentSet)
Q_IMPLEMENT_USER_METATYPE_ENUM(QContentSet::UpdateMode)


