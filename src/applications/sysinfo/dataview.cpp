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

#include "dataview.h"
#include "graph.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qlayout.h>
#include <qtimer.h>
#include <QLabel>
#include <qcontent.h>
#include <qcontentset.h>
#include <qstorage.h>

#include <qtopianamespace.h>


DataView::DataView( QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f), data(0)
{
    QTimer::singleShot(40, this, SLOT(init()));
}

DataView::~DataView()
{
    if(data)
        delete data;
}

void DataView::init()
{
    storage = QStorageMetaInfo::instance();
    QVBoxLayout *vb = new QVBoxLayout(this);

    QLabel * desc = new QLabel( tr("Data Types"), this );
    vb->addWidget( desc);

    data = new GraphData();

    graph = new BarGraph( this );
    graph->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    vb->addWidget(graph, 1);
    graph->setData( data );

    legend = new GraphLegend(this);
    vb->addWidget(legend);
    legend->setData(data);

    vb->addStretch( 100 );

    updateData();

    startTimer( 5000 );
}

void DataView::timerEvent(QTimerEvent *)
{
    if (isVisible()) {
        storage->update();
        updateData();
    }
}

void DataView::updateData()
{
    long av = 0;
    long to = -1;
    long total = 0;
    long avail = 0;

    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, storage->fileSystems(&fsf) ) {
        fileSystemMetrics( fs, &av, &to );
        total += to;
        avail += av;
    }

    int mail = 0;
    QString dirName = Qtopia::homePath() + "/Applications/qtmail";
    QDir mailDir(dirName);
    const QFileInfoList list = mailDir.entryInfoList();
    foreach( const QFileInfo &info, list ){
        mail += info.size()/1024;
    }

    QString filter = "image/*";
    int images = documentSize(filter);
    filter = "audio/*";
    int audio = documentSize(filter);
    filter = "video/*";
    int video = documentSize(filter);
    filter = "text/*";
    int txt = documentSize(filter);

    data->clear();

    QString unitKB = tr("kB"," short for kilobyte");
    QString unitMB = tr("MB", "short for megabyte");

    if (audio < 10240)
        data->addItem(tr("Audio (%1 %2)").arg(audio).arg(unitKB), audio);
    else
        data->addItem(tr("Audio (%1 %2)").arg(audio/1024).arg(unitMB), audio);

    if (images < 10240)
        data->addItem(tr("Images (%1 %2)").arg(images).arg(unitKB), images);
    else
        data->addItem(tr("Images (%1 %2)").arg(images/1024).arg(unitMB), images);

    if (mail < 10240)
        data->addItem(tr("Mailbox (%1 %2)").arg(mail).arg(unitKB), mail);
    else
        data->addItem(tr("Mailbox (%1 %2)").arg(mail/1024).arg(unitMB), mail);

    if (txt < 10240)
        data->addItem(tr("Text (%1 %2)").arg(txt).arg(unitKB),txt);
    else
        data->addItem(tr("Text (%1 %2)").arg(txt/1024).arg(unitMB),txt);

    if (video < 1024)
        data->addItem(tr("Video (%1 %2)").arg(video).arg(unitKB), video);
    else
        data->addItem(tr("Video (%1 %2)").arg(video/1024).arg(unitMB), video);

    if (avail >= 0)
        if (avail < 10240)
            data->addItem(tr("Free (%1 %2)").arg(avail).arg(unitKB), avail);
        else
            data->addItem(tr("Free (%1 %2)").arg(avail/1024).arg(unitMB), avail);

    graph->update();
    legend->update();
}

int DataView::documentSize(QString& filter)
{
    uint sum = 0;
    QContentSet allDocs(QContentFilter::MimeType, filter);
    QList<QContent> list = allDocs.items();

    foreach(const QContent &dl, list ) {
        sum += QFileInfo(dl.fileName()).size()/1024;
    }

    return sum;
}

void DataView::fileSystemMetrics(const QFileSystem *fs, long *avail, long *total)
{
    long mult = 0;
    long div = 0;
    if ( fs->blockSize() ) {
        mult = fs->blockSize() / 1024;
        div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;

    *total = fs->totalBlocks() * mult / div;
    *avail = fs->availBlocks() * mult / div;
}

