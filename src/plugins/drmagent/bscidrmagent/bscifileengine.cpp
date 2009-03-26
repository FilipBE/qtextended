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


#include "bscifileengine.h"
#include "bscidrm.h"
#include "bscidrmcontentplugin.h"
#include <QtDebug>
#include <qtopianamespace.h>

BSciDrmFileEngine::BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath )
    : QFSFileEngine( filePath )
    , m_plugin( plugin )
    , m_filePath( filePath )
    , m_io( 0 )
{
}

BSciDrmFileEngine::BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath, const QString &dcfPath, const QString &baseName )
    : QFSFileEngine( dcfPath )
    , m_plugin( plugin )
    , m_filePath( filePath )
    , m_baseName( baseName )
    , m_io( 0 )
{
}

BSciDrmFileEngine::~BSciDrmFileEngine()
{
    if( m_io )
        delete m_io;
}

/*!
    \reimp
*/
bool BSciDrmFileEngine::copy( const QString &newName )
{
    close();

    bool error = false;
    if(!QFSFileEngine::open(QFile::ReadOnly)) {
        QString errorMessage = QLatin1String("Cannot open %1 for input");
        setError(QFile::CopyError, errorMessage.arg(fileName(DefaultName)));
        error = true;
    } else {
        QString fn = Qtopia::tempName(newName);
        QFile out( fn );
        if(!out.open( QIODevice::WriteOnly )) {
            close();
            setError(QFile::CopyError, QLatin1String("Cannot open for output"));
            error = true;
        } else {
            char block[1024];
            while(true) {
                qint64 in = QFSFileEngine::read(block, 1024);
                if(in <= 0)
                    break;
                if(in != out.write(block, in)) {
                    setError(QFile::CopyError, QLatin1String("Failure to write block"));
                    error = true;
                    break;
                }
            }
            QFSFileEngine::close();
            out.close();

            if( error || !out.rename(newName) ) {
                out.remove();
                QString errorMessage = QLatin1String("Cannot create %1 for output");
                setError(QFile::CopyError, errorMessage.arg(newName));
                error = true;
            }
        }
    }
    if(!error) {
        int flags = QFSFileEngine::fileFlags(FileInfoAll) & PermsMask;
        QFile::setPermissions(newName, QFile::Permissions( flags ) );
        return true;
    }
    return false;
}


bool BSciDrmFileEngine::close()
{
    if( m_io )
    {
        m_io->close();

        m_io = 0;

        return true;
    }
    else
        return false;
}

/*!
    \reimp
*/
QString BSciDrmFileEngine::fileName( FileName file ) const
{
    if( baseName().isEmpty() )
        return QFSFileEngine::fileName( file );

    switch( file )
    {
    case DefaultName:
        return defaultName();
    case BaseName:
        return baseName();
    case PathName:
        return QFSFileEngine::fileName( DefaultName );
    case AbsoluteName:
        return QFSFileEngine::fileName( AbsoluteName ) + QDir::separator() + baseName();
    case AbsolutePathName:
        return QFSFileEngine::fileName( AbsoluteName );
    case LinkName:
        return isLink() ? QFSFileEngine::fileName( AbsoluteName ) + QDir::separator() + contentId() : QString();
    case CanonicalName:
        QFSFileEngine::fileName( CanonicalName ) + QDir::separator() + contentId();
    case CanonicalPathName:
        return QFSFileEngine::fileName( CanonicalName );
    default:
        return QString();
    }
}

/*!
    Requests that a list of all the files matching the \a filters
    list based on the \a filterNames in the file engine's directory
    are returned.

    Returns an empty list if the file engine refers to a file
    rather than a directory, or if the directory is unreadable or does
    not exist or if nothing matches the specifications.
*/
QStringList BSciDrmFileEngine::entryList( QDir::Filters filters, const QStringList &filterNames ) const
{
    QStringList entries = QFSFileEngine::entryList( filters, filterNames );

    if( filters & QDir::Files && isMultipart() )
    {
        QStringList locations = contentIds();

        foreach( QString filter, filterNames )
            locations = locations.filter( QRegExp( filter, Qt::CaseSensitive, QRegExp::Wildcard ) );

        entries += locations;
    }

    return entries;
}

/*!
    Returns the set of OR'd flags that are true
    for the file engine's file, and that are in the \a type's OR'd
    members.
*/
QAbstractFileEngine::FileFlags BSciDrmFileEngine::fileFlags( FileFlags type ) const
{
    FileFlags flags = QFSFileEngine::fileFlags( type );

    if( type & DirectoryType && isMultipart() )
    {
        flags |= DirectoryType;
        flags &= ~FileType;
    }

    if( type & LinkType && isLink() )
        flags |= LinkType;

    return flags;
}

/*!
    \reimp
*/
bool BSciDrmFileEngine::mkdir( const QString &dirName, bool createParentDirectories ) const
{
    Q_UNUSED( dirName );
    Q_UNUSED( createParentDirectories );

    return false;
}

/*!
    \reimp
*/
bool BSciDrmFileEngine::setPermissions( uint permissions )
{
    Q_UNUSED( permissions );

    return false;
}

/*!
    \reimp
*/
bool BSciDrmFileEngine::setSize ( qint64 size )
{
    Q_UNUSED( size );

    return false;
}

/*!
    \reimp
*/
qint64 BSciDrmFileEngine::write( const char *data, qint64 len )
{
    Q_UNUSED( data );
    Q_UNUSED( len );

    return -1;
}

bool BSciDrmFileEngine::open( QIODevice::OpenMode mode )
{
    if( !m_io && !(mode & QIODevice::WriteOnly) )
    {
        QDrmContentLicense *license = m_plugin->license( m_filePath );

        m_io = license ? new BSciReadDevice( m_filePath, license->permission() ) : 0;

        if( m_io )
        {
            if( m_io->open( mode | QIODevice::Unbuffered ) )
                return true;

            delete m_io;

            m_io = 0;
        }
    }

    return false;
}

qint64 BSciDrmFileEngine::pos() const
{
    return m_io ? m_io->pos() : -1;
}

qint64 BSciDrmFileEngine::read( char * data, qint64 maxlen )
{
    return m_io ? m_io->read( data, maxlen ) : -1;
}

bool BSciDrmFileEngine::seek( qint64 offset )
{
    return m_io ? m_io->seek( offset ) : false;
}

qint64 BSciDrmFileEngine::size() const
{
    return m_io ? m_io->size() : QFSFileEngine::size();
}

QString BSciDrmFileEngine::baseName() const
{
    return m_baseName;
}

QString BSciDrmFileEngine::defaultName() const
{
    return m_filePath;
}

QString BSciDrmFileEngine::contentId() const
{
    return m_baseName.startsWith( QLatin1String( "cid:" ) )
        ? m_baseName
        : QLatin1String( "OBJECT01" );
}

QStringList BSciDrmFileEngine::contentIds() const
{
    QStringList contentIds;

    SBSciFileInfo info;

    memset( &info, 0, sizeof(info) );

    if( BSCIGetFileInfo( BSciDrm::context, m_filePath.toLocal8Bit().constData(), 0, &info )  != BSCI_NO_ERROR )
    {
        for( uint i = 1; i <= info.numObjects; i++ )
            contentIds.append( QString( "OBJECT%1" ).arg( i, 2, 10, QLatin1Char( '0' ) ) );
    }

    BSCIReleaseFileInfo( BSciDrm::context, &info );

    return contentIds;
}

bool BSciDrmFileEngine::isMultipart() const
{
    return m_baseName.isEmpty() && BSCIGetFileInfo( BSciDrm::context, BSciDrm::formatPath( m_filePath ), 0, 0 ) > 1;
}

bool BSciDrmFileEngine::isLink() const
{
    return !m_baseName.startsWith( QLatin1String( "cid:" ) );
}
