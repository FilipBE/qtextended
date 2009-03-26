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

#ifndef PACKAGEMODEL_H
#define PACKAGEMODEL_H

#include <QThread>
#include <QDialog>

#include <QAbstractItemModel>
#include <QList>
#include <QHash>

#include "packagecontroller.h"

#include <qstorage.h>

class AbstractPackageController;

class PackageModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class PackageServiceInstaller;
public:
    PackageModel( QObject* parent = 0 );
    ~PackageModel();

    static const unsigned int ID_SIZE;
    static const unsigned int ROW_MAX;

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column,
            const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void populateServers();
    void setServers( const QHash<QString,QString> & );
    void setServer( const QString& server );
    QStringList getServers() const;
    QString getOperation( const QModelIndex & );
    bool isInstalled(const QModelIndex &index) const;
    InstallControl *installControl();

signals:
    void targetsUpdated( const QStringList & );
    void serversUpdated( const QStringList & );
    void domainUpdate( const QString & );
    void rowsRemoved(const QModelIndex&, int, int);
    void rowsAboutToBeRemoved(const QModelIndex&,int,int);
    void serverStatus( const QString & );
    void newlyInstalled( const QModelIndex & );
public slots:
    void setInstallTarget( const QString & );
    void activateItem( const QModelIndex& );
    void reenableItem( const QModelIndex& );
    void userTargetChoice( const QString & );
private slots:
    void publishTargets();
    void controllerUpdate();
    void packageInstalled( const InstallControl::PackageInfo & );
    void serverStatusUpdated( const QString & );
private:
    unsigned int getParent( qint64 ) const;
    unsigned int getColumn( qint64 ) const;
    unsigned int getRow( qint64 ) const;
    unsigned int makeId( unsigned int, unsigned int, unsigned int ) const;
    // QList<QVariant> columnHeads;
    QList<AbstractPackageController *> rootItems;
    mutable QStorageMetaInfo *storage;
    AbstractPackageController *networked, *installed;
    QString currentInstallTarget;

    QHash<QString,QString> servers;
    QHash<QString,QString> mediaNames;
    QString activeServer;
    static const int InstalledIndex;
    static const int NetworkedIndex;
};

////////////////////////////////////////////////////////////////////////
/////
///// inline implementations
/////
inline QStringList PackageModel::getServers() const
{
    return servers.keys();
}

// Parent can be 0 - 254, stored in leftmost 8 bits, 255 is root
inline unsigned int PackageModel::getParent( qint64 id ) const
{
    return ((quint64)id >> ( ID_SIZE - 8 )) & 0x000000ffU;
}

// Column can be 0 - 3, stored in next 2 bits
inline unsigned int PackageModel::getColumn( qint64 id ) const
{
    return ((quint64)id >> ( ID_SIZE - 10 )) & 0x00000003U;
}

// Row can be 0 - 2^^(ID_SIZE - 10), stored in bottom ID_SIZE - 10 bits
inline unsigned int PackageModel::getRow( qint64 id ) const
{
    return (quint64)id & 0x0003ffff;
}

// Note - caller must ensure the arguments are in the ranges above or results
// will be undefined (ie complete bollocks) since the top bits arent masked off
inline unsigned int PackageModel::makeId( unsigned int row, unsigned int col, unsigned int p ) const
{
    return row | ( col << (ID_SIZE-10)) | ( p << (ID_SIZE-8));
}

#endif
