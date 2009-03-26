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

#include <QAction>
#include <QList>
#include <QIcon>
#include <QAbstractListModel>

#include "qmediamenu.h"

class QAbstractMediaMenuItemPrivate
{
public:
    QAbstractMediaMenuItemPrivate()
        :level(0),icon(0),data(0),role(QMediaList::Title)
    {
    }

    int                             level;
    QIcon*                          icon;
    QString                         text;
    QString                         filter;
    QMediaList*                     data;
    QMediaList::Roles               role;
};

/*!
    \class QAbstractMediaMenuItem
    \inpublicgroup QtMediaModule


    \brief The QAbstractMediaMenuItem class provides a base class for QMediaMenuItem

    \ingroup multimedia

    The QAbstractMediaMenuItem class is used to create a custom menu item used with a QMediaMenu
*/

/*!
    Constructs item with \a role set to QMediaList::Roles
    and a QMediaList of \a data
*/
QAbstractMediaMenuItem::QAbstractMediaMenuItem(QMediaList::Roles role, QMediaList* data)
    :prev(0),d(new QAbstractMediaMenuItemPrivate)
{
    d->role  = role;
    d->data  = data;

    switch(role) {
        case QMediaList::Title:    d->text = "Songs  ";  break;
        case QMediaList::Artist:   d->text = "Artists";  break;
        case QMediaList::Album:    d->text = "Albums ";  break;
        case QMediaList::Genre:    d->text = "Genres ";  break;
        case QMediaList::Url:
        case QMediaList::MimeType:
            break;
    }
}

/*!
    Constructs item with \a role set to QMediaList::Roles,
    filtered by \a filter
    and a QMediaList of \a data
*/
QAbstractMediaMenuItem::QAbstractMediaMenuItem(QMediaList::Roles role,QString filter, QMediaList* data)
    :prev(0),d(new QAbstractMediaMenuItemPrivate)
{
    d->role   = role;
    d->filter = filter;
    d->data   = data;

    switch(role) {
        case QMediaList::Title:    d->text = "Title:  "; break;
        case QMediaList::Artist:   d->text = "Artist: "; break;
        case QMediaList::Album:    d->text = "Album:  "; break;
        case QMediaList::Genre:    d->text = "Genres: "; break;
        case QMediaList::Url:
        case QMediaList::MimeType:
            break;
    }
    d->text.append(filter);
}

/*!
    Constructs item with QIcon \a icon
    display text \a text
    and a QMediaList of \a data
*/
QAbstractMediaMenuItem::QAbstractMediaMenuItem(QIcon* icon, QString text, QMediaList* data)
    :prev(0),d(new QAbstractMediaMenuItemPrivate)
{
    d->text   = text;
    d->icon   = icon;
    d->data   = data;
}

/*!
    Constructs item
*/
QAbstractMediaMenuItem::QAbstractMediaMenuItem()
    :prev(0),d(new QAbstractMediaMenuItemPrivate)
{
}

/*!
    Add \a item to items submenu
*/
void QAbstractMediaMenuItem::add(QAbstractMediaMenuItem* item)
{
    menu.append( item );

    item->prev = this;

    connect(item,SIGNAL(dataChanged()),this,SIGNAL(dataChanged()));
}

/*!
    Remove \a item from items submenu
*/
void QAbstractMediaMenuItem::remove(QAbstractMediaMenuItem* item)
{
    for(int i = 0;i<menu.size();i++) {
        if(item == menu.at(i)) {
            menu.removeAt(i);
            return;
        }
    }
}

/*!
    Set the QMediaList \a l as the data source for item
*/
void QAbstractMediaMenuItem::setData(QMediaList* l)
{
    d->data = l;
}

/*!
    Set the items level to \a l
*/
void QAbstractMediaMenuItem::setLevel(int l)
{
    d->level = l;
}

/*!
    Returns current level
*/
int QAbstractMediaMenuItem::level()
{
    return d->level;
}

/*!
    Returns current icon, null if no icon
*/
QIcon* QAbstractMediaMenuItem::icon()
{
    return d->icon;
}

/*!
    Returns current display text
*/
QString QAbstractMediaMenuItem::text()
{
    return d->text;
}

/*!
    Returns current filter string
*/
const QString& QAbstractMediaMenuItem::filter()
{
    return d->filter;
}

/*!
    Returns current QMediaList
*/
QMediaList* QAbstractMediaMenuItem::data()
{
    return d->data;
}

/*!
    Returns current display Role
*/
QMediaList::Roles QAbstractMediaMenuItem::displayRole()
{
    return d->role;
}

/*!
    \fn QAbstractMediaMenuItem::dataChanged()
    This signal is emitted when display contents has changed.
*/

/*!
    \fn bool QAbstractMediaMenuItem::execute()

    Returns true if next selection is to be emitted
    This function is executed when item is selected but has no submenu
*/

/*!
    \fn QSize QAbstractMediaMenuItem::size()

    Returns size hint for item display, QSize(-1,-1) to use defaults
*/

/*!
    \fn void QAbstractMediaMenuItem::paint(QPainter* painter, const QStyleOptionViewItem& option)

    Implement for custom menu items, handle your own drawing
    \a painter QPainter
    \a option  QStyleOptionViewItem

    Implement size() to return valid QSize
*/

class QMediaMenuItemPrivate
{
public:
    QMediaMenuItemPrivate()
        :lvl(0),selection(false)
    {
    }

    void videos(QMediaList* data)
    {
        //Show list of videos
        data->clearFilter();
        data->setDisplayRole(QMediaPlaylist::Title);
        data->setFilter(QMediaList::MimeType,"video");
        selection = true;
        lvl = 0;
    }

    void artists(int level, QMediaList* data)
    {
        if(level < lvl) {
            //Back
            filter.clear();
            if(!albumFilter.isEmpty())
                albumFilter.clear();
            else if(!artistFilter.isEmpty())
                artistFilter.clear();
        }

        data->beginFilter();
        data->clearFilter();
        data->setFilter(QMediaList::MimeType,"audio");

        switch(level) {
            case 0:
                //List of Artists
                resetFilters();
                data->setDisplayRole(QMediaPlaylist::Artist);
                selection = false;
                break;
            case 1:
                //Artist x's Albums
                if(!filter.isEmpty())
                    artistFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Album);
                data->addFilter(QMediaList::Artist,artistFilter);
                selection = false;
                break;
            case 2:
                //Song List for Artist/Album
                if(!filter.isEmpty())
                    albumFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Title);
                data->addFilter(QMediaList::Artist,artistFilter);
                data->addFilter(QMediaList::Album,albumFilter);
                selection = true;
                break;
        }
        data->endFilter();
        lvl = level;
    }

    void albums(int level, QMediaList* data)
    {
        if(level < lvl) {
            //Back
            filter.clear();
            albumFilter.clear();
        }

        data->beginFilter();
        data->clearFilter();
        data->setFilter(QMediaList::MimeType,"audio");

        switch(level) {
            case 0:
                //List of Albums
                resetFilters();
                data->setDisplayRole(QMediaPlaylist::Album);
                selection = false;
                break;
            case 1:
                //Song List for Album
                if(!filter.isEmpty())
                    albumFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Title);
                data->addFilter(QMediaList::Album,albumFilter);
                selection = true;
                break;
        }
        data->endFilter();
        lvl = level;
    }

    void genres(int level, QMediaList* data)
    {
        if(level < lvl) {
            //Back
            filter.clear();
            if(!albumFilter.isEmpty())
                albumFilter.clear();
            else if(!artistFilter.isEmpty())
                artistFilter.clear();
            else
                genreFilter.clear();
        }

        data->beginFilter();
        data->clearFilter();
        data->setFilter(QMediaList::MimeType,"audio");

        switch(level) {
            case 0:
                //List of Genres
                resetFilters();
                data->setDisplayRole(QMediaPlaylist::Genre);
                selection = false;
                break;
            case 1:
                //Artist x's in Genre
                if(!filter.isEmpty())
                    genreFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Artist);
                data->addFilter(QMediaList::Genre,genreFilter);
                selection = false;
                break;
            case 2:
                //Albums by Genre/Artist
                if(!filter.isEmpty())
                    artistFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Album);
                data->addFilter(QMediaList::Genre,genreFilter);
                data->addFilter(QMediaList::Artist,artistFilter);
                selection = false;
                break;
            case 3:
                //Song List for Genre/Artist/Album
                if(!filter.isEmpty())
                    albumFilter = filter;
                data->setDisplayRole(QMediaPlaylist::Title);
                data->addFilter(QMediaList::Genre,genreFilter);
                data->addFilter(QMediaList::Artist,artistFilter);
                data->addFilter(QMediaList::Album,albumFilter);
                selection = true;
                break;
        }
        data->endFilter();
        lvl = level;
    }

    void songs(QMediaList* data)
    {
        //Show list of songs
        data->clearFilter();
        data->setDisplayRole(QMediaPlaylist::Title);
        data->setFilter(QMediaList::MimeType,"audio");
        selection = true;
    }

    void resetFilters()
    {
        filter.clear();
        artistFilter.clear();
        albumFilter.clear();
        genreFilter.clear();
    }

    int      lvl;
    bool     selection;

    QString  filter;
    QString  artistFilter;
    QString  albumFilter;
    QString  genreFilter;
};

/*!
    \class QMediaMenuItem
    \inpublicgroup QtMediaModule


    \brief The QMediaMenuItem class is used with QMediaMenu to provide Multimedia menus

    \ingroup multimedia

    The QMediaMenuItem class provides filtered menus items by Artist, Album, Genre etc.
*/

/*!
    Constructs item with \a role set to QMediaList::Roles
    and a QMediaList of \a data
*/
QMediaMenuItem::QMediaMenuItem(QMediaList::Roles role, QMediaList* data)
    :QAbstractMediaMenuItem(role,data),dd(new QMediaMenuItemPrivate)
{
}

/*!
    Constructs item with \a role set to QMediaList::Roles,
    filtered by \a filter
    and a QMediaList of \a data
*/
QMediaMenuItem::QMediaMenuItem(QMediaList::Roles role,QString filter, QMediaList* data)
    :QAbstractMediaMenuItem(role,filter,data),dd(new QMediaMenuItemPrivate)
{
}

/*!
    Constructs item with QIcon \a icon
    display text \a text
    and a QMediaList of \a data
*/
QMediaMenuItem::QMediaMenuItem(QIcon* icon, QString text, QMediaList* data)
    :QAbstractMediaMenuItem(icon,text,data),dd(new QMediaMenuItemPrivate)
{
}

/*!
    Constructs item
*/
QMediaMenuItem::QMediaMenuItem()
    :QAbstractMediaMenuItem(),dd(new QMediaMenuItemPrivate)
{
}
/*!
    Function executed when item selected with no submenu
*/
bool QMediaMenuItem::execute()
{
    if(text().contains("Videos") && data()) {
        dd->videos(data());
        emit dataChanged();
        return true;
    } else if(displayRole() == QMediaList::Title && data()) {
        dd->songs(data());
        emit dataChanged();
        return true;
    } else if(displayRole() == QMediaList::Artist) {

        if(level() && dd->filter.isEmpty()) {
            //Navigate Back
            switch(level()) {
                case 3: setLevel(1); break;
                case 2: setLevel(0); break;
            }
        }
        if(!level()) dd->resetFilters();
        dd->artists(level(),data());
        setLevel(level()+1);
        emit dataChanged();

    } else if(displayRole() == QMediaList::Album) {

        if(level() && dd->filter.isEmpty()) {
            //Navigate Back
            setLevel(0);
            dd->resetFilters();
        }
        dd->albums(level(),data());
        setLevel(level()+1);
        emit dataChanged();

    } else if(displayRole() == QMediaList::Genre) {

        if(level() && dd->filter.isEmpty()) {
            //Navigate Back
            switch(level()) {
                case 4: setLevel(2); break;
                case 3: setLevel(1); break;
                case 2: setLevel(0); break;
            }
        }
        if(!level()) dd->resetFilters();
        dd->genres(level(),data());
        setLevel(level()+1);
        emit dataChanged();

    }
    return dd->selection;
}

/*!
    Returns QSize(-1,-1), QMediaMenuItem uses defaults
    This is implemented when creating a custom item from
    QAbstractMediaMenuItem instead of using QMediaMenuItem.
*/
QSize QMediaMenuItem::size()
{
    return QSize(-1,-1);
}

/*!
    unused, QMediaMenuItem uses defaults
    This is implemented when creating a custom item from
    QAbstractMediaMenuItem instead of using QMediaMenuItem.
    \a painter QPainter
    \a option  QStyleOptionViewItem
*/
void QMediaMenuItem::paint(QPainter* painter, const QStyleOptionViewItem& option)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
}

/*!
    Set current filter string to \a str
*/
void QMediaMenuItem::filterBy(QString str)
{
    dd->filter = str;
}


class QMediaMenuPrivate : public QAbstractListModel
{
    Q_OBJECT
public:
    QMediaMenuPrivate(QObject *parent)
        :select(false)
    {
        Q_UNUSED(parent);
        baseItem    = new QMediaMenuItem();
        currentItem = baseItem;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return currentItem->menu.size();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        if(role == Qt::DecorationRole)
            if(currentItem->menu.at(index.row())->icon())
                return QVariant(*currentItem->menu.at(index.row())->icon());
            else
                return QVariant();
        else if(role == Qt::DisplayRole)
            return QVariant(currentItem->menu.at(index.row())->text());
        else
            return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
            return QString("Column %1").arg(section);
        else
            return QString("Row %1").arg(section);
    }

    QMediaMenuItem* item(const QModelIndex &index)
    {
        return (QMediaMenuItem*)currentItem->menu.at(index.row());
    }

    void add(QMediaMenuItem* item)
    {
        currentItem->menu.append( item );
    }

    void remove(QMediaMenuItem* item)
    {
        for(int i = 0;i<currentItem->menu.size();i++) {
            if(item == currentItem->menu.at(i)) {
                currentItem->menu.removeAt(i);
                return;
            }
        }
    }

    void resetMenu()
    {
        select      = false;
        currentItem = baseItem;
    }

    bool                   select;
    QString                currentSelection;
    QMediaMenuItem*        currentItem;
    QStringList            plist;

private:
    QMediaMenuItem*        baseItem;
};


class QMediaMenuDelegatePrivate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    QMediaMenuDelegatePrivate(QObject *parent)
    {
        menu = qobject_cast<QMediaMenu*>(parent);
        standard = menu->itemDelegate();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if(menu->current()->menu.size() == 0)
            return standard->sizeHint(option,index);

        if(menu->current()->menu.at(index.row())->size().isValid())
            return menu->current()->menu.at(index.row())->size();
        else
            return standard->sizeHint(option,index);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if(menu->current()->menu.size() == 0)
            return;

        if(menu->current()->menu.at(index.row())->size().isValid())
            menu->current()->menu.at(index.row())->paint(painter, option);
        else
            standard->paint(painter, option, index);
    }

    QMediaMenu*             menu;
    QAbstractItemDelegate*  standard;
};

/*!
    \class QMediaMenu
    \inpublicgroup QtMediaModule


    \brief The QMediaMenu class provides a menu system for browsing media content.

    \ingroup multimedia

    The QMediaMenu class is useful for displaying media content filtered by Artist, Album, Genre etc.

    \code
        QVBoxLayout* layout = new QVBoxLayout;

        QMediaPlaylist playlist(QContentFilter(QContent::Document));
        medialist = new QMediaList(playlist);

        QMediaMenu* mainmenu = new QMediaMenu();

        layout->addWidget(mainmenu);
        setLayout(layout);

        QMediaMenuItem* a = new QMediaMenuItem(new QIcon(":icon/mediaplayer/black/music"), QString("Music"), 0);
        mainmenu->add(a);

        a->add(new QMediaMenuItem(QMediaList::Artist, medialist));
        a->add(new QMediaMenuItem(QMediaList::Album, medialist));
        a->add(new QMediaMenuItem(QMediaList::Genre, medialist));
        a->add(new QMediaMenuItem(QMediaList::Title, medialist));

        QMediaMenuItem* b = new QMediaMenuItem(new QIcon(":icon/mediaplayer/black/videos"), QString("Videos"), medialist);
        mainmenu->add(b);

        QMediaMenuItem* c = (QMediaMenuItem*) new CustomMenuItem(new QIcon(":icon/mediaplayer/black/videos"), QString("Custom"), 0);
        mainmenu->add(c);

        mainmenu->resetMenu();
    \endcode
    dataChanged() is emitted when display content changes
    see signals dataChanged()
*/

/*!
    Constructs a menu with \a parent
*/
QMediaMenu::QMediaMenu(QWidget* parent)
    : QListView(parent)
{
    d = new QMediaMenuPrivate(this);

    delegate = new QMediaMenuDelegatePrivate(this);

    setItemDelegate(delegate);

    connect( this, SIGNAL(activated(QModelIndex)),
            this, SLOT(selection(QModelIndex)) );
}

/*!
    Constructs a menu
*/
QMediaMenu::~QMediaMenu()
{
    delete d;
    delete delegate;
}

/*!
    Add \a item to menu
*/
void QMediaMenu::add(QMediaMenuItem* item)
{
    // Add a new menu entry
    d->add(item);
    connect(item,SIGNAL(dataChanged()),this,SLOT(refreshData()));
}

/*!
    Remove \a item from menu
*/
void QMediaMenu::remove(QMediaMenuItem* item)
{
    // Remove menu entry
    d->remove(item);
}

/*!
    Reset menu back to base menu
*/
void QMediaMenu::resetMenu()
{
    d->resetMenu();
    setItemDelegate(delegate);
    setModel(d);
    reset();
}

/*!
    Return current menu item
*/
QMediaMenuItem* QMediaMenu::current()
{
    return d->currentItem;
}

/*!
    \internal
*/
void QMediaMenu::next()
{
    if(!d->currentItem->level() && d->currentItem->menu.size())
        refreshData();
    else {
        setItemDelegate(delegate->standard);
        if(d->currentSelection.length() > 0)
            d->currentItem->filterBy(d->currentSelection);
        d->select = d->currentItem->execute();
    }
}

/*!
    \internal
*/
void QMediaMenu::prev()
{
    if(d->currentItem->level()<2 && d->currentItem->prev) {
        d->currentItem = (QMediaMenuItem*)d->currentItem->prev;
        setItemDelegate(delegate);
        refreshData();
    } else if(d->currentItem->level() > 0) {
        //Go back a filter level
        d->currentItem->filterBy("");
        d->currentItem->execute();
        d->select = false;
    } else
        resetMenu();
}

/*!
    Updates data and display
*/
void QMediaMenu::refreshData()
{
    if(d->currentItem->data()) {
        setItemDelegate(delegate->standard);
        setModel(d->currentItem->data());
    } else {
        setItemDelegate(delegate);
        setModel(d);
    }
    reset();
}

/*!
    \internal
*/
void QMediaMenu::selection(const QModelIndex& index)
{
    if(d->currentItem && d->currentItem->data())
        d->currentSelection = d->currentItem->data()->data(index,QMediaPlaylist::Title).toString();

    if(d->select) {
        // Selection made
        d->currentItem->setLevel(0);
        d->plist = QStringList(d->currentItem->data()->data(index,QMediaPlaylist::Title).toString());
        emit playlist(d->plist);
        resetMenu();
    } else {
        if(!d->currentItem->level()) {
            d->currentItem = d->item(index);
            d->currentItem->setLevel(0);
        }
    }
    next();
}

/*!
    \fn void QMediaMenu::playlist(const QMediaPlaylist& plist)
    This signal is emitted when a selection has been made
    \a plist is a QMediaPlaylist containing the selection.
*/

#include "qmediamenu.moc"
