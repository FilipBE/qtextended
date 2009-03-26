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

#ifndef STORAGE_H
#define STORAGE_H
#include <QWidget>
#include <QHash>
#include "sysinfo.h"

class QLabel;
class GraphData;
class Graph;
class GraphLegend;
class QFileSystem;
class MountInfo;
class QStorageMetaInfo;
class QScrollArea;
class CleanupWizard;
class QByteArray;

class StorageInfoView : public QWidget
{
    Q_OBJECT
public:
    StorageInfoView( QWidget *parent=0 );
    QSize sizeHint() const;

protected:
    void timerEvent(QTimerEvent*);

signals:
    void updated();

private slots:
    void updateMounts();
    void init();
    void cleanup();

private:
    void setVBGeom();
    QStorageMetaInfo *sinfo;
    QScrollArea *area;
    QHash<QFileSystem*, MountInfo*> fsHash;
    CleanupWizard* cleanupWizard; 
};

class MountInfo : public QWidget
{
    Q_OBJECT
public:
    MountInfo( const QFileSystem*, QWidget *parent=0 );
    ~MountInfo();

public slots:
    void refresh();

private:
    void getSizeString( double &size, QString &string );

    QString title;
    const QFileSystem *fs;
    QLabel *totalSize;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};

#endif
