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

#include <QUrl>
#include <QDocumentSelector>

#include <QDebug>

#include <phonon/audiooutput.h>
#include <phonon/videowidget.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>

#include "mainwindow.h"


class MainWindowPrivate
{
public:
    QDocumentSelector*      selector;
    Phonon::AudioOutput*    audioOutput;
    Phonon::VideoWidget*    videoWidget;
    Phonon::MediaObject*    mediaObject;
};


MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags),
    d(new MainWindowPrivate)
{

    d->selector = new QDocumentSelector(this);
    d->selector->setFilter(QContentFilter(QContent::Document) &
                            (QContentFilter::mimeType("video/*") |
                            QContentFilter::mimeType("audio/*")));
    connect(d->selector, SIGNAL(documentSelected(QContent)), SLOT(documentSelected(QContent)));

    setCentralWidget(d->selector);

    d->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    d->videoWidget = new Phonon::VideoWidget(this);
    d->mediaObject = new Phonon::MediaObject(this);
//    metaInformationResolver = new Phonon::MediaObject(this);

    d->mediaObject->setTickInterval(1000);

    // MediaObject
    connect(d->mediaObject, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)),
            SLOT(currentSourceChanged(const Phonon::MediaSource &)));
    connect(d->mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(d->mediaObject, SIGNAL(tick(qint64)), SLOT(tick(qint64)));
    /*
    connect(d->mediaObject, SIGNAL(metaDataChanged(QMultiMap<QString,QString>)),
            SLOT(metaDataChanged(QMultiMap<QString,QString>)));
            */
    connect(d->mediaObject, SIGNAL(seekableChanged(bool)), SLOT(seekableChanged(bool)));
    connect(d->mediaObject, SIGNAL(hasVideoChanged(bool)), SLOT(hasVideoChanged(bool)));
    connect(d->mediaObject, SIGNAL(finished()), SLOT(finished()));
    connect(d->mediaObject, SIGNAL(prefinishMarkReached(qint32)), SLOT(prefinishMarkReached(qint32)));
    connect(d->mediaObject, SIGNAL(aboutToFinish()), SLOT(aboutToFinish()));
    connect(d->mediaObject, SIGNAL(totalTimeChanged(qint64)), SLOT(totalTimeChanged(qint64)));
    connect(d->mediaObject, SIGNAL(bufferStatus(int)), SLOT(bufferStatus(int)));

    // Meta info
    /*
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            SLOT(metaStateChanged(Phonon::State, Phonon::State)));
    */

    Phonon::createPath(d->mediaObject, d->audioOutput);
    Phonon::createPath(d->mediaObject, d->videoWidget);
}

MainWindow::~MainWindow()
{
    d->mediaObject->stop();

    delete d;
}

void MainWindow::documentSelected(QContent const& content)
{
    d->mediaObject->setCurrentSource(content.fileName());
    d->mediaObject->play();
}

void MainWindow::currentSourceChanged(Phonon::MediaSource const& mediaSource)
{
    qDebug() << "MainWindow::currentSourceChanged()" << mediaSource.url();
}

void MainWindow::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    qDebug() << "MainWindow::stateChanged(newState,oldState)" << newState << oldState;

    if (newState == Phonon::StoppedState) {
        if (d->mediaObject->hasVideo()) {
            setCentralWidget(d->selector);
            d->selector->show();
        }
    }
}

void MainWindow::tick(qint64 time)
{
    qDebug() << "MainWindow::tick()" << time;
}

void MainWindow::metaDataChanged(QMultiMap<QString, QString> metaData)
{
    qDebug () << "MainWindow::metaDataChanged()" << metaData;
}

void MainWindow::seekableChanged(bool seekable)
{
    qDebug() << "MainWindow::seekableChanged()" << seekable;
}

void MainWindow::hasVideoChanged(bool hasVideo)
{
    qDebug() << "MainWindow::hasVideoChanged()" << hasVideo;

    if (hasVideo) {
        d->selector->hide();
        d->selector->setParent(0);

        setCentralWidget(d->videoWidget);
        d->videoWidget->showMaximized();
    }
}

void MainWindow::finished()
{
    qDebug() << "MainWindow::finished()";
}

void MainWindow:: prefinishMarkReached(qint32 mark)
{
    qDebug() << "MainWindow::prefinishMarkReached()" << mark;
}

void MainWindow::aboutToFinish()
{
    qDebug() << "MainWindow::aboutToFinsih()";
}

void MainWindow::totalTimeChanged(qint64 length)
{
    qDebug() << "MainWindow::totalTimeChanged()" << length;
}

void MainWindow::bufferStatus(int percentFilled)
{
    qDebug() << "MainWindow::bufferStatus()" << percentFilled;
}

