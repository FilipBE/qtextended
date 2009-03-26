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

#ifndef QMEDIAPLAYLIST_H
#define QMEDIAPLAYLIST_H

#include <QtGui>
#include <QContentSet>
#include <QExplicitlySharedDataPointer>

class QMediaPlaylistPrivate;
class QPlaylistHandlerInterface;

class QTOPIAMEDIA_EXPORT QMediaPlaylist : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DataRole { Title = Qt::DisplayRole, Url = Qt::UserRole, Artist, Album, Genre, AlbumCover };

    QMediaPlaylist();
    QMediaPlaylist( const QString& filename );
    QMediaPlaylist( const QMediaPlaylist& playlist );
    QMediaPlaylist( const QStringList& files );
    QMediaPlaylist( const QContentFilter& filter );
    virtual ~QMediaPlaylist();

    virtual void enqueue( const QMediaPlaylist &playlist );
    virtual void playNow( const QMediaPlaylist &playlist );

    virtual void remove( const QModelIndex& index );
    virtual void clear();

    virtual QString suggestedName() const;
    virtual bool save( const QString& filename ) const;

    virtual bool isShuffle() const;
    virtual void setShuffle( bool value );
    void setShuffleValues( int historyCount, int totalCount );
    int shuffleHistoryCount();
    int shuffleTotalCount();

    bool operator==( const QMediaPlaylist& other ) const;
    bool operator!=( const QMediaPlaylist& other ) const
    {
        return !(*this==other);
    }
    QMediaPlaylist& operator=( const QMediaPlaylist& other );

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    virtual QModelIndex playing() const;
    virtual void setPlaying( const QModelIndex& index );
    virtual void playNext();
    virtual void playPrevious();
    virtual void playFirst();
    QModelIndex firstIndex();
    QModelIndex nextIndex();
    QModelIndex previousIndex();

    virtual void copy( const QMediaPlaylist &other );

    enum Probability { Frequent, Infrequent, Never, Unrated };
    virtual Probability probability( const QModelIndex& index ) const;
    virtual void setProbability( const QModelIndex& index, Probability probability );
    virtual void resetProbabilities();
    static void registerTypeHandler(const QString& extension, QPlaylistHandlerInterface *handler);
    static void unregisterTypeHandler(const QString& extension);

    QContent content(const QModelIndex& index) const;

private:
    QContentList loadBasicPlaylist(const QStringList& filenames);
    QContent randomTrack() const;

signals:
    void playingChanged( const QModelIndex& index );

private slots:
    void playingChangedProxy(int row);

    void beginRemoveRowsProxy(int first, int last);
    void endRemoveRowsProxy();
    void beginInsertRowsProxy(int first,int last);
    void endInsertRowsProxy();

private:
    QExplicitlySharedDataPointer<QMediaPlaylistPrivate> d;
};

class QPlaylistHandlerInterface
{
public:
    virtual ~QPlaylistHandlerInterface() {}
    virtual QContentList load(const QString& filename) const = 0;
    virtual bool save(const QString& filename, const QContentList &playlist) const = 0;
};

#endif
