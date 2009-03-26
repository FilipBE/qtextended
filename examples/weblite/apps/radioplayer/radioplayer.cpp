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

#include <QtopiaApplication>
#include <QtopiaServiceRequest>
#include <QXmlStreamReader>
#include "webliteclient.h"
#include <private/homewidgets_p.h>
#include <private/qsmoothlistwidget_p.h>
#include <QStackedWidget>
#include <QInputDialog>
#include <QTimer>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QtopiaService>
#include <QScreen>
#include <QtopiaIpcAdaptor>
#include <QCopChannel>
#include <private/qtopiainputdialog_p.h>
struct RadioStationInfo
{
    int id;
    QString title, genres, track;
    int bitrate, listenerCount;
};

bool operator==(const RadioStationInfo & a, const RadioStationInfo & b)
{
    return a.id == b.id;
}

class RadioStationDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    public:
        RadioStationDelegate(QObject* o = NULL) : QAbstractItemDelegate(o)
        {
        }
        virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};
class RadioGenreDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    public:
        RadioGenreDelegate(QObject* o = NULL) : QAbstractItemDelegate(o)
        {
        }
        void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & idx) const
        {
            QLinearGradient lg (0,0,0,option.rect.height());
            lg.setColorAt(0,0xf7f8f8);
            lg.setColorAt(1,0xafb1b4);
            painter->setPen(QPen(QColor(0x4183a5)));
            QFont f = painter->font();
            f.setWeight(QFont::Bold);
            painter->setFont(f);
            painter->fillRect(option.rect,QBrush(lg));
            painter->drawText(QRect(3,0,option.rect.width()-3,option.rect.height()),Qt::AlignLeft|Qt::AlignVCenter,idx.data(0).toString());
        }
        QSize sizeHint ( const QStyleOptionViewItem & , const QModelIndex & ) const { return QSize (QScreen::instance()->width()*0.9,QScreen::instance()->height()*0.1); }
};

class RadioPlayer : public QWidget
{
    Q_OBJECT

    bool first;
    WebLiteClient* client;
    QStackedWidget* stackWdg;
    QLabel* titleLbl;
    QLabel* emptyLbl;
    CheckableHomeActionButton* playButton;
    QSmoothListWidget* stationList;
    QSmoothListWidget* genreList;
    QLabel* loadingLbl;
    QString currentTrackName;
    int rotationCount;
    QString trackName;
    QCopChannel* qcop;
    QList<RadioStationInfo> recentlyPlayed;
    HomeActionButton *recentButton;
    HomeActionButton *popularButton;
    HomeActionButton *randomButton;
    HomeActionButton *searchButton;
    HomeActionButton *genresButton;

    static QString recentDbFilename()
    {
        return QDir::homePath() + "radio_recent_db.xml";
    }

    public:
        bool event (QEvent*);
        RadioPlayer(QWidget* w= NULL,Qt::WindowFlags wf=(Qt::WindowFlags)0) : QWidget(w,wf),first(true)
        {
            QColor greyColor(60,50,45);
            client = new WebLiteClient (this);
            connect (client, SIGNAL(loaded()), this, SLOT(populateList()));
            QGridLayout* mainLayout = new QGridLayout();

            mainLayout->setMargin(0);
            mainLayout->setSpacing(0);

            stackWdg = new QStackedWidget();
            titleLbl = new QLabel(tr("SHOUTcast Radio"));
            QFont fnt = titleLbl->font();
            fnt.setWeight(QFont::Bold);
            titleLbl->setFont(fnt);
            titleLbl->setAlignment(Qt::AlignHCenter);
            HomeActionButton *closeButton = new HomeActionButton(QObject::tr("Close   "),QtopiaHome::Red);
            QObject::connect(closeButton,SIGNAL(clicked()),this,SLOT(hide()));
            playButton = new CheckableHomeActionButton(QObject::tr("Play"),QObject::tr("Stop"),QtopiaHome::standardColor(QtopiaHome::Green));
            QObject::connect(playButton,SIGNAL(clicked(bool)),this,SLOT(playPause(bool)));
            playButton->setEnabled(false);
            recentButton = new HomeActionButton(QObject::tr("Recent"),greyColor,Qt::white);
            QObject::connect(recentButton,SIGNAL(clicked()),this,SLOT(gotoRecent()));
            popularButton = new HomeActionButton(QObject::tr("Popular"),greyColor,Qt::white);
            QObject::connect(popularButton,SIGNAL(clicked()),this,SLOT(gotoPopular()));
            randomButton = new HomeActionButton(QObject::tr("Random"),greyColor,Qt::white);
            QObject::connect(randomButton,SIGNAL(clicked()),this,SLOT(gotoRandom()));
            searchButton = new HomeActionButton(QObject::tr("Search"),greyColor,Qt::white);
            QObject::connect(searchButton,SIGNAL(clicked()),this,SLOT(gotoSearch()));
            genresButton = new HomeActionButton(QObject::tr("Genres"),greyColor,Qt::white);
            QObject::connect(genresButton,SIGNAL(clicked()),this,SLOT(gotoGenres()));

            genreList = new QSmoothListWidget();
            genreList->setItemDelegate(new RadioGenreDelegate(this));
            stationList = new QSmoothListWidget ();
            stationList->setItemDelegate(new RadioStationDelegate(this));
            stationList->setSelectionMode(QSmoothList::SingleSelection);
            connect (genreList, SIGNAL(itemActivated(QSmoothListWidgetItem*)), this, SLOT(setGenre(QSmoothListWidgetItem*)));

            connect (stationList, SIGNAL(clicked(QModelIndex)), this, SLOT(setStation(QModelIndex)));
            loadingLbl = new QLabel(tr("Loading..."));
            loadingLbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            emptyLbl = new QLabel(tr("No Items to Show"));
            emptyLbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            stackWdg->addWidget(genreList);
            stackWdg->addWidget(stationList);
            stackWdg->addWidget(loadingLbl);
            stackWdg->addWidget(emptyLbl);
            stackWdg->setCurrentWidget(loadingLbl);
#ifdef QTOPIA_HOMEUI_WIDE
            delete closeButton;
            mainLayout->addWidget(titleLbl,0,0,1,6);
            mainLayout->addWidget(playButton,0,6,1,1);
            mainLayout->addWidget(recentButton,8,1,1,1);
            mainLayout->addWidget(popularButton,8,2,1,1);
            mainLayout->addWidget(genresButton,8,3,1,1);
            mainLayout->addWidget(randomButton,8,4,1,1);
            mainLayout->addWidget(searchButton,8,5,1,1);
            mainLayout->addWidget(stackWdg,1,0,7,7);
#else
            mainLayout->addWidget(closeButton,0,0,1,1);
            mainLayout->addWidget(titleLbl,0,1,1,4);
            mainLayout->addWidget(playButton,0,5,1,1);
            mainLayout->addWidget(recentButton,1,0,1,1);
            mainLayout->addWidget(popularButton,2,0,1,1);
            mainLayout->addWidget(genresButton,3,0,1,1);
            mainLayout->addWidget(randomButton,4,0,1,1);
            mainLayout->addWidget(searchButton,5,0,1,1);
            mainLayout->addWidget(stackWdg,1,1,8,5);
#endif
            setLayout(mainLayout);
            setWindowState(Qt::WindowMaximized);
            QSettings sett("Trolltech","radioplayer");
            QString str = sett.value("radio/quick_genres","Alternative Classical Comedy Country Dance/House Funk Jazz Metal Mixed Oldies Pop Rap RnB Rock Talk Techno 60s 70s 80s World").toString();
            QStringList lbls = str.split (" ");
            lbls.sort();
            genreList->addItems(lbls);
            QTimer::singleShot(0,this,SLOT(gotoRecent()));

            QTimer* titleRotateTimer = new QTimer(this);
            connect(titleRotateTimer, SIGNAL(timeout()), this, SLOT(updateTitle()));
            titleRotateTimer->start (4000);
            updateTitle();

            qcop = new QCopChannel("radiosvc",this);
            connect (qcop, SIGNAL(received(QString,QByteArray)), this, SLOT(qcopReceive(QString,QByteArray)));

//            WebLiteClient* genreClient = new WebLiteClient(this);
//            connect(genreClient, SIGNAL(loaded()),this,SLOT(genresLoaded()));
//            genreClient->setUrl(QUrl("http://www.shoutcast.com/sbin/newxml.phtml"));
//            genreClient->load();
            playPause(false);
        }

        void paintEvent(QPaintEvent*)
        {
            QPainter p (this);
            QRect rct (titleLbl->mapToParent(QPoint(0,0)).x(),titleLbl->height(),width(),height());
            p.fillRect(rct,QBrush(QColor(0x11,0,0,0x99)));
        }

        virtual ~RadioPlayer();
        enum Role
        {
            TitleRole = 1,
            GenresRole = 2,
            TrackRole = 3,
            IdRole = 4,
            BitrateRole = 5,
            ListenerCountRole = 6
        };

        enum Category
        {
            RecentlyPlayed,
            Random,
            ByGenre,
            Popular,
            TextSearch
        };

        void setSelection(HomeActionButton* b)
        {
            QColor greyColor(60,50,45);
            recentButton->setColors(greyColor,Qt::white);
            popularButton->setColors(greyColor,Qt::white);
            randomButton->setColors(greyColor,Qt::white);
            searchButton->setColors(greyColor,Qt::white);
            genresButton->setColors(greyColor,Qt::white);
            b->setColors(QtopiaHome::standardColor(QtopiaHome::Green),Qt::white);
        }

    public slots:
        void load (Category, const QString & param = QString());
        void populateList();
        void gotoSearch();
        void gotoRecent();
        void gotoPopular();
        void gotoGenres();
        void genresLoaded();
        void gotoRandom();
        void updateTitle ();
        void setGenre(QSmoothListWidgetItem* item);
        void setStation(const QModelIndex &);
        void setCurrentTrackName(const QString &);
        void playPause(bool);
        void onError(int, const QString &);
        void onTitleChanged (const QString &);
        void qcopReceive(const QString &, const QByteArray &);

        void saveRecentList ();
};

QSize RadioStationDelegate::sizeHint ( const QStyleOptionViewItem & , const QModelIndex & ) const
{
    return QSize(QScreen::instance()->width()*0.9,QScreen::instance()->height()*0.1);
}

void RadioStationDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLinearGradient lg (0,0,0,option.rect.height());
    if (option.state & QStyle::State_Selected)
    {
        lg.setColorAt(0,0x29769d);
        lg.setColorAt(1,0x4183a5);
        painter->setPen(QPen(Qt::white));
    }
    else
    {
        lg.setColorAt(0,0xf7f8f8);
        lg.setColorAt(1,0xafb1b4);
        painter->setPen(QPen(QColor(0x4183a5)));
    }
    painter->fillRect(QRect(0,0,option.rect.width(),option.rect.height()),QBrush(lg));
    QFont fnt = painter->font();
    QFont fnt2 = fnt;
    fnt.setWeight(QFont::Bold);
    QFontMetrics fm(fnt);
    QString title = fm.elidedText(index.data(RadioPlayer::TitleRole).toString(),Qt::ElideRight,option.rect.width());
    painter->setFont(fnt);
    fnt2.setPointSize(fnt.pointSize()-2);
    QFontMetrics fm2(fnt2);
    QString desc = fm2.elidedText(QString("[%1] %2") .arg(index.data(RadioPlayer::GenresRole).toString()).arg(index.data(RadioPlayer::TrackRole).toString()),Qt::ElideRight,option.rect.width());
    QRect rct = QRect(0,0,option.rect.width(),option.rect.height());
    rct.adjust(2,0,-2,-0);
//    rctTitle.adjust(3,0,-3,rctTitle.height()>>1);
//    rctDesc.adjust(3,rctTitle.height()>>1,-3,0);
    painter->drawText(rct,Qt::AlignLeft|Qt::AlignTop,title);
    painter->setFont(fnt2);
    painter->drawText(rct,Qt::AlignLeft|Qt::AlignBottom,desc);
    painter->setFont(fnt);
}

void RadioPlayer::setCurrentTrackName(const QString & s)
{
    if (s.trimmed().startsWith("';"))
        trackName = "";
    else
        trackName = s;
    updateTitle ();
}

void RadioPlayer::playPause(bool play)
{
    QtopiaServiceRequest req;
    req.setService ("WebRadio");
    req.setMessage(play?"play()":"pause()");
    req.send ();
}

void RadioPlayer::load (RadioPlayer::Category cat, const QString & p)
{
    client->abort ();
    stackWdg->setCurrentWidget(loadingLbl);
    if (cat == RecentlyPlayed)
    {
        QTimer::singleShot(0,this,SLOT(populateList()));
    }
    else
    {
        QString param;
        switch (cat)
        {
            case Random:
                param = "random=50";
                break;
            case ByGenre:
                param = QString("genre=")+p;
                break;
            case Popular:
                param = "genre=Top500";
                break;
            case TextSearch:
                param = "search="+p;
                break;
            default:
                break;
        }
        client->setUrl(QUrl(QString("http://www.shoutcast.com/sbin/newxml.phtml?")+param));
        client->load ();
    }
}

void RadioPlayer::updateTitle ()
{
    QSmoothListWidgetItem* curItem = stationList->currentItem ();
    QString station, track;
    QStringList sl;
    sl << tr("SHOUTcast Internet radio");
    if (curItem)
    {
        sl << curItem->data(TitleRole).toString();
        if (playButton->isChecked() && currentTrackName.length())
        {
            sl << currentTrackName;
        }
    }
    QFontMetrics fm (titleLbl->font());
    rotationCount = (rotationCount+1)%sl.count();
    titleLbl->setText(fm.elidedText(sl[rotationCount],Qt::ElideRight,titleLbl->width()));
}

void RadioPlayer::populateList()
{
    QString filename;
    stationList->clear ();
    bool recentList = sender() != client;
    if (recentList)
    {
        recentlyPlayed.clear();
        filename = recentDbFilename();
    }
    else
    {
        client->abort ();
        filename = client->filename();
    }

    QFile f (filename);
    f.open(QIODevice::ReadOnly);
    bool empty = true;
    QXmlStreamReader rdr (&f);
    while (!rdr.atEnd())
    {
        rdr.readNext();
        if (rdr.isStartElement() && rdr.name().toString() == "station")
        {
            QXmlStreamAttributes attr = rdr.attributes();
            if (attr.value("mt").toString() == "audio/mpeg")
            {
                RadioStationInfo inf;
                inf.title = attr.value("name").toString();
                inf.id = attr.value("id").toString().toInt();
                inf.bitrate = attr.value("br").toString().toInt();
                inf.listenerCount = attr.value("lc").toString().toInt();
                inf.track = attr.value("ct").toString();
                inf.genres = attr.value("genre").toString();
                if (recentList)
                {
                    recentlyPlayed.push_back(inf);
                }
                empty = false;
                QSmoothListWidgetItem* item = new QSmoothListWidgetItem(stationList);
                item->setData((int)TitleRole, inf.title);
                item->setData((int)IdRole, QVariant(inf.id));
                item->setData((int)TrackRole, inf.track);
                item->setData((int)BitrateRole, QVariant(inf.bitrate));
                item->setData((int)ListenerCountRole, QVariant(inf.listenerCount));
                item->setData((int)GenresRole, inf.genres);
            }
        }
    }
    if (empty) {
        if (first)
            gotoPopular();
        else
            stackWdg->setCurrentWidget(emptyLbl);
    }
    else
        stackWdg->setCurrentWidget(stationList);
    first = false;
    stationList->setCurrentRow(-1);
}

void RadioPlayer::gotoSearch()
{
    QString txt = QtopiaInputDialog::getText(this, tr("Search Stations"), tr("Type Search Query:"));
    if (txt != "")
    {
        load (TextSearch, txt);
    }
    setSelection(searchButton);
}

void RadioPlayer::gotoRecent()
{
    setSelection(recentButton);
    load (RecentlyPlayed);
}
void RadioPlayer::gotoPopular()
{
    setSelection(popularButton);
    load (Popular);
}
void RadioPlayer::gotoGenres()
{
    setSelection(genresButton);
    stackWdg->setCurrentWidget(genreList);
}
void RadioPlayer::genresLoaded()
{
    WebLiteClient* wlc = (WebLiteClient*)sender();
    wlc->deleteLater();
    /*
    QFile f (wlc->filename());
    f.open (QIODevice::ReadOnly);
    QXmlStreamReader r (&f);
    QStringList ls;
    while (!r.atEnd())
    {
        r.readNext();
        if (r.isStartElement() && r.name().toString() == "genre")
        {
            QString name = r.attributes().value("name").toString();
            ls << name;
        }
    }
    if (ls.count() > 0)
    {
        genreList->clear();
        genreList->addItems(ls);
    }
    */
}
void RadioPlayer::gotoRandom()
{
    setSelection(randomButton);
    load (Random);
}
void RadioPlayer::setGenre(QSmoothListWidgetItem* item)
{
    if (item)
        load (ByGenre, item->text());
}
void RadioPlayer::setStation(const QModelIndex & mi)
{
    if (mi.row() >= 0)
    {
        RadioStationInfo inf;
        inf.id = mi.data(IdRole).toInt();
        int idx = recentlyPlayed.indexOf(inf);
        inf.title = mi.data(TitleRole).toString();
        inf.track = mi.data(TrackRole).toString();
        inf.genres = mi.data(GenresRole).toString();

        if (idx >= 0)
        {
            recentlyPlayed.removeAt (idx);
        }
        recentlyPlayed.prepend(inf);
        saveRecentList ();

        QtopiaServiceRequest req;
        req.setService ("WebRadio");
        req.setMessage("setPlaylistUrl(QUrl)");
        req << QUrl(QString("http://www.shoutcast.com/sbin/shoutcast-playlist.pls?rn=%1&file=filename.pls").arg(inf.id));
        req.send ();
        currentTrackName=tr("Loading Station...");
        playPause(true);
        playButton->setChecked(true);
        playButton->setEnabled(true);
    }
}

void RadioPlayer::qcopReceive (const QString & , const QByteArray & data)
{
    int status;
    QString m;
    QDataStream ds (data);
    ds >> status >> m;
    if (status == 105)
        onTitleChanged(m);
    else if (status >= 400)
        onError(status,m);
}
void RadioPlayer::onError(int status, const QString & err)
{
    playPause (false);
    playButton->setChecked(false);
    QMessageBox::warning(this, tr("Inavlid Radio Station"),tr("Please Select a different station. Error: (%1) %2").arg(status).arg(err));
    currentTrackName = "";
}
void RadioPlayer::onTitleChanged (const QString & str)
{
    currentTrackName = str.startsWith("'")?"":str;
    updateTitle ();
}

bool RadioPlayer::event(QEvent* e)
{
    if (e->type() == QEvent::WindowDeactivate || e->type() == QEvent::Hide)
    {
        playPause (false);
        playButton->setChecked(false);
    }
//                hide ();
    return QWidget::event(e);
//            fullScreenOff();
}

void RadioPlayer::saveRecentList ()
{
    QFile f (recentDbFilename());
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter w (&f);
    w.writeStartDocument();
    w.writeStartElement("stationlist");
    int i = 0;
    for (QList<RadioStationInfo>::iterator it = recentlyPlayed.begin(); it != recentlyPlayed.end() && i < 40; ++it,++i)
    {
        w.writeStartElement("station");
        w.writeAttribute("name",it->title);
        w.writeAttribute("mt","audio/mpeg");
        w.writeAttribute("id",QString("%1").arg(it->id));
        w.writeAttribute("br",QString("%1").arg(it->bitrate));
        w.writeAttribute("genre",it->genres);
        w.writeAttribute("ct",it->track);
        w.writeAttribute("lc",QString("%1").arg(it->listenerCount));
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndDocument();
    f.close ();
}
RadioPlayer::~RadioPlayer()
{
    playPause(false);
}

QSXE_APP_KEY
int main (int argc, char** argv)
{
    QtopiaApplication app(argc,argv);
    RadioPlayer* rp = new RadioPlayer();
    app.setMainWidget(rp);
    rp->show();
    app.exec();
}

#include "radioplayer.moc"
