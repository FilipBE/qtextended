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

#include "qcontentplugin.h"
#include "contentpluginmanager_p.h"
#include "drmcontent_p.h"
#include <QtDebug>
#include <qcategorymanager.h>

/*!
    \class QContentPlugin
    \inpublicgroup QtBaseModule

    \brief The QContentPlugin class provides an interface for the Qt Extended Document System to discover detailed information about a file.

    Content plug-ins are used by the document system to read meta-data from files that may be used to index the content or better
    describe it to the user.  When a new file is first discovered by the document system its extension is used to identify
    possible plug-ins that may be able to identify the file. If any such plug-in is found the plug-in's installContent() method
    is called with the file name and the destination QContent as arguments.  The document system will keep trying potential
    content plug-ins until one is found that identifies itself as having successfully processed the file by returning true.

    The content plug-ins are also invoked when a content record is found to be out of date, but in this case the updateContent()
    method of the plug-in is called.  A content record is determined to be out of date when the backing file's last modified date
    is more recent than the content records last update date.

  \ingroup content
  \ingroup plugins
*/

/*!
    Destroys a QContentPlugin.
*/
QContentPlugin::~QContentPlugin()
{
}

/*!
    \fn QContentPlugin::keys() const

    Returns a list of the file extensions the QContentPlugin instance can report on.
*/

/*!
    \fn QContentPlugin::installContent( const QString &fileName, QContent *content )

    Populates \a content with data from the file with the file name \a fileName.  Returns true if the content
    plug-in successfully populated the QContent.

    Installation is only performed when the content is first identified by the content system, if the file changes
    after installation updateContent() will be called to ensure the content data is up to date; otherwise returns
    false
*/

/*!
    Refreshes the content data of \a content following a change to the file it references.

    Returns true if the content data has been ensured to be up to date; otherwise returns false.
*/
bool QContentPlugin::updateContent( QContent *content )
{
    Q_UNUSED( content );

    return false;
}

/*!
    Initializes the content plug-in manager.

    This should not normally not be necessary as the plug-in manager is loaded on demand.
*/
void QContentPlugin::preloadPlugins()
{
    QContentFactory::loadPlugins();
}

/*!
    \class QContentPropertiesPlugin
    \inpublicgroup QtBaseModule

    \brief The QContentPropertiesPlugin class provides an interface for accessing properties and thumbnails embedded in content.

    The QContent class uses QContentPropertiesPlugin to provide thumbnails of content.

    The QContentProperties class uses QContentPropertiesPlugin to create instances of
    QContentPropertiesEngine for accessing properties embedded in content.

    A plug-in which implements the QContentPropertiesPlugin interface should typically also
    implement the QContentPlugin interface to ensure content is assigned the correct type and so
    that any searchable properties are written to the database.

    \sa QContent, QContentProperties, QContentPropertiesEngine

    \ingroup content
    \ingroup plugins
*/

/*!
    Destroys a QContentPropertiesPlugin.
*/
QContentPropertiesPlugin::~QContentPropertiesPlugin()
{
}

/*!
    \fn QContentPropertiesPlugin::mimeTypes() const

    Returns a list of the mime types a content properties plug-in supports.
*/

/*!
    \fn QContentPropertiesPlugin::thumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode )

    Returns a thumbnail representive of \a content.  If \a size is not null the thumbnail will be resized to fit those dimensions
    according to the given aspect ratio \a mode.

    \sa QContent::thumbnail()
*/

/*!
    \fn QContentPropertiesPlugin::createPropertiesEngine( const QContent &content )

    Returns a new instance of a QContentPropertiesEngine for viewing and editing the meta-data embedded in \a content.

    If the plug-in is unable to read \a content then a null pointer will be returned instead.

    \sa QContentProperties
*/
