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

#ifndef QSTORAGEDEVICESELECTOR_H
#define QSTORAGEDEVICESELECTOR_H

#include <qtopiaglobal.h>
#include <qstringlist.h>
#include <qcombobox.h>

class QContent;
class QListViewItem;
class QDocumentMetaInfo;
class QFileSystem;
class QStorageMetaInfo;
class QFileSystemFilter;

class QStorageDeviceSelectorPrivate;

class QTOPIA_EXPORT QStorageDeviceSelector : public QComboBox
{
    Q_OBJECT
public:
    explicit QStorageDeviceSelector( QWidget *parent=0 );
    explicit QStorageDeviceSelector( const QContent &lnk, QWidget *parent=0 );

    ~QStorageDeviceSelector();

    void setLocation( const QString& path );
    void setLocation( const QContent & );

    QString installationPath() const;
    QString documentPath() const;
    const QFileSystem *fileSystem() const;

    bool isChanged() const;

    void setFilter( QFileSystemFilter *fsf );

signals:
    void newPath();

private slots:
    void updatePaths();

private:
    void setupCombo();
    QStringList locations;
    QStorageMetaInfo *storage;
    QStorageDeviceSelectorPrivate *d;
    QFileSystemFilter *filter;
};

#endif
