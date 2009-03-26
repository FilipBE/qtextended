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

#include "qstoragedeviceselector.h"

#include <qcontent.h>
#include <qcategoryselector.h>
#include <qtopiaipcenvelope.h>
#include <qstorage.h>
#include <qtopianamespace.h>

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <stdlib.h>

/*!
  \class QStorageDeviceSelector
    \inpublicgroup QtBaseModule

  \brief The QStorageDeviceSelector class displays a list of available storage
         locations.

  \ingroup io
  \sa QDocumentPropertiesDialog
 */


class QStorageDeviceSelectorPrivate
{
public:
    QStorageDeviceSelectorPrivate() : fileSize(0), listEmpty(true) {}
    QString originalPath;
    int fileSize;
    bool listEmpty;
};

/*!
  Constructs a QStorageDeviceSelector with parent \a parent.
 */
QStorageDeviceSelector::QStorageDeviceSelector( QWidget *parent )
    : QComboBox( parent ), filter( 0 )
{
    storage = QStorageMetaInfo::instance();
    d = new QStorageDeviceSelectorPrivate;
    setLocation( QContent() );
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

/*!
  Constructs a QStorageDeviceSelector with parent \a parent.
  \a lnk is pointer to an existing QContent.
 */
QStorageDeviceSelector::QStorageDeviceSelector( const QContent & lnk, QWidget *parent )
    : QComboBox( parent ), filter( 0 )
{
    storage = QStorageMetaInfo::instance();
    d = new QStorageDeviceSelectorPrivate;
    setLocation(lnk);
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

/*!
  Destroys the widget.
 */
QStorageDeviceSelector::~QStorageDeviceSelector()
{
    if ( filter ) {
        delete filter;
        filter = 0;
    }
    delete d;
}

/*!
  Sets the display of the QStorageDeviceSelector to the location associated with the
  QContent \a lnk.
 */
void QStorageDeviceSelector::setLocation( const QContent &lnk )
{
    if ( !lnk.isNull() ) {
        d->fileSize = lnk.size();
    } else {
        d->fileSize = 0;
    }

    const QFileSystem *fs =  storage->fileSystemOf( lnk.fileName() );
    d->originalPath = fs ? fs->path() : QString();

    setupCombo();

    int currentLocation = -1;
    if ( !lnk.isNull()) {
        int n = locations.count();
        for ( int i = 0; i < n; i++ ) {
            if ( lnk.fileName().contains( locations[i] ) )
                currentLocation = i;
        }
    }
    if ( currentLocation == -1 )
        currentLocation = 0; //default to the first one

    setCurrentIndex( currentLocation );
}

/*!
  \internal
*/
void QStorageDeviceSelector::setupCombo()
{
    clear();
    locations.clear();

    QString s;
    int index = 0;
    foreach ( QFileSystem *fs, storage->fileSystems(filter) ) {
        // we add 10k to the file size so we are sure we can also save the desktop file
        if ( (!d->fileSize || fs->path() == d->originalPath || fs->availBlocks() * fs->blockSize() > d->fileSize + 10000 )
                      && fs->documents())
        {
            insertItem( index, fs->name() );
            locations.append( fs->path() );
            index++;
        }
    }

    d->listEmpty = locations.count() == 0;
    if ( d->listEmpty ) {
        insertItem( 0, tr("No filesystem available!") );
        locations.append( "" );
    }
}

/*!
  \internal
 */
void QStorageDeviceSelector::updatePaths()
{
    QString oldPath = locations[currentIndex()];

    setupCombo();

    int currentLocation = 0;
    int n = locations.count();
    for ( int i = 0; i < n; i++ ) {
        if ( oldPath == locations[i] ) {
            currentLocation = i;
        }
    }
    setCurrentIndex( currentLocation );
    if ( locations[currentIndex()] != oldPath )
        emit newPath();
}

/*!
  Returns true indicating the location displayed has been changed via
  the QStorageDeviceSelector.  It is most useful when the QStorageDeviceSelector is part of a
  dialog. That is, when the dialog is accepted with accept(), QStorageDeviceSelector::isChanged() can be
  examined to check for a change of location.
 */
bool QStorageDeviceSelector::isChanged() const
{
    if ( const QFileSystem *fs = storage->fileSystemOf(locations[currentIndex()]) )
        return fs->path() != d->originalPath;

    return true;
}

/*!
  Returns the default (home) location for the file associated with this
  QStorageDeviceSelector.
 */
QString QStorageDeviceSelector::installationPath() const
{
    return locations[ currentIndex() ]+"/";
}

/*!
  Returns the document path, \i{\<path\>/Documents} associated with this QStorageDeviceSelector.
 */
QString QStorageDeviceSelector::documentPath() const
{
    const QFileSystem *fs = storage->fileSystemOf(locations[currentIndex()]);
    if ( fs )
        return fs->documentsPath() + "/";
    else
        return locations[ currentIndex() ]+"/";
}

/*!
  Returns a pointer to QFileSystem object, associated with the current
  selection of the QStorageDeviceSelector.
 */
const QFileSystem *QStorageDeviceSelector::fileSystem() const
{
    if ( d->listEmpty )
        return 0;
    return storage->fileSystemOf( locations[ currentIndex() ] );
}

/*!
  \fn QStorageDeviceSelector::newPath()
  Emitted when the QStorageDeviceSelector changes to a new location.
 */


/*!
  QStorageDeviceSelector only shows those filesystems which pass the conditions of \a fsf.
  */
void QStorageDeviceSelector::setFilter( QFileSystemFilter *fsf )
{
    if ( filter )
        delete filter;
    filter = fsf;
    updatePaths();
}

/*!
  Sets the display of the QStorageDeviceSelector to the location containing
  \a path.
 */
void QStorageDeviceSelector::setLocation( const QString& path )
{
    d->fileSize = 0;
    const QFileSystem *fs =  storage->fileSystemOf( path );
    d->originalPath = fs ? fs->path() : QString();

    setupCombo();

    int currentLocation = -1;
    if ( !path.isEmpty() ) {
        int n = locations.count();
        for ( int i = 0; i < n; i++ ) {
            if ( path.contains( locations[i] ) )
                currentLocation = i;
        }
    }
    if ( currentLocation == -1 )
        currentLocation = 0; //default to the first one

    setCurrentIndex( currentLocation );
}

