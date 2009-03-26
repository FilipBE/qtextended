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

#include <QMediaPlaylist>
#include <QSharedData>
#include <QFileSystem>

class QMediaPlaylistPrivate : public QObject, public QSharedData
{
    Q_OBJECT
public:
    QMediaPlaylistPrivate();

    QMediaPlaylistPrivate( const QMediaPlaylistPrivate& other );

    ~QMediaPlaylistPrivate() {}

    void     updateMyShuffle( const QModelIndex& index );
    QContent randomTrack() const;

    QContentList m_urls;
    QContentList m_shuffleUrls;
    QContentFilter m_filter;

    int m_playing;
    bool m_isShuffling;

    int m_shuffleHistoryNum;
    int m_shuffleTotalNum;

    void emitPlayingChanged() { emit playingChanged(m_playing); }

    static QHash<QString, QPlaylistHandlerInterface*> m_playlistHandlers;
signals:
    void playingChanged( int row );

    void beginRemoveRows(int first, int last);
    void endRemoveRows();
    void beginInsertRows(int first, int last);
    void endInsertRows();
};

QHash<QString, QPlaylistHandlerInterface*> QMediaPlaylistPrivate::m_playlistHandlers;

class QM3UHandler : public QPlaylistHandlerInterface
{
    QContentList load(const QString& filename) const;
    bool save(const QString& filename, const QContentList &fileset) const;
};

class QPLSHandler : public QPlaylistHandlerInterface
{
    QContentList load(const QString& filename) const;
    bool save(const QString& filename, const QContentList &fileset) const;
};

QMediaPlaylistPrivate::QMediaPlaylistPrivate()
    : QObject()
    , QSharedData()
    , m_playing(-1)
    , m_isShuffling(false)
    , m_shuffleHistoryNum(2)
    , m_shuffleTotalNum(10)
{
    if(m_playlistHandlers.empty())
    {
        QMediaPlaylist::registerTypeHandler(QLatin1String("m3u"), new QM3UHandler);
        QMediaPlaylist::registerTypeHandler(QLatin1String("pls"), new QPLSHandler);
    }
}

QMediaPlaylistPrivate::QMediaPlaylistPrivate( const QMediaPlaylistPrivate& other )
    : QObject()
    , QSharedData(other)
{
    m_urls = other.m_urls;
    m_shuffleUrls = other.m_shuffleUrls;
    m_filter = other.m_filter;

    m_playing = other.m_playing;
    m_isShuffling = other.m_isShuffling;

    m_shuffleHistoryNum = other.m_shuffleHistoryNum;
    m_shuffleTotalNum = other.m_shuffleTotalNum;
}

void QMediaPlaylistPrivate::updateMyShuffle( const QModelIndex& index )
{
    // fill it up
    if((m_shuffleUrls.count() < m_shuffleTotalNum) && (m_shuffleUrls.count() < m_urls.count()))
    {
        int insertCount = m_shuffleTotalNum - m_shuffleUrls.count();
        int insertedAfter = m_shuffleUrls.count();
        if(!insertedAfter)
            insertedAfter = 1;
        emit beginInsertRows(insertedAfter-1, insertedAfter+insertCount-1);
        for(int i=0;i<insertCount;i++) {
            QContent c = randomTrack();
            if (!c.isNull())
                m_shuffleUrls.append(c);
        }
        emit endInsertRows();
    }
    // shuffle it down
    if(index.row() < m_shuffleUrls.count() && index.row() != -1)
    {
        int removeCount = index.row() - m_shuffleHistoryNum;
        emit beginRemoveRows(0, removeCount - 1);
        for(int i=0;i<removeCount; i++)
            m_shuffleUrls.removeFirst();
        emit endRemoveRows();
    }
    m_playing = m_shuffleHistoryNum;
}

#define Q_EXECUTE_ONCE(code) \
    { \
        static QAtomicInt codeexecuted(0); \
        if(codeexecuted.testAndSetOrdered(0, 1)) { \
            code; \
        } \
    }

static const QString mapping[] = {"frequent", "infrequent", "never", ""};

QContent QMediaPlaylistPrivate::randomTrack() const
{
    // 8 biased array
    static const int array[8] = {0,1,1,1,1,1,1,2};
    static const int MISS_THRESHOLD = 50;
    Q_EXECUTE_ONCE(qsrand(QDateTime::currentDateTime().toTime_t()));
    if(m_urls.isEmpty())
        return QContent();

    int bias = array[qrand()%8];
    int misses = 0;

    while(misses < MISS_THRESHOLD) {
        int index = qrand() % m_urls.count();

        if(m_shuffleUrls.contains(m_urls.at(index)))
            misses++;
        else if(m_urls.at(index).property("rating") == mapping[QMediaPlaylist::Never])
            misses++;
        else {
            switch(bias) {
                case 2:
                    if(m_urls.at(index).property("rating") == mapping[QMediaPlaylist::Frequent])
                        return m_urls.at(index);
                    break;
                case 1:
                    if(m_urls.at(index).property("rating") != mapping[QMediaPlaylist::Infrequent])
                        return m_urls.at(index);
                    break;
                case 0:
                    return m_urls.at(index);
                    break;
            }
            misses++;
        }
    }
    return QContent();
}


QContentList QM3UHandler::load(const QString& filename) const
{
    QContentList result;
    // Open file and populate playlist
    QFile file( filename );
    if( file.open( QIODevice::ReadOnly ) ) {
        // For each line of playlist
        QTextStream stream( &file );
        while( !stream.atEnd() ) {
            QString line = stream.readLine();

            // Ignore blank lines and comments
            if( !line.isEmpty() && line[0] != '#' ) {
                // Add item to list of items
                result.append( QContent( line, false ) );
            }
        }
    }
    return result;
}

bool QM3UHandler::save(const QString& filename, const QContentList &filelist) const
{
    QFile output;
    if(!filename.startsWith("/"))
        output.setFileName(QFileSystem::documentsFileSystem().documentsPath() + "/" + filename);
    else
        output.setFileName(filename);

    if(!output.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
        return false;
    foreach(QContent content, filelist) {
        if(output.write( (content.fileName()+QLatin1String("\n")).toLatin1() ) == -1) {
            output.close();
            return false;
        }
    }
    output.close();
    return true;
}

QContentList QPLSHandler::load(const QString& filename) const
{
    QContentList result;
    QString playlistvalue;
    QSettings plsfile(filename, QSettings::IniFormat);
    foreach(QString key, plsfile.childGroups())
        if(key.compare("playlist", Qt::CaseInsensitive) == 0)
            playlistvalue = key;
    if(playlistvalue.isEmpty())
        return result;
    plsfile.beginGroup(playlistvalue);
    QHash<QString, QVariant> keyvalues;
    foreach(QString key, plsfile.childKeys())
        keyvalues.insert(key.toLower(), plsfile.value(key));
    if(keyvalues.contains("numberofentries")) {
        int numentries=keyvalues.value("numberofentries").toInt();

        for(int i=1;i<=numentries;i++) {
            if(keyvalues.contains("file"+QString::number(i))) {
                QContent content(keyvalues.value("file"+QString::number(i)).toString(), false);
                if(keyvalues.contains("title"+QString::number(i)))
                    content.setName(keyvalues.value("title"+QString::number(i)).toString());
                result+=content;
            }
        }
    }
    return result;
}

bool QPLSHandler::save(const QString& filename, const QContentList &filelist) const
{
    QSettings* plsfile;
    if(!filename.startsWith("/"))
        plsfile = new QSettings(QFileSystem::documentsFileSystem().documentsPath() + "/" + filename, QSettings::IniFormat);
    else
        plsfile = new QSettings(filename, QSettings::IniFormat);

    plsfile->beginGroup("playlist");
    plsfile->setValue("NumberOfEntries", filelist.count());
    for(int i=0;i<filelist.count();i++) {
        plsfile->setValue("File"+QString::number(i+1), filelist.at(i).fileName());
        plsfile->setValue("Title"+QString::number(i+1), filelist.at(i).name());
    }
    plsfile->setValue("Version", 2);
    return true;
}

/*!
    \class QPlaylistHandlerInterface
    \inpublicgroup QtMediaModule

    \brief The QPlaylistHandlerInterface class provides an abstract base class that must be subclassed when implementing new playlist load/save handlers.

    \sa QMediaPlaylist::registerTypeHandler()
*/

/*!
    \fn QContentList QPlaylistHandlerInterface::load(const QString& filename) const

    Returns a QContentList for the given \a filename, which is used to populate a QMediaPlaylist.
*/

/*!
    \fn bool QPlaylistHandlerInterface::save(const QString& filename, const QContentList &playlist) const

    Returns true if the passed \a playlist was sucessfully written to \a filename.
*/

/*!
    \fn QPlaylistHandlerInterface::~QPlaylistHandlerInterface()

    Destroys the playlisthandler interface object.
*/

/*!
    \class QMediaPlaylist
    \inpublicgroup QtMediaModule


    \brief The QMediaPlaylist class provides the basics for playlist handling of media content.

    \ingroup multimedia

    The QMediaPlaylist class is useful for efficient handling and displaying media content.
    QMediaPlaylist class is implicitly shared, so can be passed around as arguments with minimal
    cost. Unless a QMediaPlaylist is explicitly copied via the copy() function, all modifications
    to a playlist object will be reflected in all other shared copies.

    QMediaPlaylist is also derived from QAbstractListModel, and as such can be used by the model/view
    framework, or as the data source for a QMediaList object.
    

    \code
        mediaPlaylist = new QMediaPlayList();
        todo();
    \endcode
*/

/*!
    \enum QMediaPlaylist::DataRole

    \value Title           Title of media
    \value Url             Filename, http address
    \value Artist          Artist of media
    \value Album           Album of media
    \value Genre           Genre of media
    \value AlbumCover      Cover image of media
*/

/*!
    \enum QMediaPlaylist::Probability

    \value Frequent        Play this track a greater frequency when compared to tracks of other ratings.
    \value Infrequent      Play this track a lower frequency when compared to tracks of other ratings.
    \value Never           Don't play this track again.
    \value Unrated         Play this track a moderate frequency when compared to tracks of other ratings. This is the default probability rating for unrated tracks.
*/

/*!
    \fn void QMediaPlaylist::playingChanged( const QModelIndex& index )

    This signal is emitted whenever the current playing \a index is changed.

    \sa setPlaying()
*/

/*!
    Constructs an empty QMediaPlaylist object.
*/
QMediaPlaylist::QMediaPlaylist()
    : QAbstractListModel()
    , d(new QMediaPlaylistPrivate)
{
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));
}

/*!
    Constructs a new QMediaPlaylist object. If \a filename is a url, or an unknown file format,
    tries to create a playlist, one entry per line from the file. If the file extension has
    been previously registered via registerTypeHandler(), then loading will be handed off
    to the registered file type loader.

    \sa registerTypeHandler()
*/
QMediaPlaylist::QMediaPlaylist( const QString& filename )
    : QAbstractListModel()
    , d(new QMediaPlaylistPrivate)
{
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));

    QUrl    url(filename, QUrl::TolerantMode);

    if (url.isValid()) {
        QString scheme = url.scheme();
        if (scheme == "http")
            d->m_urls = loadBasicPlaylist(QStringList(filename));
        else if (scheme == "file" || scheme.isEmpty()) {
            QString checkedName = url.path();
            QString checkedExtension = QFileInfo(checkedName).suffix();
            QHashIterator<QString, QPlaylistHandlerInterface*> i(d->m_playlistHandlers);
            bool handlerSeen=false;
            while (i.hasNext() && !handlerSeen) {
                i.next();
                if(i.key() == checkedExtension) {
                    d->m_urls = i.value()->load(checkedName);
                    handlerSeen = true;
                }
            }
            if(!handlerSeen)
                d->m_urls = loadBasicPlaylist(QStringList(checkedName));
        }
    }
}

/*!
    Constructs a shared copy of \a playlist.
*/
QMediaPlaylist::QMediaPlaylist( const QMediaPlaylist& playlist )
    : QAbstractListModel()
    , d(playlist.d)
{
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));
}

/*!
    Constructs a new QMediaPlaylist object, one entry per line from \a files.
*/
QMediaPlaylist::QMediaPlaylist( const QStringList& files )
    : QAbstractListModel()
    , d(new QMediaPlaylistPrivate)
{
    d->m_urls = loadBasicPlaylist(QStringList(files));
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));
}

/*!
    Constructs a new QMediaPlaylist object, using a QContentSet with passed \a filter to initialise.
*/
QMediaPlaylist::QMediaPlaylist( const QContentFilter& filter )
    : QAbstractListModel()
    , d(new QMediaPlaylistPrivate)
{
    d->m_filter=filter;
    d->m_urls = QContentSet(filter).items();
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));
}

/*!
    Destroys the playlist.
*/
QMediaPlaylist::~QMediaPlaylist()
{
    disconnect(d.data(), 0, this, 0);
}

/*!
    Appends the \a playlist onto the end of the current playlist. If the current playlist is empty, falls through to playNow().
*/
void QMediaPlaylist::enqueue( const QMediaPlaylist &playlist )
{
    if(d->m_urls.count() == 0)
        playNow(playlist);
    else {
        beginInsertRows(QModelIndex(), d->m_urls.count(), d->m_urls.count() + playlist.d->m_urls.count() - 1);
        d->m_urls += playlist.d->m_urls;
        endInsertRows();
    }
    // may need to emit signal to signify that rows have been added.
}

/*!
    Replaces the current playlist with \a playlist, and calls playFirst().
*/
void QMediaPlaylist::playNow( const QMediaPlaylist &playlist )
{
    d->m_urls=playlist.d->m_urls;
    reset();
    playFirst();
}

/*!
    Removes the item at \a index.
*/
void QMediaPlaylist::remove( const QModelIndex& index )
{
    if(index.isValid())
        d->m_urls.removeAt(index.row());
}

/*!
    Removes all items from the playlist.
*/
void QMediaPlaylist::clear()
{
    d->m_urls.clear();
    d->m_shuffleUrls.clear();
    setPlaying(QModelIndex());
}

/*!
    Returns a suggested filename for saving the playlist to.

    \sa save()
*/
QString QMediaPlaylist::suggestedName() const
{
    return tr( "Saved Playlist.pls" );
}

/*!
    Saves the playlist into \a filename. If a handler is registered for the
    file extension of the filename, then the handler will be called to save the file, otherwise
    it will return false and not save the file.
*/
bool QMediaPlaylist::save( const QString& filename ) const
{
    QUrl    url(filename, QUrl::TolerantMode);

    if (url.isValid()) {
        QString scheme = url.scheme();
        if (scheme == "http")
            return false;
        else if (scheme == "file" || scheme.isEmpty()) {
            QString checkedName = url.path();
            QString checkedExtension = QFileInfo(checkedName).suffix();
            if(d->m_playlistHandlers.contains(checkedExtension))
                return d->m_playlistHandlers[checkedExtension]->save(checkedName, d->m_urls);
        }
    }
    return false;
}

/*!
    Returns true if shuffle has been set on this playlist.
*/
bool QMediaPlaylist::isShuffle() const
{
    return d->m_isShuffling;
}

/*!
    Turns on or off shuffle functionality on this playlist. Shuffle is a special weighted play queue, that dynamically
    resizes to keep shuffleTotalCount() items visible, with shuffleHistoryCount() number of songs visible
    as "history" when \a shuffle is set to true. Using probabilities set via setProbability(), tracks are added as needed to then end of
    the queue as the play location is moved, and the history is shrunk to maintain maximum of
    shuffleHistoryCount() number of recently played entries.

    \sa setShuffleValues(), setProbability()
*/
void QMediaPlaylist::setShuffle( bool shuffle )
{
    if(d->m_isShuffling != shuffle) {
        d->m_isShuffling = shuffle;
        if(d->m_isShuffling) {
            d->m_shuffleUrls.clear();
            reset();
            setPlaying(index(0,0));
        }
        else
            d->m_shuffleUrls.clear();
    }
}

/*!
    Sets \a historyCount number of recently played entries, and \a totalCount total number of entries in the list
    when shuffle mode is turned on.

    \sa setShuffle(), setProbability()
*/
void QMediaPlaylist::setShuffleValues( int historyCount, int totalCount )
{
    d->m_shuffleHistoryNum = historyCount;
    d->m_shuffleTotalNum = totalCount;
    if(isShuffle())
        reset();
}

/*!
    Returns the shuffle mode history value.

    \sa setShuffleValues(), setShuffle()
*/
int QMediaPlaylist::shuffleHistoryCount()
{
    return d->m_shuffleHistoryNum;
}

/*!
    Returns the shuffle mode history value.

    \sa setShuffleValues(), setShuffle()
*/
int QMediaPlaylist::shuffleTotalCount()
{
    return d->m_shuffleTotalNum;
}

/*!
    \fn bool QMediaPlaylist::operator!=( const QMediaPlaylist& other ) const
    If \a other is a shared copy of this playlist, return false, otherwise, compare internal values
    and if they match up, return false.
*/


/*!
    If \a other is a shared copy of this playlist, return true, otherwise, compare internal values
    and if they match up, return true.
*/
bool QMediaPlaylist::operator==( const QMediaPlaylist& other ) const
{
    if (d == other.d)
        return true;

    if (d->m_playing != other.d->m_playing)
        return false;
    if (d->m_isShuffling && d->m_shuffleUrls != other.d->m_shuffleUrls)
        return false;
    if (d->m_urls != other.d->m_urls || d->m_isShuffling != other.d->m_isShuffling)
        return false;

    return true;
}

/*!
    Sets this playlist to be a shallow copy of \a other. Any modifications to the other playlist
    object will be reflected by this object, and vice-versa.
*/
QMediaPlaylist& QMediaPlaylist::operator=( const QMediaPlaylist& other )
{
    disconnect(d.data(), 0, this, 0);
    d=other.d;
    connect(d.data(), SIGNAL(playingChanged(int)), this, SLOT(playingChangedProxy(int)));

    connect(d.data(), SIGNAL(beginRemoveRows(int,int)), this, SLOT(beginRemoveRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endRemoveRows()), this, SLOT(endRemoveRowsProxy()));
    connect(d.data(), SIGNAL(beginInsertRows(int,int)), this, SLOT(beginInsertRowsProxy(int,int)));
    connect(d.data(), SIGNAL(endInsertRows()), this, SLOT(endInsertRowsProxy()));

    reset();
    return *this;
}

/*!
    \reimp
*/
QVariant QMediaPlaylist::data( const QModelIndex& index, int role ) const
{
    if(isShuffle() && d->m_shuffleUrls.count() == 0)
        d->updateMyShuffle(index);

    if(!index.isValid())
        return QVariant();

    QContent content;
    if(isShuffle())
        content = d->m_shuffleUrls.at(index.row());
    else
        content = d->m_urls.at(index.row());
    switch( role )
    {
    case QMediaPlaylist::Title:
        if(!content.property( QContent::Title ).isEmpty())
            return content.property( QContent::Title );
        else if(!content.name().isEmpty())
            return content.name();
        else
            return content.fileName();
    case QMediaPlaylist::Url:
        {
            QUrl url(content.fileName());

            if (url.scheme().isEmpty())
                url.setScheme(QLatin1String("file"));

            return url;
        }
    case QMediaPlaylist::Artist:
        return content.property( QContent::Artist );
    case QMediaPlaylist::Album:
        return content.property( QContent::Album );
    case QMediaPlaylist::Genre:
        return content.property( QContent::Genre );
    case QMediaPlaylist::AlbumCover:
        return content.thumbnail();
    default:
        // Ignore
        return QVariant();
    }
}

/*!
    \reimp
*/
int QMediaPlaylist::rowCount( const QModelIndex& ) const
{
    if(isShuffle())
        return d->m_shuffleUrls.count();
    else
        return d->m_urls.count();
}

/*!
    Returns the index of the currently playing track.
*/
QModelIndex QMediaPlaylist::playing() const
{
    return d->m_playing == -1 ? QModelIndex() : index(d->m_playing, 0);
}

/*!
    Sets the currently playing track to \a index.

    \sa playingChanged()
*/
void QMediaPlaylist::setPlaying( const QModelIndex& index )
{
    if(!index.isValid() && isShuffle())
        d->updateMyShuffle(index);

    if(!index.isValid())
        return;

    d->m_playing = index.row();
    if(isShuffle() && index.row() > d->m_shuffleHistoryNum)
        d->updateMyShuffle(index);

    d->emitPlayingChanged( );
}

/*!
    Creates a deep copy of playlist \a other.
*/
void QMediaPlaylist::copy( const QMediaPlaylist &other )
{
    d=other.d;
    d.detach();
}

/*!
    Sets the shuffle \a probability for the media located at \a index.

    \sa setShuffle(), setShuffleValues()
*/
void QMediaPlaylist::setProbability( const QModelIndex& index, Probability probability )
{
    if(!index.isValid())
        return;
    // TODO: handle shuffle playlist indexing
    if(index.row() >= d->m_urls.count())
        return;
    QContent &track = d->m_urls[index.row()];
    track.setProperty( "rating", mapping[probability] );
    if(track.id() != QContent::InvalidId)
        track.commit();
}

/*!
    Resets the shuffle mode probabilities of all media items in the playlist to QMediaPlaylist::Unrated.

    \sa setProbability()
*/
void QMediaPlaylist::resetProbabilities()
{
    for(int i=0;i<d->m_urls.count();i++)
    {
        QContent &track = d->m_urls[i];
        if(track.property("rating") != mapping[Unrated]) {
            track.setProperty( "rating", mapping[Unrated] );
            if(track.id() != QContent::InvalidId)
                track.commit();
        }
    }
}

/*!
    Returns the shuffle mode probability setting of the media located at \a index.

    \sa setProbability(), setShuffle()
*/
QMediaPlaylist::Probability QMediaPlaylist::probability( const QModelIndex& index ) const
{
    if(!index.isValid())
        return Unrated;
    // TODO: handle shuffle playlist indexing
    if(index.row() >= d->m_urls.count())
        return Unrated;
    QContent &track = isShuffle() ? d->m_shuffleUrls[index.row()] : d->m_urls[index.row()];
    QString rating=track.property( "rating" );
    if(rating == mapping[Frequent])
        return Frequent;
    else if(rating == mapping[Infrequent])
        return Infrequent;
    else if(rating == mapping[Never])
        return Never;
    else
        return Unrated;
}

QContentList QMediaPlaylist::loadBasicPlaylist(const QStringList& filenames)
{
    QContentList result;
    foreach(QString filename, filenames)
    {
        QUrl    url(filename, QUrl::TolerantMode);

        if (url.isValid()) {
            QString scheme = url.scheme();
            if (scheme == "http")
                result << QContent(filename, false);
            else if (scheme == "file" || scheme.isEmpty()) {
                QString checkedName = url.path();
                QString checkedExtension = QFileInfo(checkedName).suffix();
                QHashIterator<QString, QPlaylistHandlerInterface*> i(d->m_playlistHandlers);
                bool handlerSeen=false;
                while (i.hasNext() && !handlerSeen) {
                    i.next();
                    if(i.key() == checkedExtension) {
                        result << i.value()->load(checkedName);
                        handlerSeen = true;
                    }
                }
                if(!handlerSeen)
                    result << QContent(checkedName, false);
            }
        }
    }
    return result;
}

/*!
    Registers a playlist load/save \a handler for the given file \a extension.

    \sa load(), save()
*/
void QMediaPlaylist::registerTypeHandler(const QString& extension, QPlaylistHandlerInterface *handler)
{
    if(QMediaPlaylistPrivate::m_playlistHandlers.contains(extension))
        delete QMediaPlaylistPrivate::m_playlistHandlers.take(extension);
    if(handler)
        QMediaPlaylistPrivate::m_playlistHandlers.insert(extension, handler);
}

/*!
    Unregisters a playlist load/save handler for the given file \a extension.
*/
void QMediaPlaylist::unregisterTypeHandler(const QString& extension)
{
    if(QMediaPlaylistPrivate::m_playlistHandlers.contains(extension))
        delete QMediaPlaylistPrivate::m_playlistHandlers.take(extension);
}

QContent QMediaPlaylist::randomTrack() const
{
    return d->randomTrack();
}

/*!
    Sets the playing index to the next media in the playlist.
*/
void QMediaPlaylist::playNext()
{
    setPlaying(nextIndex());
}

/*!
    Sets the playing index to the previous media in the playlist.
*/
void QMediaPlaylist::playPrevious()
{
    setPlaying(previousIndex());
}

/*!
    Sets the playing index to the first media in the playlist.
*/
void QMediaPlaylist::playFirst()
{
    setPlaying(firstIndex());
}

/*!
    Returns the index of the first media item in the playlist.
*/
QModelIndex QMediaPlaylist::firstIndex()
{
    return index(0);
}

/*!
    Returns the index of the next media item in the playlist.
*/
QModelIndex QMediaPlaylist::nextIndex()
{
    if(isShuffle() || d->m_playing+1 < d->m_urls.count())
        return index(d->m_playing+1);
    else
        return QModelIndex();
}

/*!
    Returns the index of the previous media item in the playlist.
*/
QModelIndex QMediaPlaylist::previousIndex()
{
    if(d->m_playing > 0)
        return index(d->m_playing-1);
    else
        return QModelIndex();
}

void QMediaPlaylist::playingChangedProxy(int row)
{
    emit playingChanged(index(row));
}

void QMediaPlaylist::beginRemoveRowsProxy(int first, int last)
{
    beginRemoveRows(QModelIndex(), first, last);
}

void QMediaPlaylist::endRemoveRowsProxy()
{
    endRemoveRows();
}

void QMediaPlaylist::beginInsertRowsProxy(int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
}

void QMediaPlaylist::endInsertRowsProxy()
{
    endInsertRows();
}

/*!
    Returns the QContent object for the media located at \a index.
*/
QContent QMediaPlaylist::content(const QModelIndex& index) const
{
    if(index.isValid()) {
        if(isShuffle()) {
            if(index.row() < d->m_shuffleUrls.count())
                return d->m_shuffleUrls.at(index.row());
        }
        else {
            if(index.row() < d->m_urls.count())
                return d->m_urls.at(index.row());
        }
    }
    return QContent();
}

#include "qmediaplaylist.moc"
