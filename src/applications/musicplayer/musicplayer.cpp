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

#include "musicplayer.h"
#include "albumview.h"
#include "sidebar.h"
#include <Qtopia>
#include <QMediaPlaylist>
#include <QMediaList>
#include <QMediaContent>
#include <QRibbonSelector>
#include <QtopiaAbstractService>
#include <QFile>
#include <QMediaContent>
#include <QMediaControl>
#include <QSmoothList>
#include <private/homewidgets_p.h>

class MusicPlayerListViewDelegate;

class hackedQMediaPlaylist : public QMediaPlaylist {
    Q_OBJECT
public:
    hackedQMediaPlaylist() : QMediaPlaylist() { }
//     hackedQMediaPlaylist(const QMediaPlaylist& in) : QMediaPlaylist(in) {}
    void emitDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) { emit dataChanged(topLeft, bottomRight); }
    QMediaPlaylist& operator=( const QMediaPlaylist& other ) { return QMediaPlaylist::operator=(other); }

};

class MusicPlayerPrivate
{
public:
    MusicPlayerPrivate();
    hackedQMediaPlaylist currentPlaylist;
    QMediaList mediaList;
    QMediaPlaylist allMediaList;
    QSmoothList *listbox;
    QContentSet playlistContentSet;
    QContentSetModel playlistContentSetModel;
    QRibbonSelector *ribbonSelector;
    AlbumView *albumView;
    SideBar *sidebar;
    QChar lastIndex;
    QWidget *container;
    QMediaContent *mediaContent;
    QMediaControl *mediaControl;
    QModelIndex pressedIndex;
    QDateTime pressedTime;
    MusicPlayerListViewDelegate *listViewDelegate;
    QHBoxLayout *titleBoxLayout;
    QLabel *titleLabel;
    HomeActionButton *backButton;
    enum ViewType { vtNowPlayingView, vtGenreView, vtAlbumCoverView, vtArtistView, vtAlbumView, vtPlaylistView };
    ViewType currentViewType;
    ViewType previousViewType;
};

MusicPlayerPrivate::MusicPlayerPrivate()
    : playlistContentSetModel(&playlistContentSet)
    , mediaContent(NULL)
    , mediaControl(NULL)
{
}

class MusicPlayerListViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MusicPlayerListViewDelegate(QObject *parent = 0)
        : QItemDelegate(parent)
    {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
    {
        QFontMetrics fm(titleFont(option));
        if(qobject_cast<MusicPlayer*>(parent())!= NULL)
            return QSize(1, qobject_cast<MusicPlayer*>(parent())->d->listbox->height()/5);
        else {
            qWarning() << "Bad parent! estimating size";
            return QSize(1, fm.height() * 6 >> 2);
        }
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        if (hasClipping())
            painter->setClipRect(option.rect);
        MusicPlayerPrivate::ViewType viewType = qobject_cast<MusicPlayer*>(parent())->d->currentViewType;

        const int margin = option.rect.height() / 10;

        QRect contentsRect = option.rect;
        contentsRect.adjust(margin, margin, -margin, -margin);

        if( viewType == MusicPlayerPrivate::vtAlbumView || viewType == MusicPlayerPrivate::vtPlaylistView || viewType == MusicPlayerPrivate::vtGenreView ) {
            if (option.state & QStyle::State_Selected)
            {
                QColor bg = qRgb(156, 153, 156);
                QLinearGradient bgg(option.rect.x(), option.rect.y(), option.rect.x(), option.rect.bottom());
                bgg.setColorAt(0.0f, bg);
                bgg.setColorAt(0.1f, bg);
                bgg.setColorAt(0.101f, bg.darker(120));
                bgg.setColorAt(0.9f, bg);
                bgg.setColorAt(0.901f, bg.darker(120));
                bgg.setColorAt(1.0f, bg.darker(120));
                painter->fillRect(option.rect, bgg);
                painter->setPen(Qt::white);
                QFont tmpfont=painter->font();
                tmpfont.setBold(true);
                painter->setFont(tmpfont);
                painter->drawText(contentsRect, Qt::AlignLeft|Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString());
            }
            else
            {
                QColor bg = Qt::black;
                painter->fillRect(option.rect, bg);
                painter->setPen(qRgb(172, 117, 41));
                painter->drawText(contentsRect, Qt::AlignLeft|Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString());
                QPen pen;
                pen.setWidth(2);
                pen.setStyle(Qt::DotLine);
                pen.setColor(qRgb(49, 97, 148));
                painter->setPen(pen);
                painter->drawLine(option.rect.left(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
            }
        } else if( qobject_cast<MusicPlayer*>(parent())->d->currentViewType == MusicPlayerPrivate::vtNowPlayingView ) {
            if (option.state & QStyle::State_Selected)
            {
                QColor bg = qRgb(156, 153, 156);
                QLinearGradient bgg(option.rect.x(), option.rect.y(), option.rect.x(), option.rect.bottom());
                bgg.setColorAt(0.0f, bg);
                bgg.setColorAt(0.1f, bg);
                bgg.setColorAt(0.101f, bg.darker(120));
                bgg.setColorAt(0.9f, bg);
                bgg.setColorAt(0.901f, bg.darker(120));
                bgg.setColorAt(1.0f, bg.darker(120));
                painter->fillRect(option.rect, bgg);
                painter->setPen(Qt::white);
                QFont tmpfont=painter->font();
                tmpfont.setBold(true);
                painter->setFont(tmpfont);
                QRect textRect = contentsRect;
                textRect.adjust(0, 0, -contentsRect.height() -margin, 0);
                painter->drawText(textRect, Qt::AlignLeft|Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString());
                QRect arrowRect = contentsRect;
                arrowRect.adjust(textRect.width() + margin, 0, 0, 0);
                QIcon arrow;
                MusicPlayer *player=qobject_cast<MusicPlayer*>(parent());
                if(player != NULL && player->d->mediaControl != NULL && index==player->d->currentPlaylist.playing()) {
                    if(player->d->mediaControl->playerState() == QtopiaMedia::Playing)
                        arrow = QIcon(QLatin1String(":icon/pause"));
                    else
                        arrow = QIcon(QLatin1String(":icon/play"));
                } else
                    arrow = QIcon(QLatin1String(":icon/play"));
                arrow.paint(painter, arrowRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
            }
            else
            {
                MusicPlayer *player=qobject_cast<MusicPlayer*>(parent());
                if(player != NULL && player->d->mediaControl != NULL && index==player->d->currentPlaylist.playing()) {
                    QFont font = painter->font();
                    font.setBold(true);
                    painter->setFont(font);
                }
                QColor bg = Qt::black;
                painter->fillRect(option.rect, bg);
                painter->setPen(qRgb(172, 117, 41));
                painter->drawText(contentsRect, Qt::AlignLeft|Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString());
                QPen pen;
                pen.setWidth(2);
                pen.setStyle(Qt::DotLine);
                pen.setColor(qRgb(172, 117, 41));
                painter->setPen(pen);
                painter->drawLine(option.rect.left(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
            }
        } else if( qobject_cast<MusicPlayer*>(parent())->d->currentViewType == MusicPlayerPrivate::vtArtistView ) {
            if (option.state & QStyle::State_Selected)
            {
                QColor bg = qRgb(156, 153, 156);
                QLinearGradient bgg(option.rect.x(), option.rect.y(), option.rect.x(), option.rect.bottom());
                bgg.setColorAt(0.0f, bg);
                bgg.setColorAt(0.1f, bg);
                bgg.setColorAt(0.101f, bg.darker(120));
                bgg.setColorAt(0.9f, bg);
                bgg.setColorAt(0.901f, bg.darker(120));
                bgg.setColorAt(1.0f, bg.darker(120));
                painter->fillRect(option.rect, bgg);
                painter->setPen(Qt::white);
                QFont tmpfont=painter->font();
                tmpfont.setBold(true);
                painter->setFont(tmpfont);
                QRect textRect = contentsRect;
                textRect.adjust(0, 0, -contentsRect.height() -margin, 0);
                painter->drawText(textRect, Qt::AlignLeft|Qt::AlignVCenter, index.model()->data(index, Qt::DisplayRole).toString());
                QRect arrowRect = contentsRect;
                arrowRect.adjust(textRect.width() + margin, 0, 0, 0);
                QIcon arrow(QLatin1String(":icon/right"));
                arrow.paint(painter, arrowRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
            }
            else
            {
                QColor bg = Qt::black;
                painter->fillRect(option.rect, bg);
                painter->setPen(qRgb(172, 117, 41));
                QString text=index.model()->data(index, Qt::DisplayRole).toString();
                QRect initialLetterRect = contentsRect;
                initialLetterRect.setWidth(painter->fontMetrics().width(text,1));
                QRect restOfStringRect = contentsRect;
                restOfStringRect.adjust(initialLetterRect.width(), 0, 0, 0);
                painter->drawText(initialLetterRect, Qt::AlignLeft|Qt::AlignVCenter, text.left(1));
                painter->setPen(qRgb(49, 97, 148));
                painter->drawText(restOfStringRect, Qt::AlignLeft|Qt::AlignVCenter, text.right(text.length()-1));
                QPen pen;
                pen.setWidth(2);
                pen.setStyle(Qt::DotLine);
                pen.setColor(qRgb(49, 97, 148));
                painter->setPen(pen);
                painter->drawLine(option.rect.left(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
            }
        }

        painter->restore();
    }

private:
    QFont titleFont(const QStyleOptionViewItem &option) const
    {
        QFont fmain = option.font;
        fmain.setWeight(QFont::Bold);
        return fmain;
    }
};

QStringList generateMusicMimeTypeList()
{
    QStringList musicMimeTypes = QMediaContent::supportedMimeTypes().filter( QRegExp( QLatin1String( "audio/\\S+" ) ) );
    if( musicMimeTypes.contains( QLatin1String( "audio/mpeg" ) ) ) {
        if( !musicMimeTypes.contains( QLatin1String( "audio/mpeg3" ) ) )
            musicMimeTypes.append( QLatin1String( "audio/mpeg3" ) );
        if( !musicMimeTypes.contains( QLatin1String( "audio/mp3" ) ) )
            musicMimeTypes.append( QLatin1String( "audio/mp3" ) );
        if( !musicMimeTypes.contains( QLatin1String( "audio/x-mp3" ) ) )
            musicMimeTypes.append( QLatin1String( "audio/x-mp3" ) );
    }
    musicMimeTypes.removeAll( QLatin1String( "audio/mpegurl" ) );
    musicMimeTypes.removeAll( QLatin1String( "audio/x-mpegurl" ) );
    musicMimeTypes.removeAll( QLatin1String( "audio/x-scpls" ) );
    return musicMimeTypes;
}

class MusicPlayerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    MusicPlayerService( QObject *parent )
        : QtopiaAbstractService( "MusicPlayer", parent )
        { publishAll(); }

public slots:
    void showNowPlaying();
    void showGenre();
    void showAlbums();
    void showArtists();
    void showPlaylists();
};

void MusicPlayerService::showNowPlaying()
{
    qobject_cast<MusicPlayer*>(parent())->viewNowPlaying();
}

void MusicPlayerService::showGenre()
{
    qobject_cast<MusicPlayer*>(parent())->viewGenre();
}

void MusicPlayerService::showAlbums()
{
    qobject_cast<MusicPlayer*>(parent())->viewAlbums();
}

void MusicPlayerService::showArtists()
{
    qobject_cast<MusicPlayer*>(parent())->viewArtists();
}

void MusicPlayerService::showPlaylists()
{
    qobject_cast<MusicPlayer*>(parent())->viewPlaylists();
}

MusicPlayer::MusicPlayer( QWidget* parent, Qt::WFlags f ):
    QWidget( parent, f )
    , d(new MusicPlayerPrivate)
{

    QPalette pal=palette();
    pal.setColor(QPalette::Window, Qt::black);
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    setPalette(pal);
    setAutoFillBackground(true);

    setWindowTitle( tr("Music Player") );

    QVBoxLayout *layout1=new QVBoxLayout(this);
    layout1->setMargin(0);
    layout1->setSpacing(1);

    d->ribbonSelector = new QRibbonSelector(NULL, Qt::Horizontal);
    pal = d->ribbonSelector->palette();
    pal.setColor(QPalette::WindowText, qRgb(0, 100, 150));
    d->ribbonSelector->setPalette(pal);
    connect(d->ribbonSelector, SIGNAL(indexSelected(int)), this, SLOT(ribbonClicked(int)));
    QStringList labels;

    for (char a='A'; a <= 'Z'; a++) {
        labels += QChar(a);
    }
    d->ribbonSelector->setLabels(labels);
    layout1->addWidget(d->ribbonSelector);

    d->container = new QWidget;
    d->titleBoxLayout = new QHBoxLayout;
    d->titleBoxLayout->setSpacing(0);
    d->titleBoxLayout->setMargin(0);
    d->backButton = new HomeActionButton(QtopiaHome::Red);
    d->backButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    d->backButton->setText(tr("Back"));
    connect(d->backButton, SIGNAL(clicked()), this, SLOT(doBack()));
    d->titleBoxLayout->addWidget(d->backButton);
    d->titleLabel = new QLabel;
    d->titleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    d->titleLabel->setIndent(style()->pixelMetric(QStyle::PM_ButtonMargin));
    d->titleBoxLayout->addWidget(d->titleLabel, 1);
    QFont font = d->titleLabel->font();
    font.setBold(true);
    d->titleLabel->setFont(font);
    d->titleLabel->setMinimumSize(10, (QFontMetrics(font).height()*3)/2);
    pal=d->titleLabel->palette();
    pal.setColor(QPalette::Window, qRgb(41, 93, 139));
    d->titleLabel->setPalette(pal);
    d->titleLabel->setVisible(false);
    d->backButton->setVisible(false);
    layout1->addLayout(d->titleBoxLayout);


    QHBoxLayout *layout2=new QHBoxLayout;
    layout2->setSpacing(1);
    layout2->setMargin(0);
    d->listbox = new QSmoothList;
    d->listViewDelegate = new MusicPlayerListViewDelegate(this);
    d->listbox->setItemDelegate(d->listViewDelegate);
    d->listbox->setOpaqueItemBackground(true);
    d->listbox->setSelectionMode(QSmoothList::SingleSelection);
    d->sidebar = new SideBar;
    connect(d->sidebar, SIGNAL(playButtonPushed()), this, SLOT(playButtonPushed()));
    connect(d->sidebar, SIGNAL(addToQueueButtonPushed()), this, SLOT(addToQueueButtonPushed()));
    layout2->addWidget(d->listbox, 1);
    layout2->addWidget(d->sidebar);
    d->container->setLayout(layout2);
    layout1->addWidget(d->container, 1);
    d->albumView = new AlbumView(this);
    layout1->addWidget(d->albumView, 1);
    d->albumView->setVisible(false);

    connect(d->albumView, SIGNAL(clicked(QModelIndex)), this, SLOT(doMediaListPlayNow(QModelIndex)));
    connect(&d->currentPlaylist, SIGNAL(playingChanged(QModelIndex)), this, SLOT(doPlayingChanged(QModelIndex)));
    connect(d->listbox, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(smoothListCurrentChanged(QModelIndex, QModelIndex)));

    setLayout(layout1);

    connect(&d->currentPlaylist, SIGNAL(modelReset()), this, SLOT(modelReset()));

    // load current playlist
    viewAlbums();

    new MusicPlayerService(this);

    QTimer::singleShot(1, this, SLOT(delayedInitialisations()));
}

MusicPlayer::~MusicPlayer()
{
    delete d;
}

void MusicPlayer::viewNowPlaying()
{
    d->albumView->setVisible(false);
    d->container->setVisible(true);
    d->listbox->setVisible(true);
    d->sidebar->setVisible(false);
    d->ribbonSelector->setVisible(false);
    d->titleLabel->setVisible(true);
    d->backButton->setVisible(true);
    // display the now playing list
    d->previousViewType=d->currentViewType;
    d->currentViewType = MusicPlayerPrivate::vtNowPlayingView;
    d->listbox->setModel(&d->currentPlaylist);
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(nowPlayingViewClicked(QModelIndex)));
}

void MusicPlayer::viewGenre()
{
    d->albumView->setVisible(false);
    d->container->setVisible(true);
    d->listbox->setVisible(true);
    d->sidebar->setVisible(false);
    d->ribbonSelector->setVisible(true);
    d->titleLabel->setVisible(false);
    d->backButton->setVisible(false);
    // display the genre list
    d->mediaList.setDisplayRole(QMediaPlaylist::Genre);
    d->mediaList.setSorting(QList<int>(), QMediaList::Descending);
    if(d->mediaList.isFiltered())
        d->mediaList.clearFilter();
    d->listbox->setModel(&d->mediaList);
    d->currentViewType = MusicPlayerPrivate::vtGenreView;
    d->listbox->setCurrentIndex(d->mediaList.index(0), QSmoothList::PositionAtCenter);
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(doMediaListPlayNow(QModelIndex)));
}

void MusicPlayer::viewAlbums()
{
    d->albumView->setVisible(false);
    d->container->setVisible(false);
    d->listbox->setVisible(false);
    d->sidebar->setVisible(false);
    d->albumView->setVisible(true);
    d->ribbonSelector->setVisible(true);
    d->titleLabel->setVisible(false);
    d->backButton->setVisible(false);
    d->listbox->setModel(&d->mediaList);
    d->currentViewType = MusicPlayerPrivate::vtAlbumCoverView;
    // display the albums list
    d->mediaList.setDisplayRole(QMediaPlaylist::Album);
    d->mediaList.setSorting(QList<int>() << QMediaPlaylist::Artist << QMediaPlaylist::Album, QMediaList::Descending);
    if(d->mediaList.isFiltered())
        d->mediaList.clearFilter();
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(doMediaListPlayNow(QModelIndex)));
}

void MusicPlayer::viewAlbums(const QString& artist)
{
    d->albumView->setVisible(false);
    d->container->setVisible(true);
    d->listbox->setVisible(true);
    d->sidebar->setVisible(true);
    d->ribbonSelector->setVisible(false);
    d->titleLabel->setVisible(true);
    d->backButton->setVisible(true);
    // display the albums list for the given artist
    d->mediaList.setDisplayRole(QMediaPlaylist::Album);
    d->mediaList.setFilter(QMediaList::Artist, artist);
    d->mediaList.setSorting(QList<int>(), QMediaList::Descending);
    d->listbox->setModel(&d->mediaList);
    d->currentViewType = MusicPlayerPrivate::vtAlbumView;
    d->listbox->setCurrentIndex(d->mediaList.index(0), QSmoothList::PositionAtCenter);
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(doMediaListPlayNow(QModelIndex)));
}

void MusicPlayer::viewArtists()
{
    d->albumView->setVisible(false);
    d->container->setVisible(true);
    d->listbox->setVisible(true);
    d->sidebar->setVisible(true);
    d->ribbonSelector->setVisible(true);
    d->titleLabel->setVisible(false);
    d->backButton->setVisible(false);
    // display the artists list
    d->mediaList.setDisplayRole(QMediaPlaylist::Artist);
    d->mediaList.setSorting(QList<int>(), QMediaList::Descending);
    d->mediaList.clearFilter();
    d->listbox->setModel(&d->mediaList);
    d->currentViewType = MusicPlayerPrivate::vtArtistView;
    d->listbox->setCurrentIndex(d->mediaList.index(0), QSmoothList::PositionAtCenter);
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(artistViewClicked(QModelIndex)));
    connect(d->listbox, SIGNAL(pressed(QModelIndex)), this, SLOT(artistViewPressed(QModelIndex)));
}

void MusicPlayer::viewPlaylists()
{
    d->albumView->setVisible(false);
    d->container->setVisible(true);
    d->listbox->setVisible(true);
    d->sidebar->setVisible(false);
    d->ribbonSelector->setVisible(false);
    d->titleLabel->setVisible(true);
    d->titleLabel->setText(tr("Playlists"));
    d->backButton->setVisible(false);
    // display the playlists list
    //TODO
    QContentFilter playlistfilter = QContentFilter( QContent::Document ) &
            ( QContentFilter( QContentFilter::MimeType, "audio/mpegurl" )
            | QContentFilter::mimeType( QLatin1String( "audio/x-mpegurl" ) )
            | QContentFilter::mimeType( QLatin1String( "audio/x-scpls" ) ));

    d->playlistContentSet=QContentSet(playlistfilter);
    d->listbox->setModel(&d->playlistContentSetModel);
    d->currentViewType = MusicPlayerPrivate::vtPlaylistView;
    d->listbox->setCurrentIndex(d->playlistContentSetModel.index(0));
    disconnect(d->listbox, SIGNAL(clicked(QModelIndex)), 0, 0);
    disconnect(d->listbox, SIGNAL(pressed(QModelIndex)), 0, 0);
    connect(d->listbox, SIGNAL(clicked(QModelIndex)), this, SLOT(playlistViewClicked(QModelIndex)));
}

void MusicPlayer::ribbonClicked(int index)
{
    const QChar &charindex=d->ribbonSelector->labels().at(index).at(0);
    if(index != d->lastIndex) {
        int i=0;
        if(d->albumView->isVisible()) {
            while(i < d->mediaList.rowCount() && d->mediaList.data(d->mediaList.index(i), QMediaPlaylist::Artist).toString().left(1) < charindex)
                i++;
            d->albumView->setIndex(i);
        }
        else {
            while(i < d->mediaList.rowCount() && d->mediaList.data(d->mediaList.index(i), d->mediaList.displayRole()).toString().left(1) < charindex)
                i++;
            d->listbox->setCurrentIndex(d->mediaList.index(i), QSmoothList::PositionAtCenter);
            d->listbox->scrollTo(d->mediaList.index(i), QSmoothList::PositionAtCenter);
        }
    }
}

void MusicPlayer::delayedInitialisations()
{
    QStringList musicMimeTypes = generateMusicMimeTypeList();

    if( !musicMimeTypes.isEmpty() ) {

        // Construct music menu
        QContentFilter musicfilter;

        foreach( const QString &mimeType, musicMimeTypes )
            musicfilter |= QContentFilter::mimeType( mimeType );

        musicfilter &= QContentFilter( QContent::Document );

        d->allMediaList = QMediaPlaylist(musicfilter);
        d->mediaList.setModel(d->allMediaList);
    }

    d->albumView->setModel(&d->mediaList, QMediaPlaylist::AlbumCover);

/*    if(QFile::exists(Qtopia::applicationFileName("musicplayer", "saved.pls"))) {
        d->currentPlaylist = QMediaPlaylist(Qtopia::applicationFileName("musicplayer", "saved.pls"));
    }*/
}

void MusicPlayer::doMediaListPlayNow(const QModelIndex& index)
{
    viewNowPlaying();
    d->currentPlaylist.playNow(d->mediaList.playlist(index));
}

void MusicPlayer::doMediaListQueueNow(const QModelIndex& index)
{
    viewNowPlaying();
    d->currentPlaylist.enqueue(d->mediaList.playlist(index));
}

void MusicPlayer::doPlayingChanged(const QModelIndex& index)
{
    if(d->mediaControl != NULL) {
        d->mediaControl->stop();
        delete d->mediaControl;
        d->mediaControl=NULL;
    }
    if(d->mediaContent != NULL) {
        disconnect(d->mediaContent, 0, 0, 0);
        delete d->mediaContent;
    }
    d->mediaContent = new QMediaContent(qvariant_cast<QUrl>(d->currentPlaylist.data(index, QMediaPlaylist::Url)));
    connect(d->mediaContent, SIGNAL(controlAvailable(QString)), this, SLOT(mediaControlAvailable(QString)));
    d->listbox->setCurrentIndex(index, QSmoothList::PositionAtCenter);
}

void MusicPlayer::mediaControlAvailable(QString const& id)
{
    if (id == QMediaControl::name()) {
        if(d->mediaControl != NULL) {
            d->mediaControl->stop();
            delete d->mediaControl;
            d->mediaControl=NULL;
        }
        d->mediaControl = new QMediaControl(d->mediaContent);
        d->mediaControl->start();
        connect(d->mediaControl, SIGNAL(playerStateChanged(QtopiaMedia::State)), this, SLOT(playerStateChanged(QtopiaMedia::State)));
    }
}

void MusicPlayer::smoothListCurrentChanged(const QModelIndex &index, const QModelIndex &)
{
    if(!index.isValid())
        return;
    if(d->sidebar->isVisible()) {
        QVariant v=d->mediaList.data(index, QMediaPlaylist::AlbumCover);
        if(v.canConvert<QImage>())
            d->sidebar->setThumbNail(v.value<QImage>());
    }
    if(d->titleLabel->isVisible()) {
        if(d->currentViewType == MusicPlayerPrivate::vtAlbumView) {
            d->titleLabel->setText(d->mediaList.data(index, QMediaPlaylist::Artist).toString());
        } else if(d->currentViewType == MusicPlayerPrivate::vtNowPlayingView) {
            if(!d->currentPlaylist.data(index, QMediaPlaylist::Artist).toString().isEmpty() && !d->currentPlaylist.data(index, QMediaPlaylist::Album).toString().isEmpty())
                d->titleLabel->setText(d->currentPlaylist.data(index, QMediaPlaylist::Artist).toString() + " - " + d->currentPlaylist.data(index, QMediaPlaylist::Album).toString());
            else if(d->currentPlaylist.data(index, QMediaPlaylist::Artist).toString().isEmpty())
                d->titleLabel->setText(d->currentPlaylist.data(index, QMediaPlaylist::Album).toString());
            else if(d->currentPlaylist.data(index, QMediaPlaylist::Album).toString().isEmpty())
                d->titleLabel->setText(d->currentPlaylist.data(index, QMediaPlaylist::Artist).toString());
        }
    }
}

void MusicPlayer::modelReset()
{
    smoothListCurrentChanged(d->listbox->currentIndex(), QModelIndex());
}

void MusicPlayer::artistViewClicked(const QModelIndex& index)
{
    QRect itemRect = d->listbox->visualRect(index);
    QRect arrowRect(itemRect.x()+itemRect.width()-itemRect.height(), itemRect.y(), itemRect.height(), itemRect.height());
    if(index == d->pressedIndex && (QDateTime::currentDateTime() > d->pressedTime.addSecs(1) || arrowRect.contains(d->listbox->mapFromGlobal(QCursor::pos()))))
        viewAlbums(d->mediaList.data(index, Qt::DisplayRole).toString());
    else
        d->listbox->setCurrentIndex(index, QSmoothList::PositionAtCenter);
}

void MusicPlayer::artistViewPressed(const QModelIndex& index)
{
    d->pressedIndex = index;
    d->pressedTime = QDateTime::currentDateTime();
}

void MusicPlayer::doBack()
{
    if(d->currentViewType == MusicPlayerPrivate::vtAlbumView) {
        MusicPlayer::viewArtists();
    } else if(d->currentViewType == MusicPlayerPrivate::vtNowPlayingView) {
        switch(d->previousViewType) {
            case MusicPlayerPrivate::vtGenreView:
                viewGenre();
                break;
            case MusicPlayerPrivate::vtAlbumCoverView:
                viewAlbums();
                break;
            case MusicPlayerPrivate::vtArtistView:
            case MusicPlayerPrivate::vtAlbumView:
                viewArtists();
                break;
            case MusicPlayerPrivate::vtPlaylistView:
                viewPlaylists();
                break;
            default:
                qWarning() << __PRETTY_FUNCTION__ << "No view to go back to.";
                break;
        }
    } else
        qWarning() << "In function" << __PRETTY_FUNCTION__ << ", functionality not implemented yet.";
}

void MusicPlayer::playlistViewClicked(const QModelIndex& index)
{
    d->currentPlaylist=QMediaPlaylist(d->playlistContentSetModel.content(index).fileName());
    d->currentPlaylist.playFirst();
    viewNowPlaying();
}

void MusicPlayer::nowPlayingViewClicked(const QModelIndex& index)
{
    QRect itemRect = d->listbox->visualRect(index);
    QRect arrowRect(itemRect.x()+itemRect.width()-itemRect.height(), itemRect.y(), itemRect.height(), itemRect.height());
    if(arrowRect.contains(d->listbox->mapFromGlobal(QCursor::pos()))) {
        if(index==d->currentPlaylist.playing() && d->mediaControl != NULL) {
            if(d->mediaControl->playerState() == QtopiaMedia::Playing)
                d->mediaControl->pause();
            else
                d->mediaControl->start();
            d->listbox->update(index);
        } else {
            d->currentPlaylist.setPlaying(index);
        }
    } else
        d->listbox->setCurrentIndex(index, QSmoothList::PositionAtCenter);
}

void MusicPlayer::playerStateChanged(QtopiaMedia::State state)
{
    if(state == QtopiaMedia::Stopped && d->currentPlaylist.nextIndex().isValid())
    {
        d->currentPlaylist.playNext();
        d->listbox->setCurrentIndex(d->currentPlaylist.playing());
    }
    else
        d->currentPlaylist.emitDataChanged(d->currentPlaylist.playing(), d->currentPlaylist.playing());
}

void MusicPlayer::playButtonPushed()
{
    if(d->currentViewType == MusicPlayerPrivate::vtArtistView || d->currentViewType == MusicPlayerPrivate::vtAlbumView) {
        doMediaListPlayNow(d->listbox->currentIndex());
    }
}

void MusicPlayer::addToQueueButtonPushed()
{
    if(d->currentViewType == MusicPlayerPrivate::vtArtistView || d->currentViewType == MusicPlayerPrivate::vtAlbumView) {
        doMediaListQueueNow(d->listbox->currentIndex());
    }
}

#include "musicplayer.moc"
