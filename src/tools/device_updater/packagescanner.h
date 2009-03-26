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

#ifndef PACKAGESCANNER_H
#define PACKAGESCANNER_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QStringList>
#include <QMutex>

#include "scannerthread.h"

// By default, this is the value for MAX on the progress bar
// so base all progress values from this
#define DEFAULT_PROGRESS_MAX 100

class DeviceConnector;

struct PackageItem
{
    QString qpkPath;
    QString qpdPath;
    QString name;
    QString display;
    QDateTime lastMod;
    bool hasBeenUploaded;
};


class PackageScanner : public QAbstractListModel
{
    Q_OBJECT
public:
    PackageScanner( const QString &, QObject *parent = 0 );
    ~PackageScanner();
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags(const QModelIndex&) const;
    PackageItem *findPackageByName( const QString & );
    void appendPackage( PackageItem *package );
public slots:
    void refresh();
    void sendPackage( const QModelIndex & );
    void sendPackage( const QString & );
signals:
    void progressMessage( const QString & );
    void progressValue( int );
    void updated();
private slots:
    void scannerDone();
    void connectorComplete();
private:
    void sendIt( PackageItem *pkg );
    QString mDir;
    QList<PackageItem*> mPackageList;
    mutable QMutex mPackageListMutex;
    ScannerThread *mScanner;
    DeviceConnector *mConnector;
    friend class ScannerThread;
};

#endif
