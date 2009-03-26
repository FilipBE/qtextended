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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QtGui>
#include <QtopiaMedia>

class MusicPlayerPrivate;

class MusicPlayer : public QWidget
{
    Q_OBJECT
public:
    MusicPlayer( QWidget* parent = 0, Qt::WFlags f = 0 );
    ~MusicPlayer();

public slots:
    void viewNowPlaying();
    void viewGenre();
    void viewAlbums();
    void viewAlbums(const QString& artist);
    void viewArtists();
    void viewPlaylists();
    void ribbonClicked(int index);
    void delayedInitialisations();
    void doMediaListPlayNow(const QModelIndex& index);
    void doMediaListQueueNow(const QModelIndex& index);
    void doPlayingChanged(const QModelIndex& index);
    void mediaControlAvailable(QString const& id);
    void smoothListCurrentChanged(const QModelIndex &, const QModelIndex &);
    void modelReset();
    void artistViewClicked(const QModelIndex& index);
    void artistViewPressed(const QModelIndex& index);
    void doBack();
    void playlistViewClicked(const QModelIndex& index);
    void nowPlayingViewClicked(const QModelIndex& index);
    void playerStateChanged(QtopiaMedia::State state);
    void playButtonPushed();
    void addToQueueButtonPushed();

private:
    MusicPlayerPrivate *d;
    friend class MusicPlayerListViewDelegate;
};
#endif
