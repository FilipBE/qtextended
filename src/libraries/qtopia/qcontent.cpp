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
#include <QCache>
#include <QReadWriteLock>

#include <qcontent.h>
#include "drmcontent_p.h"

#include <qtranslatablesettings.h>
#include <qtopiasql.h>
#include <qmimetype.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopiaipcenvelope.h>
#include "contentpluginmanager_p.h"
#include <qcategorymanager.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include "qcontent_p.h"
#endif
#include <QValueSpaceObject>
#include "qcontentstore_p.h"
#include "qfscontentengine_p.h"
#include <QContentSet>
#include <QContentFilter>

static const uint ContentCacheSize = 100;

/*!
    \variable QContent::InvalidId
    \brief A constant representing an invalid QContent identifier
*/
const QContentId QContent::InvalidId = QContentId(0xffffffff, Q_UINT64_C(0xffffffffffffffff));

Q_IMPLEMENT_USER_METATYPE(QContent);
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QContentId);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentId, QContentId);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentIdList, QContentIdList);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentList, QContentList);
Q_IMPLEMENT_USER_METATYPE_ENUM(QContent::ChangeType);
Q_IMPLEMENT_USER_METATYPE_ENUM(QContent::Role);
Q_IMPLEMENT_USER_METATYPE_ENUM(QContent::DrmState);

static QReadWriteLock databaseLock;

Q_GLOBAL_STATIC_WITH_ARGS(QContent, nullQContent, (QContentStore::instance()->contentFromEngineType( QString() )));
Q_GLOBAL_STATIC_WITH_ARGS(QContent::DocumentSystemConnection, documentSystemConnection,
                          (qApp->type() == QApplication::GuiServer ? QContent::DocumentSystemDirect : QContent::DocumentSystemClient));

/*!
    \relates QContent
    \typedef QtopiaDatabaseId

    A QtopiaDatabaseId is a globally unique identifier for a QContent backing store.
    Synonym for quint32.
*/

/*!
    \relates QContent
    \typedef QContentId

    A QContentId is a globally unique identifier for a QContent record.
    Synonym for QPair<QtopiaDatabaseId, quint64>.
 */

/*!
    \relates QContent
    \typedef QContentIdList

    Synonym for QList<QContentId>.
 */

/*!
  \class QContent
    \inpublicgroup QtBaseModule

  \brief The QContent class represents a content carrying entity in the
  Qt Extended system, where content includes resources which an end-user may
  view or manage.

  By creating content with a \l Role of \l Data using the setRole() method,
  resources not intended to be directly consumed by the end-user may also be
  managed.

  A QContent object can be either an executable application or a document. Documents include
  text documents, pictures, pieces of music, and video clips.

  An instance of a QContent may, for example, represent any one of the following
  types of content:
  \list
  \o a stream
  \o a file
  \o a temporary file
  \o a DRM controlled file (possibly encrypted).
  \o an item in a container, e.g. an archive or DRM multi-part file
  \endlist

  The QContent class is responsible for
  providing access to meta-data about the content contained in a file
  or stream.

  As a system-wide invariant, the backing store is authoritative for information
  about content available on the device.  For example content stored in a DRM file
  or a stream may not have a logical file on the file-system.

  In general applications should use the QContent and QContentSet interface to
  manage, and search for resources, instead of calling through to the file-system.

  The backing store is typically an SQL database, although it could be some other
  kind of persistent storage.

  The static methods install() and uninstall()
  are used to update records of meta-data in the backing store in response to hardware and
  software events such as:
  \list
  \o removable storage being removed
  \o new content arriving via WAP or MMS
  \o DRM expiry.
  \endlist

  To be notified of these events, create a QContentSet object and connect
  to its \l{QContentSet::changed()} signal.

  \sa QContentSet, QContentFilter

  \ingroup content

   For an example of using QContent to read, write and create documents see the \l {Tutorial: A Notes
   Application}{Notes Application} tutorial.  For other examples using QContent, see the {Tutorial:
   Content Filtering}{Content Filtering} and \l {Tutorial: Listening for Content Changes}{Change
    Listener} tutorials.
*/

/*!
    \enum QContent::ChangeType

    This enum specifies the type of change made to a QContent.

    \value Added the QContent has been added.
    \value Removed the QContent has been deleted.
    \value Updated the QContent has been modified.

    \sa QContentSet::changed()
*/

/*!
  \typedef QContent::UsageMode
    \deprecated
    \c UsageMode has been deprecated, use Role instead.

    \sa QContent::Role
 */

/*!
    \enum QContent::Role

    This enum specifies the type of document this QContent object appears to be.

    \value UnknownUsage unknown mode (default)
    \value Document a user-visible document file suitable for "open"
    \value Data a data or config file not suitable for "open"
    \value Application an application (possibly Java, possibly DRM controlled)
    \value Folder a category used to group applications.
 */


/*!
  \enum QContent::DrmState

    This enum specifies the DRM State of this QContent object.

    \value Unprotected plain text "legacy" file, not subject to DRM
    \value Protected file subject to DRM.
 */

/*!
  \fn bool QContent::linkFileKnown() const

   Returns true if the file associated with this QContent object is already
   known. If false, calling file() will generate a file name.

   \sa QContent::fileKnown(), QContent::linkFile(), QContent::file()
 */

/*!
    Constructs an empty invalid content object.
*/
QContent::QContent()
    : d( 0 )
{
    *this = *nullQContent();
}

/*!
  Creates a QContent object by fetching the meta-data from the backing
  store specified by \a id.  This method will be fast if the object
  has already been referenced by QContent and QContentSet
  classes loaded in the current process, since internally the results
  of calls to the backing store are cached.
 */
QContent::QContent( QContentId id )
    : d( 0 )
{
    *this = QContentStore::instance()->contentFromId( id );
}

/*!
  Create a QContent based on the content contained in the file represented by \a fi.

  Passing \a store specifies whether this content object is stored into the backing store,
  or only used as a local object.
 */
QContent::QContent( const QFileInfo &fi, bool store )
    : d( 0 )
{
    QContentStore::LookupFlags lookup = QContentStore::Lookup | QContentStore::Construct;

    if( store )
        lookup |= QContentStore::Commit;

    *this = QContentStore::instance()->contentFromFileName( fi.absoluteFilePath(), lookup );
}

/*!
  Create a QContent based on the content contained in the file represented by \a fileName.

  Passing \a store specifies whether this content object is stored into the backing store database,
  or only used as a local object.
 */
QContent::QContent( const QString &fileName, bool store )
    : d( 0 )
{
    QContentStore::LookupFlags lookup = QContentStore::Lookup | QContentStore::Construct;

    if( store )
        lookup |= QContentStore::Commit;

    *this = QContentStore::instance()->contentFromFileName( fileName, lookup );
}

/*!
  Create a QContent by copying the \a other content object.
*/
QContent::QContent( const QContent &other )
    : d( 0 )
{
    (*this) = other;  // use assignment operator

    Q_ASSERT(d.constData() != NULL);
}

/*!
  \internal
 */
QContent::QContent( QContentEngine *link  )
    : d( link )
{
}

/*!
  Destroys the content object.
 */
QContent::~QContent()
{
    // destruction of QSharedDataPointer d not required
}

/*!
    Returns true if a QContent is uninitialized; returns false otherwise.
*/
bool QContent::isNull() const
{
    return d == nullQContent()->d;
}

/*!
  Returns false if the Content link is invalid or if the backing file is unavailable, either.
  due to removal of media or deletion of the file.

  Note: the parameter \a force is retained for compatibility, but is ignored.
 */
bool QContent::isValid(bool force) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;
    else
        return d->isValid( force );
}

/*!
    Returns the file name of the file backing this content object.
    \sa setFile(), executableName(), name(), file()
*/
QString QContent::fileName() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->fileName();
}

/*!
  Returns the user-visible name for this content object
 */
QString QContent::name() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->translatedName();
}

/*!
    Returns an untranslated user visible name for this content object.
*/
QString QContent::untranslatedName() const
{
    return d->name();
}

/*!
  Returns the RFC2045 mime-type for the content.
 */
QString QContent::type() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->mimeType().id();
}

/*!
  Returns the plain-text size of the content in bytes
 */
qint64 QContent::size() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return 0;
    else
        return d->size( true );
}

/*!
  Returns the icon for this object.  If the object is an application, then it
  will be the application icon, otherwise it will be a generic icon for the
  mime-type of the object.
 */
QIcon QContent::icon() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QIcon();

    QIcon icon = d->icon();

    if( icon.isNull() )
        icon = d->mimeType().icon();

    return icon;
}

/*!
  Returns the icon for this object.  If the object is an application, then it
  will be the application icon, otherwise it will be a generic icon for the
  mime-type of the object.

  If the object is a DRM controlled,  a key emblem is super-imposed on the
  icon. If the content does not have current rights for \a permission the icon is shown
  grayed out (using the QIcon dynamic routines).
 */
QIcon QContent::icon( QDrmRights::Permission permission ) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QIcon();

    QIcon icon = d->icon();

    if( icon.isNull() )
    {
        if( d->drmState() == Protected )
            icon = d->mimeType().icon( (d->permissions( false ) & permission) == permission ? QMimeType::DrmValid : QMimeType::DrmInvalid );
        else
            icon = d->mimeType().icon();
    }

    return icon;
}


/*!
  Takes a QFileInfo reference \a fi to a content object, and installs the
  meta-data for it in the backing store.  The path must be an absolute path and
  the file must exist.  If the file is a .desktop file linking to an application
  or a document then link target is installed in the database.

  If MIME type data is not available, it is determined by file extension, or
  failing that, the magic number.

  If the object is a DRM-controlled file (i.e. it is a .dcf file, or is otherwise
  shown as DRM controlled) the DRM subsystem is queried for the DRM status.

  \sa QContentSet::scan()
 */
QContentId QContent::install( const QFileInfo &fi )
{
    if (!fi.exists())
    {
        qWarning() << "Attempt to install a file into the content database that doesn't exist:" << fi.fileName();
        return QContent::InvalidId;
    }
    return QContentStore::instance()->contentFromFileName(
            fi.absoluteFilePath(), QContentStore::Construct|QContentStore::Commit ).id();
}

/*!
  Removes the Content with \a id from the backing store.  This does not remove any files
  associated with the QContent, if the file is in a scanned location it will be reinstalled
  the next time that location is scanned.
 */
void QContent::uninstall( QContentId id )
{
    if(id == QContent::InvalidId)
    {
        qLog(DocAPI) << "Cannot uninstall QContent::InvalidId";
        return;
    }
    qLog(DocAPI) << "Uninstalling QContent for id " << id;
    QContentStore::instance()->uninstallContent( id );
}

/*!
  Installs a set of files in a \a batch, committing to the database only once. This achieves a significant
  performance boost over multiple calls to install.

  \sa install()
 */

void QContent::installBatch( const QList<QFileInfo> &batch )
{
    QList< QContent > content;
    foreach(const QFileInfo &fi, batch)
        content.append( QContentStore::instance()->contentFromFileName( fi.absoluteFilePath(), QContentStore::Construct ) );

    QContentStore::instance()->batchCommitContent( content );
}

/*!
  Uninstalls a set of files in a \a batch, committing to the database only once. This achieves a significant
  performance boost over multiple calls to uninstall().

  \sa uninstall()
 */

void QContent::uninstallBatch( const QList<QContentId> &batch )
{
    QContentStore::instance()->batchUninstallContent( batch );
}

/*!
  Clear all error flags and errors strings on all QContent objects.
  Note: this method clears the global error cache for all QContent objects
  in this process.
 */
void QContent::clearErrors()
{
}

/*!
  Given an application binary name \a bin return the QContentId of the QContent record
  for that application.  If \a bin is the fully qualified path of an ordinary file the
  QContentId of that file will be returned.

  If \a bin does not refer to any application or file in the backing store then an
  InvalidId will be returned.

  Note that binary names are unique across Qtopia.
 */
QContentId QContent::execToContent( const QString& bin )
{
    QContent content = QContentStore::instance()->contentFromFileName( bin, QContentStore::Lookup );

    if( !content.isNull() )
        return content.id();
    else
        return QContent::InvalidId;
}

/*!
  Returns a value of the DrmState enumeration indicating whether the content is DRM protected or not.
 */
QContent::DrmState QContent::drmState() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return Unprotected;
    else
        return d->drmState();
}

/*!
    \deprecated
  Returns the document status of the object as per the UsageMode enum.  This
  value does not usually change.  The status is used to determine what to display
  to the user. Data objects make no sense to display to the user, as
  they cannot be launched or categorized. These files are only of use to the
  applications which operate on them.

    \sa role()
 */
QContent::UsageMode QContent::usageMode() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return UnknownUsage;
    else
        return d->role();
}

/*!
  Returns the document status of the object as per the Role enum.  This
  value does not usually change.  The status is used to determine what to display
  to the user. \l Data objects make no sense to display to the user, as
  they cannot be launched or categorized. These files are only of use to the
  applications which operate on them.
 */
QContent::Role QContent::role() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return UnknownUsage;
    else
        return d->role();
}

/*!
    Sets the document status of the object to \a role as per the Role enum.
 */
void QContent::setRole( Role role )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setRole( role );
}


/*!
  Returns the comment for this object, typically used on ToolTips
  For DRM controlled objects this will include a summary of the
  rights and DRM status
 */
QString QContent::comment() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
    {
        QString comment = d->property( QLatin1String( "none" ), QLatin1String( "comment" ) );

        if( comment.isEmpty() )
            return QMimeType::fromId( type() ).description();
        else
            return comment;
    }
}

/*!
    Sets a string \a comment for this object.
 */
void QContent::setComment( const QString &comment )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setProperty( QLatin1String( "none" ), QLatin1String( "comment" ), comment );
}

/*!
  Returns the error string of this object.  If error() is false
  then this method returns an empty string.  Use error() instead
  of checking for an empty error string. The error string is not translated
  and therefore should not be used in a user interface.
 */
QString QContent::errorString() const
{
    return d->errorString();
}

/*!
  Returns true if this QContent object is in an error state.
  Call the errorString() method to return a text description of the error.
 */
bool QContent::error() const
{
    return !d->errorString().isEmpty();
}

/*!
    Returns all the DRM permissions the content currently has rights for.
    If \a force is true the rights will be re-queried. If \a force is false a
    cached value might be returned.
 */
QDrmRights::Permissions QContent::permissions( bool force ) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QDrmRights::InvalidPermission;
    else
        return d->permissions( force );
}

/*!
    Returns a QDrmRights representation of any rights for this object for the
    given \a permission.

    If drmStatus() returns Unprotected, then this method returns a QDrmRights
    object with valid rights for all permissions.  Use drmStatus() != Unprotected
    to test for unprotected content and permissions() & [permission] to test for
    a specific permission.

    \bold {Displaying the play permissions of a QContent:}
    \code
    void showPlayRights( const QContent &content, QWidget *parent )
    {
        QDrmRights = content.rights( QDrmRights::Play );

        QString title;

        switch( rights.status() )
        {
        case QDrmRights::Invalid:
            title = tr( "Play permissions invalid." );
            break;
        case QDrmRights::Valid:
            title = tr( "Play permissions valid." );
            break;
        case QDrmRights::ValidInFuture:
            title = tr( "Play permissions pending." );
            break;
        }

        QString message = title;

        foreach( QDrmRights::Constraint constraint, rights.constraints() )
        {
            message += '\n' + constraint.name() + ":\t" + constraint.value();

            for( int i = 0; i < constraint.attributeCount(); i++ )
                message += "\n\t" constraint.attributeName( i ) + ":\t" ) + constraint.attributeValue( i );

        }

        QMessageBox::information( parent, title, QString( "<qt>%1</qt>" ).arg( message );
    }
    \endcode
 */
QDrmRights QContent::rights( QDrmRights::Permission permission ) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QDrmRights();

        return d->rights( permission );
}

/*!
  Returns true if the QContent has been assigned a file name.  Calling file() when
  fileKnown() is false will generate a new file and assign its file name to the QContent.

  \sa file()
  \deprecated
 */
bool QContent::fileKnown() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;
    else
        return !d->fileName().isEmpty();
}

/*!
  Returns the path of the .desktop file the QContent was generated from or an empty string
  if the QContent was not generated from a .desktop file.

  \sa file(), linkFileKnown()
  \deprecated
 */
QString QContent::linkFile() const
{
    return d->property( QLatin1String( "dotdesktop" ), QLatin1String( "linkFile" ) );
}

/*!
  Returns the file name of the file associated with the QContent.  If no file is currently associated with the
  content this will reserve a new file name and so should be called with caution.  To determine if there is
  a file associated with the QContent call fileKnown().

  To get the file name with out potentially creating a new file use fileName() instead.

  \sa executableName(), name(), fileName(), fileKnown()
  \deprecated
 */
QString QContent::file() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();

    QString fileName = d->fileName();

    if( fileName.isEmpty() )
    {
        QIODevice *dev = QContentStore::instance()->openContent( const_cast< QContent * >( this ), QIODevice::WriteOnly );

        if( dev )
        {
            dev->close();

            delete dev;
        }

        fileName = d->fileName();
    }

    return fileName;
}

/*!
  Returns a DRM permission for each MIME type an application QContent can open.
  The launcher will ensure that a documents opened from the launcher menu have
  valid rights for their MIME type permission before launching the application.

  The permission list will contain one permission for each MIME type returned
  by mimeTypes().

  \sa mimeTypes()
 */
QList< QDrmRights::Permission > QContent::mimeTypePermissions() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QList<QDrmRights::Permission>();

    QList<QDrmRights::Permission> permissions;

    QList< QMimeEngineData > execTypes = d->executableMimeTypes();

    foreach( QMimeEngineData execType, execTypes )
        permissions.append( execType.permission );

    return permissions;
}

/*!
  Returns an icon path for each MIME type an application QContent an open.

  The icon list will contain one icon for each MIME type returned by mimeTypes().

  \sa mimeTypes()
 */
QStringList QContent::mimeTypeIcons() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QStringList();

    QStringList icons;

    QList< QMimeEngineData > execTypes = d->executableMimeTypes();

    foreach( QMimeEngineData execType, execTypes )
        icons.append( execType.icon );

    return icons;
}

/*!
  Returns a list of MIME types that an application QContent can view or edit.  If the QContent is not
  an application this will return an empty list.
 */
QStringList QContent::mimeTypes() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QStringList();

    QStringList mimeTypes;

    QList< QMimeEngineData > execTypes = d->executableMimeTypes();

    foreach( QMimeEngineData execType, execTypes )
        mimeTypes.append( execType.type );

    return mimeTypes;
}

/*!
  Sets the list of MIME types that the application can view or edit to \a mimeTypes.

  The application's icon will be associated with the mime types in document lists and
  the application will be unable to open DRM protected content from the launcher.

  MIME type associations are written to the database when commit() is called, if the
  QContent is not an application the associations will not be written to the database.
 */
void QContent::setMimeTypes( const QStringList &mimeTypes )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;
    d->setMimeTypes(mimeTypes, QStringList(), QList< QDrmRights::Permission >());
}

/*!
  Sets the list of MIME types that the application can view or edit to \a mimeTypes.

  The icon displayed each MIME type is obtained from the \a mimeTypeIcons list, and
  the DRM permission which is verified for documents opened from the launcher is obtained
  from the \a permissions list.

  Each mime type is assigned the icon and DRM permission from the corresponding index in the
  \a mimeTypeIcons and \a permissions lists.  If there is only one value in either list it is
  repeated for each MIME type.  If a list has more then one item but fewer than  \a mimeTypes
  then a null value is assigned for the remainder of the MIME types.

  MIME type associations are written to the database when commit() is called, if the
  QContent is not an application the associations will not be written to the database.

  \sa mimeTypeIcons(), mimeTypePermissions()
 */
void QContent::setMimeTypes( const QStringList &mimeTypes, const QStringList& mimeTypeIcons, const QList< QDrmRights::Permission >& permissions)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;
    d->setMimeTypes(mimeTypes, mimeTypeIcons, permissions);
}

/*!
  Returns a list of category IDs for the categories a QContent has been assigned.
 */
QStringList QContent::categories() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QStringList();

    return d->categories();
}

/*!
  Assignment operator. Assigns the data in \a other to this content object.
 */
QContent &QContent::operator=( const QContent &other )
{
    d = other.d;   // shallow QSharedData copy
    Q_ASSERT(d.constData() != NULL);
    return (*this);
}

/*!
  Equality operator.  Returns true if this QContent object is the same as
  the \a other; otherwise returns false.

  The values of the \l {QContent}s are only tested if both are not null and both
  have invalid IDs, otherwise only the IDs are used to determine equality.
 */
bool QContent::operator==( const QContent &other ) const
{
    if( d == other.d )
        return true;
    else if( other.d == nullQContent()->d )
        return false;
    else if( d->id() != QContent::InvalidId )
        return d->id() == other.d->id();
    else if( other.d->id() == QContent::InvalidId )
        return d->name() == other.d->name()
            && d->fileName() == other.d->fileName()
            && d->mimeType() == other.d->mimeType()
            && d->role() == other.d->role();
    else
        return false;
}

/*!
    Queries the launcher configuration settings and returns true if an application QContent
    is in the systems PreloadApps list; otherwise returns false.

    If the QContent is a document then the return value will indicate if the associated application
    is preloaded

    Preloaded applications are applications that are launched in the background when Qt Extended is started.
*/
bool QContent::isPreloaded() const
{
    // Preload information is stored in the Launcher config in v1.5.
    if (isDocument())
        return false;
    QSettings cfg("Trolltech","Launcher");
    cfg.beginGroup("AppLoading");
    QStringList apps = cfg.value("PreloadApps").toStringList();
    if (apps.contains(executableName()))
        return true;
    return false;
}

/*!
  Returns the name of the name of the application that will be launched if the QContent is executed.

  If the QContent is an application this is the name of the binary, if it is a document it is the
  executable name of the application associated with the document MIME type.

  If the content is not an application and there is no associated application associated with its
  MIME type then a null string will be returned.

  \sa execute()
 */
QString QContent::executableName() const
{
    if( role() == Application )
    {
        return d->fileName();
    }
    else
    {
        QContent app = d->mimeType().application();
        if ( app.id() != InvalidId )
            return app.executableName();
        else
            return QString();
    }
}

/*!
    Sets the executable name property to \a exec.  If the content is not an executable this will change the file name.

    The property will not be written to the backing store until commit() is called.

    Use setFile() instead.

    \deprecated
    \sa setFile()
 */
void QContent::setExecutableName(const QString &exec)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setFileName( exec );
}

/*!
  Executes the application associated with this QContent.

  \sa executableName()
 */
void QContent::execute() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;
    else
        d->execute( QStringList() );
}

/*!
  Executes the application associated with this QContent, with
  \a args as arguments.

  \sa executableName()
 */
void QContent::execute(const QStringList& args) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;
    else
        d->execute(args);
}

/*!
    Returns the path to the icon for this file.

    \sa icon()
 */
QString QContent::iconName() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->iconName();
}

/*!
    Sets the Name property to \a name.

    The property will not be written to the backing store until commit()
    is called.

    This does not rename the underlying file, to rename a file use rename() instead.

  \sa name(), rename()
 */
void QContent::setName( const QString& name )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setName( name );
}

/*!
    Sets the Type property to \a type.

    The property will not be written to the backing store until commit()
    is called.

  \sa name()
 */
void QContent::setType(const QString& type)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setMimeType( QMimeType( type ) );

}

/*!
    Sets the icons associated with this content to \a iconpath.

    The property will not be written to the backing store until commit()
    is called.

    \sa icon(), commit()
 */
void QContent::setIcon(const QString& iconpath)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setIconName( iconpath );
}

/*!
  Returns true if this QContent object represents a document, or false if it represents
  an application. Convenience function.

    \sa usageMode()
 */
bool QContent::isDocument() const
{
    return role() != Application;
}

/*!
    Returns a representive thumbnail of the content.  If \a size is not null the thumbnail will be resized to fit those dimensions
    according to the given aspect ratio \a mode.
*/
QImage QContent::thumbnail( const QSize &size, Qt::AspectRatioMode mode ) const
{
#ifndef QTOPIA_CONTENT_INSTALLER
    return QContentStore::instance()->thumbnail(*this, size, mode);
#else
    Q_UNUSED(size);
    Q_UNUSED(mode);

    return QImage();
#endif
}

/*!
    \enum QContent::Property

    Convenience enumeration that provides some of the more common content properties. All
    the properties have a key equal to their enumeration name in the default property group
    \c {none}.

    \value Album  The name of the album which the content is part of.
    \value Artist The name of the artist who produced the content.
    \value Author The author of the content.
    \value Composer The composer of the content.
    \value ContentUrl A URL from which a copy of the content can be downloaded.  Typically associated with OMA DRM content.
    \value Copyright A copyright notice for the content.
    \value CopyrightUrl A URL with copyright/legal information.
    \value Description A description of the content.
    \value Genre The genre the content belongs to.
    \value InformationUrl A URL where additional information about the content can be obtained.  Typically associated with OMA DRM content.
    \value PublisherUrl The URL for the content publisher's website.
    \value RightsIssuerUrl A URL where rights for DRM protected content may be obtained.  Typically associated with OMA DRM content.
    \value Track The content's position on an album.
    \value Version The content's version number.
    \value Title The title of the content.
 */

/*!
    Sets the \a key property of a QContent to \a value.  A key is unique within a property
    \a {group}, but may be repeated in another group.  If no \a group is specified then the
    key is assumed to belong to the default property group \c {none}.

    The property value will not be written to the backing store until commit() is called.

    \sa commit()
 */
void QContent::setProperty(const QString& key, const QString& value, const QString &group)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setProperty( !group.isEmpty() ? group : QLatin1String( "none" ), key, value );
}

/*!
    Sets the \a key property of a QContent to \a value.

    The property will not be written to the backing store until commit() is called.

    \sa commit()
 */
void QContent::setProperty( Property key, const QString &value )
{
    setProperty( propertyKey( key ), value );
}

/*!
    Returns the value of the \a key property within a property \a group.

    Property groups provide a namespace for keys. Within a group a QContent may
    only have one value for a key, but a key may be repeated in another group
    with a different value.  If no \a group is specified then the default group
    \c none is assumed.
 */
QString QContent::property(const QString& key, const QString &group) const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->property( !group.isEmpty() ? group : QLatin1String( "none" ), key );
}

/*!
    Returns the value of the \a key property.
 */
QString QContent::property( Property key ) const
{
    return property( propertyKey( key ) );
}

/*!
    Returns the string key for a given \a property.
 */
QString QContent::propertyKey( Property property )
{
    switch( property )
    {
    case Album:           return QLatin1String( "Album"           );
    case Artist:          return QLatin1String( "Artist"          );
    case Author:          return QLatin1String( "Author"          );
    case Composer:        return QLatin1String( "Composer"        );
    case ContentUrl:      return QLatin1String( "ContentUrl"      );
    case Copyright:       return QLatin1String( "Copyright"       );
    case CopyrightUrl:    return QLatin1String( "CopyrightUrl"    );
    case Description:     return QLatin1String( "Description"     );
    case Genre:           return QLatin1String( "Genre"           );
    case InformationUrl:  return QLatin1String( "InformationUrl"  );
    case PublisherUrl:    return QLatin1String( "PublisherUrl"    );
    case RightsIssuerUrl: return QLatin1String( "RightsIssuerUrl" );
    case Track:           return QLatin1String( "Track"           );
    case Version:         return QLatin1String( "Version"         );
    case Title:           return QLatin1String( "Title"           );
    default:
         return QLatin1String( "Unknown" );
    }
}

/*!
    Sets the list of category IDs assigned to a QContent to \a categoryList.

    This will overwrite the list of categories already assigned to a QContent, so new
    categories should be appended to the list returned by categories() to prevent existing
    categories being lost.

    \code
        content.setCategories( content.categories() << newCategoryId );
    \endcode

    The categories will not be written to the backing store until commit()
    is called.

    \sa categories(), commit()
 */
void QContent::setCategories( const QStringList &categoryList )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setCategories( categoryList );
}
/*!
  \fn bool QContent::commit()

   Writes the changes to the QContent object to the backing store. Returns true if successful

   When a QContent is committed a notification is sent to all applications which can be listened
   for on the QtopiaApplication::contentChanged() signal.
*/


/*!
    Writes the changes to the QContent to the backing store. Returns true if successful and sets
    \a change to the type of change committed.
 */
bool QContent::commit(ChangeType &change)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;

    change = id() == InvalidId ? Added : Updated;

    return QContentStore::instance()->commitContent( this );
}

/*!
    Sets the file that this content references to \a {filename}.

    This does not move the contents of the file, to move a file use \l moveTo() instead.

    \sa moveTo()
 */
void QContent::setFile( const QString& filename )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    d->setFileName( filename );
}

/*!
    Uninstalls a QContent from the database and removes the associated file from the file-system.
 */
void QContent::removeFiles()
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return;

    QContentStore::instance()->removeContent( this );

}

/*!
    \deprecated
    Uninstall the .desktop/link file for this object from the database, and remove it from the file-system.
 */
void QContent::removeLinkFile()
{
}

/*!
    \deprecated
    Set the link/.desktop file that this content references to \a filename.
 */
void QContent::setLinkFile( const QString& filename )
{
    d->setProperty( QLatin1String( "dotdesktop" ), QLatin1String( "linkFile" ), filename );
}

/*!
    Returns the root path of the media the content is stored on.
 */
QString QContent::media() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QString();
    else
        return d->media();
}

/*!
    Sets the root path of the \a media the file is stored on.  Once a QContent has been committed to the database
    the media cannot be changed. Returns true if successful; otherwise false.
 */
bool QContent::setMedia( const QString &media )
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;
    else
    {
        d->setMedia( media );

        return true;
    }
}

/*!
    Returns the ID of this QContent.  If this QContent is not present in
    the backing store its value will be QContent::InvalidId.
 */
QContentId QContent::id() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QContent::InvalidId;
    else
        return d->id();
}

/*!
    Opens a QIODevice using mode \a mode.

    Returns the QIODevice if successful, otherwise returns 0.  It is the
    caller's responsibility to delete the return value.
 */
QIODevice *QContent::open(QIODevice::OpenMode mode)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return NULL;

    if( d->fileName().isEmpty() )
        return QContentStore::instance()->openContent( this, mode );
    else
        return d->open( mode );
}

/*!
    Opens a read only QIODevice.

    Returns the QIODevice if successful, otherwise returns 0.  It is the
    caller's responsibility to delete the return value.
 */
QIODevice *QContent::open() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return NULL;

    return const_cast< QContent * >( this )->d->open( QIODevice::ReadOnly );
}

/*!
    Saves \a data to the file associated with a QContent.

    If the QContent is associated with an existing file its contents will be overwritten,
    otherwise a new file will be created.  In order to create a new file the QContent must
    have valid values of name() and type().

    Returns true if all the data is written to the file and false otherwise.

    \bold {Saving data to a new file:}
    \code
    bool saveData( const QString &name, const QString &type, const QByteArray &data )
    {
        QContent content;

        content.setName( name );
        content.setType( type );

        if( content.save( data ) )
        {
            content.commit();
            return true;
        }
        else
        {
            return false;
        }
    }
    \endcode
 */
bool QContent::save(const QByteArray &data)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;

    QIODevice *dev = open(QIODevice::WriteOnly);
    if (!dev)
        return false;

    int bytesWritten = dev->write(data);
    dev->close();
    delete dev;

    if (bytesWritten != data.size()) {
        return false;
    }

    return true;
}

/*!
    Read the unformatted contents of the file associated with a QContent into \a data.

    If \a data is not empty the contents will be overridden.

    Returns true if data could be read from the file and false otherwise.
 */
bool QContent::load(QByteArray &data) const
{
    QIODevice *dev = open();
    if (!dev)
        return false;

    if (dev->size() > 0)
        data = dev->readAll();
    dev->close();
    delete dev;

    return true;
}

/*!
    Copies the contents of \a from to this QContent.

    Returns true is successful, otherwise false.
 */
bool QContent::copyContent(const QContent &from)
{
    QFile source( from.fileName() );

    if( source.copy( fileName() ) )
        return true;

    return false;
}

/*!
    Copies the contents of the file and the meta-info from this QContent to
    \a newPath.

    Returns true is successful, otherwise false.

    \sa moveTo()
 */

bool QContent::copyTo(const QString &newPath)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;

    return QContentStore::instance()->copyContentTo( this, newPath );
}

/*!
    Moves the contents of the file and the meta-info from this QContent to
    \a newPath.

    Returns true if the contents is successfully moved, otherwise false.

    Note: The id() of the original file will be invalid and should not be used.

    \sa copyTo(), rename()
 */

bool QContent::moveTo(const QString &newPath)
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return false;

    return QContentStore::instance()->moveContentTo( this, newPath );
}


/*!
    Sets the name of the content to \a name and renames the associated file to match.

    Returns true if the file could be renamed and false otherwise.

    \sa moveTo(), setName()
*/
bool QContent::rename(const QString &name)
{
    return QContentStore::instance()->renameContent(this, name);
}

/*!
  Updates all the QContentSets this content is a member of.
 */
void QContent::updateSets(QContentId id, QContent::ChangeType c)
{
#ifdef QTOPIA_CONTENT_INSTALLER
    Q_UNUSED(id);
    Q_UNUSED(c);
#else
    if(id != InvalidId)
        QContentUpdateManager::instance()->addUpdated(id, c);
#endif
}

/*!
  Returns the value of the last time underlying file was updated.
 */
QDateTime QContent::lastUpdated() const
{
    Q_ASSERT( d.constData() != NULL );
    if( d.constData() == NULL )
        return QDateTime();
    else
        return d->lastUpdated();
}

/*!
    Returns true if the QContent is detached; otherwise returns false.

    \internal
*/
bool QContent::isDetached() const
{
    return d->ref == 1;
}

/*!
    \enum QContent::DocumentSystemConnection

    Identifies how an application is connected to the Qt Extended Document System.

    \value DocumentSystemClient The application performs document system operations by connecting to the Qt Extended Document Server.
    \value DocumentSystemDirect The application connects directly to the backing store and performs all document system operations locally.
*/

/*!
    Identifies whether the application connects to the Qt Extended Document Server or performs Document System operations locally.
*/
QContent::DocumentSystemConnection QContent::documentSystemConnection()
{
    return *::documentSystemConnection();
}

/*!
    Sets the type of \a connection the application has to the Qt Extended Document System.
    Returns true if the connection type is set, false otherwise.

    Notes:
    \list
    \o The connection type cannot be changed once connected to the Document System.
    \o Applications with the GuiServer type always have a direct connection.
    \o This function has no effect before QApplication is constructed.
    \endlist

    \sa QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION()
*/
bool QContent::setDocumentSystemConnection( DocumentSystemConnection connection )
{
    if ( !qApp ) {
        qWarning() << "QContent::setDocumentSystemConnection() has no effect before QApplication is constructed.";
        return false;
    }
    if ( QContentStore::initialized() )
        qWarning() << "QContent::setDocumentSystemConnection() cannot be called because the document system is already connected.";
    else if ( qApp->type() != QApplication::GuiServer )
        *::documentSystemConnection() = connection;
    return ( *::documentSystemConnection() == connection );
}


/*!
    \fn QContent::serialize(Stream &stream) const

    Writes the contents of a QContent to a \a stream.

    \internal
*/
template <typename Stream> void QContent::serialize(Stream &stream) const
{
    stream << d->engineType();
    stream << *d;
}

/*!
    \fn QContent::deserialize(Stream &stream)

    Reads the contents of a QContent from \a stream.

    \internal
*/
template <typename Stream> void QContent::deserialize(Stream &stream)
{
    QString engineType;

    stream >> engineType;

    *this = QContentStore::instance()->contentFromEngineType( engineType );

    stream >> *d;
}

QDataStream& operator<<(QDataStream& ds, const QContentId &id)
{
    ds << id.first << id.second;
    return ds;
}

QDataStream& operator>>(QDataStream& ds, QContentId &id)
{
    ds >> id.first >> id.second;
    return ds;
}

QTOPIA_EXPORT QDebug operator<<(QDebug debug, const QContent &content)
{
    if( content.isNull() )
        debug << "QContent()";
    else
        debug << "QContent(" << (*content.d) << ")";

    return debug;
}

/*!
  Return a uint suitable for use as a hash value.  This allows QContentId
  to be stored in a QHash
*/
uint qHash(const QContentId &id)
{
    int n;
    uint h = 0;
    uint g = 0;

    const uchar *p = reinterpret_cast<const uchar *>(&id.first);
    n = sizeof(id.first);
    while (n--) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }

    const uchar *q = reinterpret_cast<const uchar *>(&id.second);
    n = sizeof(id.second);
    while (n--) {
        h = (h << 4) + *q++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }

    return h;
}
