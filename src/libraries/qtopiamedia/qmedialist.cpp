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

#include <stdlib.h> //random()

#include "qmedialist.h"

#include <QMediaPlaylist>
#include <QMediaContent>
#include <QContent>
#include <QMap>
#include <QTime>
#include <QRegExp>

class QMediaListPrivate : public QObject
{
    Q_OBJECT
public:
    QMediaListPrivate(QMediaList *parent)
        : QObject(parent)
        , filtermode(false)
        , randomizer(false)
    {
        grouping    = QMediaList::ShowGrouped;
        filterState = 0;
        displayRole = 0;
        sorter      = QMediaList::Ascending;
        count       = 0;
        sortOrder.clear();
        needsRefresh = true;

        emptyDisplayRole = QMediaList::HideEmpty;

        q = parent;
    }
    ~QMediaListPrivate()
    {
    }

    QMediaList::Grouping    grouping;
    bool              filtermode;
    bool              randomizer;
    QMediaList::SortDirection   sorter;
    bool              needsRefresh;

    int               count;
    int               displayRole;
    QMediaList::Properties  emptyDisplayRole;

    int               filterState;
    QRegExp           searchTitle;
    QRegExp           searchUrl;
    QRegExp           searchArtist;
    QRegExp           searchAlbum;
    QRegExp           searchGenre;
    QStringList       mimeTypes;

    QMediaPlaylist    mediaPlaylist;

    QMap<int, QModelIndex> map;
    QList<int>        sortOrder;

    QMediaList *q;

    QMediaListPrivate &operator=(const QMediaListPrivate& other);
    bool operator==(const QMediaListPrivate& other) const;

public slots:
    void setNeedsRefresh();
};

void QMediaListPrivate::setNeedsRefresh()
{
    needsRefresh = true;
    if(q)
        q->endFilter();
}

QMediaListPrivate &QMediaListPrivate::operator=(const QMediaListPrivate& other)
{
    grouping = other.grouping;
    filtermode = other.filtermode;
    randomizer = other.randomizer;
    sorter = other.sorter;
    needsRefresh = other.needsRefresh;
    count = other.count;
    displayRole = other.displayRole;
    emptyDisplayRole = other.emptyDisplayRole;
    filterState = other.filterState;
    searchTitle = other.searchTitle;
    searchUrl = other.searchUrl;
    searchArtist = other.searchArtist;
    searchAlbum = other.searchAlbum;
    searchGenre = other.searchGenre;
    mimeTypes = other.mimeTypes;
    mediaPlaylist = other.mediaPlaylist;
    map = other.map;
    sortOrder = other.sortOrder;
    return *this;
}

bool QMediaListPrivate::operator==(const QMediaListPrivate& other) const
{
    return
        grouping == other.grouping &&
        filtermode == other.filtermode &&
        randomizer == other.randomizer &&
        sorter == other.sorter &&
        needsRefresh == other.needsRefresh &&
        count == other.count &&
        displayRole == other.displayRole &&
        emptyDisplayRole == other.emptyDisplayRole &&
        filterState == other.filterState &&
        searchTitle == other.searchTitle &&
        searchUrl == other.searchUrl &&
        searchArtist == other.searchArtist &&
        searchAlbum == other.searchAlbum &&
        searchGenre == other.searchGenre &&
        mimeTypes == other.mimeTypes &&
        mediaPlaylist == other.mediaPlaylist &&
        map == other.map &&
        sortOrder == other.sortOrder;

}

/*!
    \class QMediaList
    \inpublicgroup QtMediaModule


    \brief The QMediaList class provides a filtered list of media content.

    \ingroup multimedia

    The QMediaList class is useful for displaying media content filtered by Artist, Album, Genre etc. For example to display a list of music by artist name containing "Billy".

    \code
        mediaPlaylist = QMediaList();
        itemview->setModel( mediaPlaylist );
        mediaPlaylist.clearFilter();
        mediaPlaylist.setDisplayRole(QMediaList::Title);
        mediaPlaylist.beginFilter();
        mediaList.setFilter(QMediaList::MimeType,"audio");
        mediaList.addFilter(QMediaList::Artist, "Billy");
        mediaList.randomize();
        mediaList.endFilter();
    \endcode
*/

/*!
    \enum QMediaList::Roles

    \value Title           Title of media
    \value Url             Filename, http address
    \value Artist          Artist of media
    \value Album           Album of media
    \value Genre           Genre of media
    \value MimeType        MimeType of media
*/

/*!
    \enum QMediaList::Properties
    \value ShowEmpty          Display empty field when no data is available.
    \value ShowEmptyAsUnknown Display Unknown when empty field.
    \value HideEmpty          Do not display if field has no data.
*/

/*!
    \enum QMediaList::SortDirection
    \value Ascending       Ascending order sorting.
    \value Descending      Descending order sorting.
    \value Unsorted        No sorting.
*/

/*!
    \enum QMediaList::Grouping
    \value ShowAll         Display All entries.
    \value ShowGrouped     Display a unique list.
*/

/*!
    Constructs a list from the Document System.
*/

QMediaList::QMediaList()
    : QAbstractListModel()
{
    d = new QMediaListPrivate(this);
    clearFilter();
}

/*!
    Constructs a QMediaList from a QMediaPlaylist \a playlist.
*/

QMediaList::QMediaList(const QMediaPlaylist& playlist)
    : QAbstractListModel()
{
    d = new QMediaListPrivate(this);
    setModel(playlist);
}

/*!
    Constructs a copy of the \a other QMediaList.
*/

QMediaList::QMediaList(const QMediaList& other)
    : QAbstractListModel()
{
    d = new QMediaListPrivate(this);
    *this=other;
}

/*!
    \reimp
*/
int QMediaList::rowCount(const QModelIndex &) const
{
    if(d->needsRefresh == true) {
        const_cast<QMediaList*>(this)->refreshData();
    }
    return d->count;
}

/*!
    \reimp
*/
QVariant QMediaList::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(index.row() >= d->count)
        return QVariant();

    if(d->needsRefresh == true) {
        const_cast<QMediaList*>(this)->refreshData();
    }

    if(role == Qt::DisplayRole) {
        if(d->emptyDisplayRole != QMediaList::HideEmpty) {
            if(d->mediaPlaylist.data(d->map[index.row()],d->displayRole).toString().length() == 0) {
                if(d->emptyDisplayRole == QMediaList::ShowEmpty)
                    return QVariant();
                else
                    return QVariant("<Unknown>");
            } else
            return d->mediaPlaylist.data(d->map[index.row()],d->displayRole);
        } else
            return d->mediaPlaylist.data(d->map[index.row()],d->displayRole);
    } else
        return d->mediaPlaylist.data(d->map[index.row()], role);
}

/*!
    \reimp
*/

QVariant QMediaList::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

/*!
    Set filter mode to create, filter will not be applied until endFilter() is called.
*/
void QMediaList::beginFilter()
{
    d->filtermode = true;
}

/*!
    Resets current filter.
*/
void QMediaList::clearFilter()
{
    d->sortOrder.clear();
    if(d->filterState != 0) {
        d->filterState = 0;
        d->randomizer  = false;

        if(!d->filtermode)
            endFilter();
    }
}

/*!
    Removes filter on Category \a filterType

    eg. removeFilter(QMediaList::Album);

    Returns false on error.
*/
bool QMediaList::removeFilter(Roles filterType)
{
    if((d->filterState & filterType) != 0) {
        d->filterState &= ~filterType;

        if(!d->filtermode)
            endFilter();
        return true;
    }
    return false;
}

/*!
    Resets current filter, applying new filter on Category \a filterType containing string \a text

    eg. setFilter(QMediaList::MimeType,"audio");

    Returns false on error.
*/
bool QMediaList::setFilter(Roles filterType, const QString& text)
{
    clearFilter();
    return addFilter(filterType, text);
}

/*!
    Appends new filter on Category \a filterType containing string \a text

    eg. addFilter(QMediaList::Title,"Billy");

    Returns false on error.
*/
bool QMediaList::addFilter(Roles filterType, const QString& text)
{
    switch(filterType)
    {
        case QMediaList::Title:
            d->searchTitle.setPattern(text);
            d->searchTitle.setPatternSyntax(QRegExp::Wildcard);
            d->filterState |= QMediaList::Title;
            break;
        case QMediaList::Url:
            d->searchUrl.setPattern(text);
            d->searchUrl.setPatternSyntax(QRegExp::Wildcard);
            d->filterState |= QMediaList::Url;
            break;
        case QMediaList::Artist:
            d->searchArtist.setPattern(text);
            d->searchArtist.setPatternSyntax(QRegExp::Wildcard);
            d->filterState |= QMediaList::Artist;
            break;
        case QMediaList::Album:
            d->searchAlbum.setPattern(text);
            d->searchAlbum.setPatternSyntax(QRegExp::Wildcard);
            d->filterState |= QMediaList::Album;
            break;
        case QMediaList::Genre:
            d->searchGenre.setPattern(text);
            d->searchGenre.setPatternSyntax(QRegExp::Wildcard);
            d->filterState |= QMediaList::Genre;
            break;
        case QMediaList::MimeType:
            QString str = text + "/\\S+";
            d->mimeTypes = QMediaContent::supportedMimeTypes().filter(
                    QRegExp( QLatin1String( str.toLocal8Bit().constData() ) ) );
            d->filterState |= QMediaList::MimeType;
    }
    if(!d->filtermode)
        endFilter();

    return true;
}

/*!
    \fn void QMediaList::endFilter()

    Applies current filter to data.
*/
void QMediaList::endFilter()
{
    d->filtermode = false;

    d->needsRefresh = true;
    reset();
}

/*!
    Returns true if filters are in effect on the model.
*/
bool QMediaList::isFiltered() const
{
    return d->filterState != 0;
}

/*!
    \fn void QMediaList::randomize()

    Randomizes the filtered list.
*/
void QMediaList::randomize()
{
    if(d->randomizer == false) {
        d->randomizer = true;
        d->needsRefresh = true;
        reset();
    }
}

/*!
    Sorts the list by \a sortType
*/
void QMediaList::setSorting(SortDirection sortType)
{
    if(d->sorter != sortType) {
        d->sorter = sortType;
        d->needsRefresh = true;
        reset();
    }
}

/*!
    Sorts the list by \a sortType, using sort order \a roles
*/
void QMediaList::setSorting(QList<int> roles, SortDirection sortType)
{
    if(d->sorter != sortType || d->sortOrder != roles) {
        d->sorter = sortType;
        d->sortOrder = roles;
        d->needsRefresh = true;
        reset();
    }
}

/*!
    \fn void QMediaList::setDisplayRole(int role, Grouping grouping)

    Set what \a role is currently begin displayed. Setting the
    \a grouping changes the display from unique to all.

    Options for \a role are: QMediaPlaylist::DataRole

    eg. setDisplayRole(QMediaPlaylist::Title, QMediaList::ShowAll);
*/
void QMediaList::setDisplayRole(int role, Grouping grouping)
{
    if(d->displayRole != role || d->grouping != grouping) {
        d->displayRole = role;
        d->grouping = grouping;
        d->needsRefresh = true;
        reset();
    }
}

/*!
    Returns the media lists default display role

    \sa Qt::DisplayRole
*/
int QMediaList::displayRole() const
{
    return d->displayRole;
}

/*!
    Returns whether to display all, or to aggregate the data type set by setDisplayRole()
*/
QMediaList::Grouping QMediaList::displayGrouping() const
{
    return d->grouping;
}

/*!
    \fn void QMediaList::showEmpty(Properties role)

    Set \a role to handle empty data field.

    Options for \a role are: QMediaList::Properties

    eg. showEmpty(QMediaList::HideEmpty);
*/
void QMediaList::showEmpty(Properties role)
{
    if(d->emptyDisplayRole != role) {
        d->emptyDisplayRole = role;
        d->needsRefresh = true;
        reset();
    }
}

/*!
    Sets the given model to be used as the data provider for the list to \a playlist
*/
void QMediaList::setModel(const QMediaPlaylist& playlist)
{
    if(d->mediaPlaylist != playlist) {
        d->mediaPlaylist = playlist;
        d->needsRefresh = true;
        connect(&playlist, SIGNAL(dataChanged(QModelIndex,QModelIndex)), d, SLOT(setNeedsRefresh()));
        connect(&playlist, SIGNAL(layoutChanged()), d, SLOT(setNeedsRefresh()));
        connect(&playlist, SIGNAL(modelReset()), d, SLOT(setNeedsRefresh()));
        connect(&playlist, SIGNAL(rowsInserted(QModelIndex,int,int)), d, SLOT(setNeedsRefresh()));
        connect(&playlist, SIGNAL(rowsRemoved(QModelIndex,int,int)), d, SLOT(setNeedsRefresh()));
        reset();
    }
}

/*!
    Applies current filters to data and refresh content of list.
*/
void QMediaList::refreshData()
{
    d->count = 0;
    d->map.clear();

    for(int i=0;i<d->mediaPlaylist.rowCount();i++) {
        if(d->filterState & QMediaList::MimeType) {
            bool check = false;
            foreach( const QString &mimeType, d->mimeTypes ) {
                if(d->mediaPlaylist.content(d->mediaPlaylist.index(i,0)).type().contains(mimeType) > 0) {
                    check = true;
                }
            }
            if(!check)
                continue;
        }
        if(d->filterState & QMediaList::Genre) {
            if(!d->searchGenre.exactMatch(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0), QMediaPlaylist::Genre).toString()))
                continue;
        }
        if(d->filterState & QMediaList::Album) {
            if(!d->searchAlbum.exactMatch(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0), QMediaPlaylist::Album).toString()))
                continue;
        }
        if(d->filterState & QMediaList::Artist) {
            if(!d->searchArtist.exactMatch(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0), QMediaPlaylist::Artist).toString()))
                continue;
        }
        if(d->filterState & QMediaList::Url) {
            if(!d->searchUrl.exactMatch(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0), QMediaPlaylist::Url).toString()))
                continue;
        }
        if(d->filterState & QMediaList::Title) {
            if(!d->searchTitle.exactMatch(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0), QMediaPlaylist::Title).toString()))
                continue;
        }
        if(d->emptyDisplayRole == QMediaList::HideEmpty) {
            if(d->mediaPlaylist.data(d->mediaPlaylist.index(i,0),d->displayRole).toString().length() == 0)
                continue;
        }
        if((d->grouping == QMediaList::ShowAll) || (d->filterState & QMediaList::Title) || (d->filterState & QMediaList::Url)) {
            d->map[d->count] = d->mediaPlaylist.index(i,0);
            d->count++;
        } else {
            //Grouping List
            bool check = false;
            for(int k=0;k<d->count;k++) {
                if(d->mediaPlaylist.data(d->map[k],d->displayRole).toString().length() ==
                        d->mediaPlaylist.data(d->mediaPlaylist.index(i,0),d->displayRole).toString().length()) {
                    if((d->count != 0) && !check) {
                        if(d->mediaPlaylist.data(d->map[k],d->displayRole).toString().contains(
                                    d->mediaPlaylist.data(d->mediaPlaylist.index(i,0),d->displayRole).toString()) > 0) {
                            check = true;
                        }
                    } else
                        break;
                }
            }
            if(!check) {
                d->map[d->count] = d->mediaPlaylist.index(i,0);
                d->count++;
            }
        }
    }
    if(d->randomizer) {
        long        seed;
        QModelIndex indx;
        for(int i = 0;i<d->count;i++) {
            srandom(QTime::currentTime().msec()+i);
            seed = random() % d->count;
            indx = d->map[i];
            d->map[i] = d->map[seed];
            d->map[seed] = indx;
        }
        d->randomizer = false;
    } else if(d->sorter != Unsorted) {
            QModelIndex indx;
            for(int i = 0;i<d->count-1;i++) {
                for(int j=i+1;j<d->count;j++) {
                    bool swap = false;
                    QString str1(d->mediaPlaylist.data(d->map[i],d->displayRole).toString().toLower());
                    QString str2(d->mediaPlaylist.data(d->map[j],d->displayRole).toString().toLower());

                    if(d->sorter == Ascending) {
                        if(d->sortOrder.size() > 0) {
                            for(int k=0;k<d->sortOrder.size();k++) {
                                str1 = d->mediaPlaylist.data(d->map[i],d->sortOrder.at(k)).toString().toLower();
                                str2 = d->mediaPlaylist.data(d->map[j],d->sortOrder.at(k)).toString().toLower();
                                if(str1 < str2) {
                                    swap = true;
                                    break;
                                } else if(str1 > str2) {
                                    break;
                                }
                            }
                        } else
                            if(str1 < str2)
                                swap = true;
                    } else {
                        if(d->sortOrder.size() > 0) {
                            for(int k=0;k<d->sortOrder.size();k++) {
                                str1 = d->mediaPlaylist.data(d->map[i],d->sortOrder.at(k)).toString().toLower();
                                str2 = d->mediaPlaylist.data(d->map[j],d->sortOrder.at(k)).toString().toLower();
                                if(str1 > str2) {
                                    swap = true;
                                    break;
                                } else if(str1 < str2)
                                    break;
                            }
                        } else
                            if(str1 > str2)
                                swap = true;
                    }
                    if(swap) {
                        indx = d->map[i];
                        d->map[i] = d->map[j];
                        d->map[j] = indx;
                    }
                }
            }
    }
    d->needsRefresh = false;
}

/*!
    Constructs a QMediaPlaylist from the selected \a index, or if index is empty, then from the whole QMediaList.
*/
QMediaPlaylist QMediaList::playlist(const QModelIndex &index) const
{
    QMediaPlaylist result;
    if(index.isValid()) {
        Roles role;
        // add a filter for the indexed entry, and return playlist();
        switch(d->displayRole) {
            case QMediaPlaylist::Url:
                role = Url;
                break;
            case QMediaPlaylist::Artist:
                role = Artist;
                break;
            case QMediaPlaylist::Album:
                role = Album;
                break;
            case QMediaPlaylist::Genre:
                role = Genre;
                break;
            case QMediaPlaylist::Title:
            default:
                role = Title;
                break;
        }
        QMediaList internalList(*this);
        internalList.addFilter(role, data(index, d->displayRole).toString());
        result=internalList.playlist();
    }
    else {
        QMediaList internalList(*this);
        internalList.setDisplayRole(d->displayRole, ShowAll);
        internalList.refreshData();
        QStringList filelist;
        foreach(const QModelIndex &iter, internalList.d->map.values()) {
            filelist << internalList.d->mediaPlaylist.content(iter).fileName();
        }
        if(filelist.count() != 0)
            result=QMediaPlaylist(filelist);
    }
    return result;
}

/*!
    Assigns the specified \a medialist to this object.
*/
QMediaList &QMediaList::operator=(const QMediaList &medialist)
{
    *d=*(medialist.d);
    return *this;
}

/*!
    Returns true if this QMediaList and the \a other are equal; otherwise returns false.
*/
bool QMediaList::operator==(const QMediaList &other) const
{
    return *d==*(other.d);
}

#include "qmedialist.moc"
