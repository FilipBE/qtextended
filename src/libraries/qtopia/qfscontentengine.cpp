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
#include "qfscontentengine_p.h"
#include <QtDebug>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QFileSystem>
#include "qcontentstore_p.h"

/*!
    \class QFSContentEngine
    \inpublicgroup QtBaseModule

    \brief QFSContentEngine is the default content for non DRM protected files.

    \internal
*/

/*!
    Constructs a new unpopulated QFSContentEngine.
*/
QFSContentEngine::QFSContentEngine()
    : QContentEngine( QLatin1String( "*/*" ) )
{
}

/*!
    Constructs a new QFSContentEngine with the engine mime type \a engineType.
*/
QFSContentEngine::QFSContentEngine( const QString &engineType )
    : QContentEngine( engineType )
{
}

/*!
    Destroys a QFSContentEngine.
*/
QFSContentEngine::~QFSContentEngine()
{
}

/*!
    \reimp
*/
QContentEngine *QFSContentEngine::copyTo( const QString &newPath )
{
    if( fileName().isEmpty() )
    {
        setError( "Empty source path" );

        return 0;
    }

    QFile source( fileName() );

    if( source.copy( newPath ) )
    {
        QFSContentEngine *copy = new QFSContentEngine;

        copy->copy( *this );

        copy->setFileName( newPath );

        return copy;
    }

    setError( source.errorString() );

    return 0;
}

/*!
    \reimp
*/
bool QFSContentEngine::moveTo( const QString &newPath )
{
    if( fileName().isEmpty() )
    {
        setError( "Empty source path" );

        return false;
    }

    QString oldName = fileName();

    QFile source( oldName );

    if( source.copy( newPath ) )
    {
        if( source.remove() )
        {
            setFileName( newPath );

            return true;
        }

        QFile::remove( newPath );
    }

    setError( source.errorString() );

    return false;
}

/*!
    \reimp
*/
bool QFSContentEngine::rename(const QString &name)
{
    if (fileName().isEmpty()) {
        setError("Empty source path");

        return false;
    }

    QString extension = determineExtension(name, mimeType());
    QString baseName = determineBaseName(name, extension);

    QString newPath = newFilePath(baseName, extension, QFileInfo(fileName()).absoluteDir());

    QFile file(fileName());

    if (file.rename(newPath)) {
        setFileName(newPath);
        setName(baseName);

        return true;
    } else {
        setError(file.errorString());

        return false;
    }
}

/*!
    \reimp
*/
bool QFSContentEngine::execute( const QStringList &arguments ) const
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if( role() == QContent::Application )
    {
        qLog(DocAPI) << "QFSContentEngine::execute" << Qtopia::dehyphenate(name()) << fileName() << arguments;

        Qtopia::execute( fileName(), arguments.count() ? arguments[0] : QString() );

        return true;
    }
    else
    {
        QContent app = mimeType().application();

        if ( app.isValid() )
        {
            app.execute( QStringList() << arguments << fileName() );

            return true;
        }
}
#else
    Q_UNUSED(arguments)
#endif

    return false;
}

/*!
    \reimp
*/
QIODevice *QFSContentEngine::open( QIODevice::OpenMode mode )
{
    if( name().isEmpty() )
        return 0;

    if( fileName().isEmpty() )
    {
        if( mode & QIODevice::WriteOnly )
        {
            if( mimeType().isNull() )
                setMimeType( QMimeType::fromFileName( name() ) );

            QString media = this->media();

            QFileSystem fs = !media.isNull()
                    ? QFileSystem::fromFileName( media )
                    : QFileSystem::documentsFileSystem();

            QDir dir = determineDirectory(mimeType(), fs.documentsPath());
            QString extension = determineExtension(name(), mimeType());
            QString baseName = determineBaseName(name(), extension);

            setName(baseName);
            setFileName(newFilePath(baseName, extension, dir));
        }
        else
            return 0;
    }

    QFile *file = new QFile( fileName() );

    if( !file->open( mode ) )
    {
        setError( file->errorString() );

        delete file;

        file = 0;
    }

    return file;
}

/*!
    \reimp
*/
QContentEngine *QFSContentEngine::createCopy() const
{
    QFSContentEngine *engine = new QFSContentEngine;

    engine->copy( *this );
    engine->setId( id() );

    return engine;
}

bool QFSContentEngine::isOutOfDate() const
{
    return lastUpdated().isNull() || lastUpdated() != QFileInfo( fileName() ).lastModified();
}

/*!
    \reimp
*/
qint64 QFSContentEngine::querySize()
{
    return QFile( fileName() ).size();
}

/*!
    \reimp
*/
bool QFSContentEngine::queryValidity()
{
    return !fileName().isEmpty() && ( role() == QContent::Application || QFile::exists( fileName() ) );
}

/*!
    Constructs a new file name fo the content with a user visible \a name and mime \a type in the
    given \a directory.
*/
QString QFSContentEngine::newFilePath(const QString &name, const QString &extension, const QDir &directory)
{
    QString safeName;

    foreach (QChar c, name)
        safeName += (c.isLetterOrNumber() || c == QLatin1Char('.')) ? c : QLatin1Char('_');

    QString possibleName = safeName + extension;

    int number = 0;

    while (directory.exists(possibleName))
        possibleName = safeName + QLatin1Char('_') + QString::number(number++) + extension;

    return directory.filePath(possibleName);
}

/*!
    Determines the approriate directory within a \a location path for saving content of a given \a {mimeType}.
*/
QDir QFSContentEngine::determineDirectory(const QMimeType &mimeType, const QString &location)
{
    QDir dir(location + QLatin1Char('/') + mimeType.id());

    if (!dir.exists())
        QDir::root().mkpath(dir.absolutePath());

    return dir;
}

/*!
    Determines the base name of a file based on its \a name and the \a extension for its type.
*/
QString QFSContentEngine::determineBaseName(const QString &name, const QString &extension)
{
    QString baseName = name;

    if (!extension.isEmpty() && name.endsWith(extension, Qt::CaseInsensitive))
        baseName.chop(extension.length());

    return baseName;
}

/*!
    Determines the appropriate extension for content with the given \a name and \a type.

    If name ends in one of the known extensions for the type that extension will be returned,
    otherwise the default extension for the type will be returned.
*/
QString QFSContentEngine::determineExtension( const QString &name, const QMimeType &type )
{
    foreach( QString extension, type.extensions() )
    {
        extension.prepend( QLatin1Char( '.' ) );

        if( name.endsWith( extension, Qt::CaseInsensitive ) )
            return extension;
    }

    QString extension = type.extension();

    if( !extension.isEmpty() )
    {
        extension.prepend( QLatin1Char( '.' ) );
    }
    else
    {
        int index = name.indexOf( QLatin1Char( '.' ), name.length() - 6 );

        if( index != -1 )
            extension = name.mid( index );
    }

    return extension;
}

/*!
    \reimp
*/
bool QFSContentEngine::remove()
{
    QFile file( fileName() );

    if( !file.remove() )
    {
        setError( file.errorString() );

        return false;
    }
    else
        return true;
}

