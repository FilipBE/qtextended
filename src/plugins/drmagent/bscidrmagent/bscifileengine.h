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

#ifndef BSCIFILEENGINE_H
#define BSCIFILEENGINE_H

#include <qdrmcontentplugin.h>
#include <QFSFileEngine>

class BSciDrmFileEngine : public QFSFileEngine
{
public:
    BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath );
    BSciDrmFileEngine( QDrmContentPlugin *plugin, const QString &filePath, const QString &dcfPath, const QString &baseName );
    virtual ~BSciDrmFileEngine();

    virtual bool close();

    virtual bool copy( const QString & newName );

    virtual QStringList entryList( QDir::Filters filters, const QStringList &filterNames ) const;

    virtual FileFlags fileFlags( FileFlags type = FileInfoAll ) const;

    virtual QString fileName( FileName file = DefaultName ) const;

    virtual bool mkdir( const QString & dirName, bool createParentDirectories ) const;

    virtual bool open( QIODevice::OpenMode mode );

    virtual qint64 pos() const;

    virtual qint64 read( char * data, qint64 maxlen );

    virtual bool seek ( qint64 offset );

    virtual bool setPermissions( uint permissions );

    virtual bool setSize( qint64 size );

    virtual qint64 size() const;

    virtual qint64 write( const char *data, qint64 len );

protected:
    virtual QString baseName() const;

    virtual QString defaultName() const;

    virtual QString contentId() const;

    virtual QStringList contentIds() const;

    virtual bool isMultipart() const;

    virtual bool isLink() const;

private:
    QDrmContentPlugin *m_plugin;
    QString m_filePath;
    QString m_baseName;
    QIODevice *m_io;
};

#endif
