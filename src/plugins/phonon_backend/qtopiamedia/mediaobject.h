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

#ifndef MEDIAOBJECT_H
#define MEDIAOBJECT_H

#include <QObject>

#include <phonon/mediaobjectinterface.h>

#include <qtopiamedia/media.h>

#include "medianode.h"

class QMediaContent;

namespace Phonon
{

namespace qtopiamedia
{

class Backend;

class MediaObjectPrivate;

class MediaObject :
    public QObject,
    public MediaObjectInterface,
    public MediaNode
{
    Q_OBJECT
    Q_INTERFACES(Phonon::MediaObjectInterface Phonon::qtopiamedia::MediaNode)


public:
    MediaObject(Backend* backend, QObject* parent);
    ~MediaObject();

    void play();
    void pause();
    void stop();

    void seek(qint64 milliseconds);

    qint32 tickInterval() const;
    void setTickInterval(qint32 interval);

    bool hasVideo() const;
    bool isSeekable() const;

    qint64 currentTime() const;
    qint64 totalTime() const;

    Phonon::State state() const;

    QString errorString() const;
    Phonon::ErrorType errorType() const;

    Phonon::MediaSource source() const;
    void setSource(const Phonon::MediaSource &);
    void setNextSource(const Phonon::MediaSource &source);

    //qint64 remainingTime() const { return totalTime() - currentTime(); }
    qint32 prefinishMark() const;
    void setPrefinishMark(qint32);

    qint32 transitionTime() const;
    void setTransitionTime(qint32);

    bool connectNode(MediaNode* node);
    bool disconnectNode(MediaNode* node);
    void setContent(QMediaContent* content);

signals:
    void currentSourceChanged(const MediaSource &newSource);
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);
    void tick(qint64 time);
    void metaDataChanged(QMultiMap<QString, QString>);
    void seekableChanged(bool);
    void hasVideoChanged(bool);

    void finished();
    void prefinishMarkReached(qint32);
    void aboutToFinish();
    void totalTimeChanged(qint64 length);
    void bufferStatus(int percentFilled);

private slots:
    void mediaError(QString const& errorString);
    void controlAvailable(QString const& controlName);
    void controlUnavailable(QString const& controlName);

    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 position);
    void lengthChanged(quint32 ms);

    void tickTimeout();

private:
    void readyContent();

    MediaObjectPrivate* d;
};

}   // ns qtopiamedia

}   // Phonon
#endif  // MEDIAOBJECT_H
