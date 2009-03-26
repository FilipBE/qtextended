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
#include <webfeed.h>
#include <mediafeed.h>
#include "webliteclient.h"
#include "webliteimg.h"
#include <private/homewidgets_p.h>
#include <QListView>
#include <QStackedWidget>
#include <QInputDialog>
#include <QTimer>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextDocument>
#include <private/qsmoothlistwidget_p.h>
#include <QPainter>
#include <QMediaControl>
#include <QMediaContent>
#include <QMediaVideoControl>
#include <QMediaControlNotifier>
#include <QtopiaService>
#include <QtopiaIpcAdaptor>
#include <QCopChannel>
#include <private/qtopiainputdialog_p.h>

static QString dur2str(int d)
{
    if (d == 0)
        return "0:00";
    int colons = 0;
    QString durText;
    for (int dur = d; dur != 0; dur /= 60, colons++)
    {
        durText = QString("%1").arg((int)dur%60,2,10,QChar('0'))+(durText.length()?":":"")+durText;
    }
    if (colons == 1)
        durText = "0:" + durText;
    return durText;
}

class ProgSlider: public QSlider
{
    Q_OBJECT
    public:
        ProgSlider (QWidget* o=NULL);
        int progressValue () const;
        double progressPerc () const;
        void setProgressPerc(double);
        void setProgressValue(int);
        void paintEvent(QPaintEvent* pe);
    private:
        int progVal;
};
ProgSlider::ProgSlider (QWidget* w)
    :QSlider (Qt::Horizontal,w), progVal (-1)
{
}

void ProgSlider::paintEvent(QPaintEvent* )
{
    QPainter p (this);
    QRect rct (QPoint(0,0),size());
    rct.adjust(0,0,-1,-1);
    QLinearGradient lg (0,height(),width(),0);
    lg.setColorAt(0,QColor(0x2a2a2a));
    lg.setColorAt(1,QColor(0x585858));
    QLinearGradient lg2 (0,0,width(),0);
    lg2.setColorAt(0,QColor(0x686464));
    lg2.setColorAt(1,QColor(0xa59e9e));
    QLinearGradient lg3 (0,0,width(),0);
    lg3.setColorAt(0,QColor(0x2d618f));
    lg3.setColorAt(1,QColor(0x295579));
    QBrush brush (lg);
    QPen pen;
    pen.setBrush (lg);
    p.setPen (pen);
    QPainterPath pp;
    pp.addRoundRect (rct,32);
    QPainterPath pp2;
    if (progVal > 0)
    {
        QRect rct2 (rct);
        rct2.setWidth((int)(rct.width() * progressPerc()));
        pp2.addRoundRect (rct2,32);
        QPainterPath pp3;
        QRect rct3 (rct);
        rct3.setWidth((rct.width() * value())/maximum());
        pp3.addRoundRect (rct3,24);
        p.fillPath(pp2,QBrush(lg2));
        p.fillPath(pp3,QBrush(lg3));
    }
    p.drawPath(pp);
}

int ProgSlider::progressValue () const
{
    if (progVal < 0)
        return maximum();
    else
        return progVal;
}
double ProgSlider::progressPerc () const
{
    return (double)(progressValue() - minimum()) / (maximum() - minimum ());
}
void ProgSlider::setProgressPerc (double v)
{
    setProgressValue ((int)(minimum() + ((double)maximum() - minimum())*v));
}
void ProgSlider::setProgressValue (int v)
{
    progVal = v;
    update ();
}

class PodcastPlayerSM : public QObject
{
    Q_OBJECT
    public:
        enum State
        {
            Pausing,
            Paused,
            Error,
            WantToPlay = 0x100,
            Idle,
            Loading,
            TestingBufferInactive,
            TestingBufferActive,
            Buffering,
            PausingToBuffer,
            InitializingPlayer,
            StartingPlayer,
            CreatingVideoWidget,
            Active
        } curState;

        enum Event
        {
            Init,

            DownloadProgress,
            DownloadError,
            MediaProgress,
            MediaPaused,
            MediaValid,

            VideoValid,
            VideoInvalid,
            MediaInvalid,
            MediaError,
            VideoWidgetCreated,

            MediaPlaying,
            UserPlay,
            UserPause,
            UserClose,
            BufferOk,
            BufferWait
        } ;
    signals:
        void enterState (PodcastPlayerSM::State);
        void eventTrigger (Event);

    public slots:
        void transition (State s)
        {
            curState = s;
            emit enterState (s);
        }
        void sendEvent (Event e)
        {
            emit eventTrigger (e);
        }

        void handleEvent (Event e)
        {
            switch (e)
            {
                case Init:
                    transition (Loading);
                    break;
                case DownloadProgress:
                    if (curState == Loading || curState == Buffering)
                        transition (TestingBufferInactive);
                    break;
                case DownloadError:
                    if (curState == Buffering)
                        transition (Error);
                    break;
                case MediaProgress:
                    if (curState == Active)
                        transition (TestingBufferActive);
                    break;
                case MediaPaused:
                    if (curState == PausingToBuffer)
                        transition (Buffering);
                    else if (curState == Pausing || curState == Active)
                        transition (Paused);
                    else
                        transition (Error);
                    break;
                case MediaPlaying:
                    if (curState == StartingPlayer)
                        transition (Active);
                    break;
                case MediaError:
                    transition (Error);
                    break;
                case MediaValid:
                    if (curState == InitializingPlayer)
                        transition (StartingPlayer);
                    break;
                case MediaInvalid:
                    if (curState == InitializingPlayer)
                        transition (Error);
                    break;
                case VideoValid:
                    if (curState == StartingPlayer)
                        transition (CreatingVideoWidget);
                    break;
                case VideoInvalid:
                case VideoWidgetCreated:
                    if (curState == CreatingVideoWidget)
                        transition (Active);
                    break;
                case UserPlay:
                    if (!(curState & WantToPlay))
                    {
                        transition (TestingBufferInactive);
                    }
                    break;
                case UserPause:
                    if (curState & WantToPlay)
                        transition (Pausing);
                    break;
                case UserClose:
                    if (curState & WantToPlay)
                        transition (Idle);
                    break;
                case BufferOk:
                    if (curState == TestingBufferActive)
                        transition (Active);
                    else if (curState == TestingBufferInactive)
                        transition (InitializingPlayer);
                    break;
                case BufferWait:
                    if (curState == TestingBufferActive)
                        transition (PausingToBuffer);
                    else if (curState == TestingBufferInactive)
                        transition (Buffering);
                    break;
            }
        }
    public:
        PodcastPlayerSM(QObject* o) : QObject(o),curState(Idle)
        {
            connect (this,SIGNAL(eventTrigger(Event)), this, SLOT(handleEvent(Event)));
        }
};

class TitleLabel : public QLabel
{
    Q_OBJECT
    public:
        TitleLabel(const QString & s, QWidget* w = NULL) : QLabel(s,w)
        {
        }
    void paintEvent(QPaintEvent*)
    {
        QPainter p (this);
        p.setFont(font());
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(0x0e6693)));
        QLinearGradient lg (0,0,0,height());
        lg.setColorAt(0,Qt::white);
        lg.setColorAt(1,QColor(0xb0b2b5));
        QFontMetrics fm (p.font());
        p.fillRect(QRect(0,0,width(),height()),QBrush(lg));
        p.drawText(QRect(0,0,width(),height()),Qt::AlignCenter|Qt::TextWordWrap,fm.elidedText(text(),Qt::ElideRight,width()-4));
    }
};

class PodcastLoader : public QObject
{
    Q_OBJECT
    public:
        WebMediaFeedParser mediaParser;
        WebPodcastFeedParser podParser;
        WebFeedItem* rssItem;
        WebFeedChannel* podcast;
        QPointer<WebLiteImg> imgLoader;
        WebFeedLoader* feedLoader;
        QStandardItem* standardItem;
        bool pending;
        PodcastLoader (WebFeedItem* item, QStandardItem* sti, QObject* parent = NULL) : QObject(parent),rssItem(item), podcast(NULL),standardItem(sti),pending(false)
        {
            standardItem->setText(item->title);
            QList<WebFeedLink*> l = item->elementsByType<WebFeedLink>();
            for (QList<WebFeedLink*>::iterator it = l.begin(); it!= l.end();++it)
            {
                WebFeedLink* lnk = *it;
                if (lnk->rel == "enclosure")
                {
                    imgLoader = new WebLiteImg(this);
                    connect(imgLoader, SIGNAL(decoded()), this, SLOT(imgLoaded()));
                    imgLoader->setUrl(lnk->href);
                    imgLoader->load ();
                }
                else if (lnk->rel == "")
                {
                    feedLoader = new WebFeedLoader(this);
                    feedLoader->installExtension(&mediaParser);
                    feedLoader->installExtension(&podParser);
                    connect (feedLoader, SIGNAL(updated()), this, SLOT(podcastLoaded()));
                    feedLoader->setUrl (lnk->href);
                    feedLoader->load ();
                }
            }
        }
    signals:
        void showPending ();
        void updated ();
        void activated(WebFeedChannel*);

    public slots:
        void podcastLoaded ()
        {
            podcast = (WebFeedChannel*)feedLoader->root();
            if (podcast)
            {
                QList<WebFeedImage*> l = podcast->elementsByType<WebFeedImage>();
                if (!l.empty())
                {
                    WebFeedImage* img = *(l.begin());
                    imgLoader->setUrl(img->url);
                    imgLoader->load();
                }

                if (standardItem)
                {
                    standardItem->setText(podcast->title);
                }
                emit updated ();
                if (pending)
                    emit activated(podcast);
            }
        }
        void imgLoaded()
        {
            if (standardItem)
            {
                standardItem->setIcon(QIcon(imgLoader->pixmap()));
            }
            emit updated ();
        }

        void activate()
        {
            if (podcast)
                emit activated(podcast);
            else
            {
                pending = true;
                emit showPending();
            }
        }
};

class PodcastStandardItem : public QSmoothListWidgetItem
{
    public:
        const WebFeedItem *rssItem;
            PodcastStandardItem (const WebFeedItem * it, QSmoothListWidget* w) : QSmoothListWidgetItem(w),rssItem(it)
            {
                setData(Qt::SizeHintRole,QVariant(QSize(QScreen::instance()->width(),QScreen::instance()->height()/(rssItem->description.length()?5:10))));
            }
};

static QString unescape (const QString & str)
{
    QTextDocument doc;
    doc.setHtml (str);
    return doc.toPlainText();
}

class PodcastDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    QSmoothListWidget* wdg;
public:

    PodcastDelegate(QSmoothListWidget* wl): QAbstractItemDelegate(wl),wdg(wl) {}
    QSize sizeHint(const QStyleOptionViewItem & , const QModelIndex & mi) const
    {
        return mi.data(Qt::SizeHintRole).value<QSize>();
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        PodcastStandardItem* sti = (PodcastStandardItem*)wdg->item(index.row());
        painter->setRenderHint(QPainter::Antialiasing);
        QLinearGradient lg (0,0,0,opt.rect.height());
        QFont f = painter->font ();
        f.setWeight(QFont::Bold);
        if (opt.state & QStyle::State_Selected)
        {
            lg.setColorAt(0,0x29769d);
            lg.setColorAt(1,0x4183a5);
            painter->setPen(QPen(Qt::white));
        }
        else
        {
            lg.setColorAt(0,Qt::white);
            lg.setColorAt(1,QColor(0xb0b2b5));
            painter->setPen(QPen(QColor(0x0e6693)));
        }
        painter->setFont(f);
        QFontMetrics fm (f);
        int titleHeight = fm.height ();
        painter->fillRect(opt.rect,QBrush(lg));
        QString durText;
        QRect titleRect = opt.rect.adjusted(2,0,-2,0-opt.rect.height()+titleHeight);
        QList<WebFeedPodcastInfo*> linf = sti->rssItem->elementsByType<WebFeedPodcastInfo>();
        if (linf.size())
        {
            int d = linf[0]->durationSecs;
            if (d)
            {
                durText = dur2str(d);
            }
        }
        QFont f2 = f;
        f2.setPixelSize(10);
        f2.setWeight(QFont::Normal);
        QFontMetrics fm2 (f2);
        if (durText.length())
            titleRect.adjust(0,0,0 - 2 - fm2.width(durText),0);
        painter->drawText(titleRect,Qt::AlignTop|Qt::AlignLeft,fm.elidedText(sti->rssItem->title,Qt::ElideRight,opt.rect.width()-4));
        painter->setFont(f2);
        if (durText.length())
            painter->drawText(opt.rect.adjusted(titleRect.width()+2,0,0,0),Qt::AlignHCenter,durText);
        painter->drawText(opt.rect.adjusted(2,titleHeight+1,-2,0),Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap,fm2.elidedText(unescape(sti->rssItem->description),Qt::ElideRight,opt.rect.width()*2-12));
    }
};

class PodcastDirDelegate : public QAbstractItemDelegate
{
    public:
    QStandardItemModel* model;
    PodcastDirDelegate (QStandardItemModel* sim, QObject *parent = 0)
    : QAbstractItemDelegate(parent),model (sim)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem & , const QModelIndex & ) const
    {
        return QSize(QScreen::instance()->width()>>2,QScreen::instance()->height()>>2);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QStandardItem* sti = model->itemFromIndex(index);
        QPainterPath pp;
        pp.addRoundRect(opt.rect,32);
        QLinearGradient lg (0,0,opt.rect.width(),opt.rect.height());
        lg.setColorAt(0,QColor(0,0,0,70));
        lg.setColorAt(1,QColor(0,0,0,128));
        painter->fillPath (pp,QBrush(QColor(0xff,0xff,0xff,235)));
        QPen pen;
        pen.setBrush(QBrush(lg));
        painter->setPen(pen);
        painter->drawPath(pp);
        painter->setPen(QPen(QColor(0,0,0)));
        QFont f = painter->font ();
        f.setWeight(QFont::Bold);
        painter->setFont (f);
        QIcon icon = sti->icon ();
        QFontMetrics fm (f);
        if (icon.isNull())
        {
            // draw text on entire rect
            painter->drawText (opt.rect.adjusted(2,2,-2,-2),Qt::TextWordWrap|Qt::AlignVCenter|Qt::AlignHCenter, fm.elidedText(sti->text(), Qt::ElideRight, opt.rect.width()*2));
        }
        else
        {
            QString text = fm.elidedText(sti->text(),Qt::ElideRight, opt.rect.height()-4);
            int textWidth = fm.height()+4;
            QPixmap pxm (opt.rect.width(),opt.rect.height());
            pxm.fill(QColor(0xff,0xff,0xff,0));
            {
                QPainter p (&pxm);
                p.setRenderHint(QPainter::Antialiasing);
                p.setFont(f);
                QRect r (0-opt.rect.height(),0,opt.rect.height(),textWidth);
                p.rotate(-90);
                p.drawText(r,Qt::AlignCenter,text);
                p.rotate (90);
            }
            painter->drawPixmap(opt.rect.x(),opt.rect.y(),pxm);
            QRect r = opt.rect.adjusted(textWidth,0,0,0);
            painter->fillRect(r,QBrush(Qt::lightGray));
            painter->setPen(QPen(Qt::lightGray));
            painter->drawRect(r);
            icon.paint(painter,r);
        }
        painter->restore();
    }
};

class PodcastViewApp : public QWidget
{
    Q_OBJECT
    public:
        HomeActionButton *playerFullScreenButton;
        QStackedWidget* stackWdg;
        QWidget* mainMenuView;
        QWidget* podcastView;
        QWidget* podcastPlayer;
        QListView* directoryList;
        QStandardItemModel* directoryModel;
        PodcastDirDelegate* podcastDirDelegate;
        WebFeedLoader* dirLoader;
        QLabel* mainTitle;
        QWidget* loadingView;
        WebFeedChannel curPodcast;
        QLabel* podcastTitleLbl;
        QLabel* podcastIconLbl;
        QLabel* podcastDescLbl;
        QSmoothListWidget* podcastList;
        QPointer<WebLiteClient> mediaGetter;
        int curMediaDuration, curMediaFilesize,mediaPos;
        QTime startDownloadTime;
        PodcastPlayerSM* playerSM;
        QLabel* playerBackgroundLbl;
        QLabel* playerTitleLbl;
        QLabel* durationLbl;
        QPointer<QMediaContentContext> mediaCtx;
        QPointer<QMediaContent> mediaContent;
        QPointer<QMediaControl> mediaControl;
        QPointer<QMediaControlNotifier> mediaNotifier;
        QPointer<QMediaControlNotifier> videoNotifier;
        ProgSlider* progSlider;
        QPointer<QWidget> videoWidget;
        bool mute,fullScreen;
        CheckableHomeActionButton *playPauseButton;
        PodcastViewApp(QWidget* w = NULL) : QWidget(w),mute(false),fullScreen(false)
        {
            stackWdg =  new QStackedWidget();
            QHBoxLayout* hbl = new QHBoxLayout();
            hbl->setSpacing(0);
            hbl->setMargin(0);
            setLayout(hbl);
            hbl->addWidget(stackWdg);
            podcastView = new QWidget ();
            podcastPlayer = new QWidget ();
            QFont f;

            // main menu
            {
                mainMenuView = new QWidget();
                HomeActionButton *mainCloseButton = new HomeActionButton(QObject::tr("Close"),QtopiaHome::Red);
                connect(mainCloseButton, SIGNAL(clicked()), this, SLOT(close()));
                mainTitle = new TitleLabel(tr("Podcast Directory"));
                f = mainTitle->font();
                f.setWeight(QFont::Bold);
                mainTitle->setFont(f);
                mainTitle->installEventFilter(this);
                mainTitle->setAlignment(Qt::AlignCenter);
                QVBoxLayout* mainMenuVbl = new QVBoxLayout ();
                mainMenuView->setLayout(mainMenuVbl);
                QHBoxLayout* mainMenuTopLayout = new QHBoxLayout ();
                mainMenuTopLayout->addWidget(mainCloseButton,0);
                mainMenuTopLayout->addWidget(mainTitle,1);
                mainMenuTopLayout->setSpacing(0);
                mainMenuTopLayout->setMargin(0);
                dirLoader = new WebFeedLoader (this);
                QString u = QString("file://%1etc/podcasts.xml").arg(Qtopia::qtopiaDir());
                connect (dirLoader, SIGNAL(updated()), this, SLOT(dirLoaded()));
                dirLoader->setUrl (QUrl(u));
                dirLoader->load();

                directoryList = new QListView();
                connect (directoryList, SIGNAL(clicked(QModelIndex)),this, SLOT(podcastClicked(QModelIndex)));
                directoryList->setFrameStyle(QFrame::NoFrame);
                QPalette pal =directoryList->viewport()->palette();
                pal.setColor(QPalette::Background,Qt::black);
                directoryList->setAutoFillBackground(false);
                directoryList->viewport()->setAutoFillBackground(false);
                directoryList->setViewMode(QListView::IconMode);
                directoryList->setModel(new QStandardItemModel (this));
                directoryList->setItemDelegate(new PodcastDirDelegate ((QStandardItemModel*)directoryList->model(),this));
                mainMenuVbl->setMargin(0);
                mainMenuVbl->addLayout (mainMenuTopLayout,0);
                mainMenuVbl->addWidget (directoryList, 1);
                mainMenuVbl->setSpacing(0);
                mainMenuVbl->setMargin(0);
            }

            stackWdg->addWidget (mainMenuView);
            {
                HomeActionButton *podcastBackButton = new HomeActionButton(QObject::tr("Back"),QtopiaHome::Red);
                connect(podcastBackButton, SIGNAL(clicked()), this, SLOT(showDir()));
                podcastTitleLbl = new TitleLabel(tr("Podcast"));
                podcastTitleLbl->setFont(f);
                podcastIconLbl = new QLabel();
                podcastIconLbl->setFrameStyle(QFrame::Box);
                podcastDescLbl = new TitleLabel("");
//                podcastDescLbl->setMaximumSize(QScreen::instance()->width(),QScreen::instance()->height()>>3);
                QVBoxLayout* podcastVbl = new QVBoxLayout();
                podcastVbl->setMargin(0);
                podcastVbl->setSpacing(0);
                QHBoxLayout* podcastHbl1 = new QHBoxLayout();
                podcastHbl1->setMargin(0);
                podcastHbl1->setSpacing(0);
                QHBoxLayout* podcastHbl2 = new QHBoxLayout();
                podcastHbl2->setMargin(0);
                podcastHbl2->setSpacing(0);
                QVBoxLayout* podcastTopVbl = new QVBoxLayout();
                podcastTopVbl->setMargin(0);
                podcastTopVbl->setSpacing(0);
                podcastHbl2->addWidget(podcastBackButton,0);
                podcastHbl2->addWidget(podcastTitleLbl,1);
                podcastTopVbl->addLayout(podcastHbl2);
                podcastTopVbl->addWidget(podcastDescLbl);
                podcastHbl1->addLayout(podcastTopVbl,1);
                podcastHbl1->addWidget(podcastIconLbl,0);
                podcastVbl->addLayout(podcastHbl1,0);
                podcastList = new QSmoothListWidget();
                podcastList->setItemDelegate (new PodcastDelegate(podcastList));
                connect (podcastList, SIGNAL(clicked(QModelIndex)), this, SLOT(podcastItemSelected(QModelIndex)));
                podcastVbl->addSpacing(3);
                podcastVbl->addWidget(podcastList,1);
                podcastView->setLayout(podcastVbl);
            }

            stackWdg->addWidget (podcastView);
            // player
            {
                HomeActionButton *playerBackButton = new HomeActionButton(QObject::tr("Back"),QtopiaHome::Red);
                playerFullScreenButton = new HomeActionButton(QObject::tr("Full\nScreen"),QtopiaHome::Green);
                playPauseButton = new CheckableHomeActionButton(QObject::tr("Play"),QObject::tr("Pause"),QtopiaHome::Green);
                CheckableHomeActionButton *muteButton = new CheckableHomeActionButton(QObject::tr("Mute"),QObject::tr("Unmute"),QtopiaHome::Green);
                connect(playerBackButton, SIGNAL(clicked()), this, SLOT(showPodcast()));
                connect(playerFullScreenButton, SIGNAL(clicked()), this, SLOT(fullScreenOn()));
                connect(playPauseButton, SIGNAL(clicked(bool)), this, SLOT(playPause(bool)));
                connect(muteButton, SIGNAL(clicked(bool)), this, SLOT(setMute(bool)));
                playPauseButton->setChecked(true);
                playerFullScreenButton->setEnabled(false);
                playerTitleLbl = new TitleLabel(tr("Podcast Player"));
                playerTitleLbl->setFont(f);
                QVBoxLayout* playerMainVbl = new QVBoxLayout();
                playerMainVbl->setMargin(0);
                playerMainVbl->setSpacing(0);
                QHBoxLayout* playerTopHbl = new QHBoxLayout();
                playerTopHbl->setMargin(0);
                playerTopHbl->setSpacing(0);
                playerTopHbl->addWidget(playerBackButton,0);
                playerTopHbl->addWidget(playerTitleLbl,1);
                QHBoxLayout* playerBottomHbl = new QHBoxLayout();
                playerBottomHbl->setMargin(0);
                playerBottomHbl->setSpacing(3);
                playerBackgroundLbl = new QLabel();
                playerBackgroundLbl->setMargin(10);
                playerBackgroundLbl->setWordWrap(true);
                playerBackgroundLbl->setText(tr("Loading..."));
                playerBackgroundLbl->setAutoFillBackground(true);
                playerBackgroundLbl->setBackgroundRole(QPalette::Background);
                playerBackgroundLbl->setForegroundRole(QPalette::Foreground);
                durationLbl = new QLabel();
                QPalette p = playerBackgroundLbl->palette ();
                p.setColor(QPalette::Background,Qt::black);
                p.setColor(QPalette::Foreground,Qt::white);
                playerMainVbl->addLayout(playerTopHbl,0);
                QVBoxLayout* playerButtonVbl = new QVBoxLayout();
                playerButtonVbl->setSpacing(0);
                playerButtonVbl->setMargin(0);
                playerButtonVbl->addWidget(playPauseButton,0);
                playPauseButton->setMinimumHeight(QScreen::instance()->height()>>3);
                playerButtonVbl->addWidget(muteButton,0);
                muteButton->setMinimumHeight(QScreen::instance()->height()>>3);
                playerButtonVbl->addWidget(playerFullScreenButton,0);
                playPauseButton->setMinimumHeight(QScreen::instance()->height()>>3);
                playerButtonVbl->addStretch(1);
                QHBoxLayout* playerMainHbl = new QHBoxLayout();
                playerMainHbl->addLayout(playerButtonVbl);
                playerMainHbl->addWidget(playerBackgroundLbl,1);
                playerMainVbl->addLayout(playerMainHbl,1);
                playerBottomHbl->addStretch(1);
                progSlider = new ProgSlider ();
                connect (progSlider, SIGNAL(sliderReleased()), this, SLOT (onSliderReleased()));
                playerBottomHbl->addWidget(progSlider,4);
                playerBottomHbl->addWidget(durationLbl,1);
                playerBottomHbl->setMargin (0);
                playerBottomHbl->addStretch(1);
                playerMainVbl->addLayout(playerBottomHbl,0);
                podcastPlayer->setLayout(playerMainVbl);
            }
            stackWdg->addWidget (podcastPlayer);

            loadingView = new QWidget();
            HomeActionButton* cancelLoadBtn = new HomeActionButton(QObject::tr("Back"),QtopiaHome::Red);
            connect (cancelLoadBtn, SIGNAL(clicked()), this, SLOT(showDir()));
            QHBoxLayout* loadingHbl = new QHBoxLayout();
            QVBoxLayout* loadingVbl = new QVBoxLayout();
            loadingHbl->setMargin(0);
            loadingHbl->setSpacing(0);
            loadingVbl->setMargin(0);
            loadingVbl->setSpacing(0);
            loadingHbl->addWidget(cancelLoadBtn,0);
            loadingHbl->addWidget(new TitleLabel(QObject::tr("Loading Podcast"),this),1);
            loadingView->setLayout(loadingVbl);
            stackWdg->addWidget(loadingView);
            QLabel *loadingLbl = new QLabel(tr("Please Wait..."));
            loadingLbl->setAlignment(Qt::AlignCenter);
            loadingVbl->addLayout(loadingHbl,0);
            loadingVbl->addWidget(loadingLbl,1);
            playerSM = new PodcastPlayerSM(this);
            connect (playerSM, SIGNAL(enterState(PodcastPlayerSM::State)),this, SLOT(handleSMStateEnter(PodcastPlayerSM::State)));
            setWindowState(Qt::WindowMaximized);
        }
        void paintEvent (QPaintEvent* )
        {
            QPainter p (this);
            p.fillRect(QRect(0,0,width(),height()),QBrush(Qt::black));
        }
        void mouseReleaseEvent (QMouseEvent*)
        {
            if (fullScreen)
                fullScreenOff();
        }
    public slots:
        void showPodcast()
        {
            stackWdg->setCurrentWidget(podcastView);
            playerSM->sendEvent(PodcastPlayerSM::UserClose);
            playPauseButton->setChecked(true);
        }

        void onSliderReleased ()
        {
            int millis = progSlider->value ();
            if (millis < progSlider->progressValue() - 1000 && mediaControl)
            {
                mediaControl->seek (millis);
            }
        }

        void fullScreenOn ()
        {
            if (videoWidget)
                videoWidget->setGeometry(0,0,QScreen::instance()->width(),QScreen::instance()->height());
            fullScreen = true;
            grabMouse ();
        }
        void fullScreenOff ()
        {
            fullScreen = false;
            releaseMouse ();
            if (videoWidget)
            {
                videoWidget->setGeometry(QRect(playerBackgroundLbl->mapToGlobal(QPoint(0,0)),playerBackgroundLbl->size()));
            }
        }

        bool testBuffer (bool active)
        {
            if (mediaGetter)
            {
                int loadedBytes = mediaGetter->loadedBytes();
                int totalBytes = mediaGetter->totalBytes ();
                if (totalBytes <= 0)
                    totalBytes = curMediaFilesize;
                if (loadedBytes && loadedBytes == totalBytes)
                    return true;
                int downloadTime = startDownloadTime.elapsed ();
                int duration = curMediaDuration * 1000;
                int playedTime = mediaControl ? mediaControl->position() : 0;
                double bpms = duration ? ((double)totalBytes)/duration : 96;
                double dbpms = (double)loadedBytes / downloadTime;
                if ((bpms && duration && !totalBytes))
                    totalBytes = (int)(bpms * duration);
                double percLoaded = totalBytes ? ((double)loadedBytes)/totalBytes : 0;
                int playableTime = (int)(percLoaded * duration);
                int timeLeftToPlay = duration - playedTime;
                int timeLeftToDownload = (int)((totalBytes - loadedBytes)/dbpms);
                int availPlayable = playableTime - playedTime;
                return (availPlayable > 3000 && (active || timeLeftToDownload <= timeLeftToPlay || availPlayable > 8000));
            }
            else
                return false;
        }

        void playPause (bool p)
        {
            playerSM->sendEvent(p?PodcastPlayerSM::UserPlay:PodcastPlayerSM::UserPause);
        }
        void setMute (bool m)
        {
            mute = m;
            if (mediaControl)
                mediaControl->setMuted(m);
        }

        void initMediaPlayer()
        {
            if (mediaControl)
            {
                playerSM->sendEvent(PodcastPlayerSM::MediaValid);
            }
            else
            {
                if (mediaCtx)
                    delete mediaCtx;
                if (mediaContent)
                    delete mediaContent;
                if (mediaNotifier)
                    delete mediaNotifier;
                if (videoNotifier)
                    delete videoNotifier;

                mediaCtx = new QMediaContentContext( this );
                QUrl u(QString("file://")+mediaGetter->filename());
                mediaContent = new QMediaContent( u,QLatin1String( "Media" ), this );

                connect(mediaContent, SIGNAL(mediaError(QString)), this, SLOT(onMediaError(QString)));
                mediaNotifier = new QMediaControlNotifier( QMediaControl::name(),this);
                connect( mediaNotifier, SIGNAL(valid()), this, SLOT(onMediaValid()) );
                connect( mediaNotifier, SIGNAL(invalid()), this, SLOT(onMediaInvalid()) );
                mediaCtx->addObject(mediaNotifier);
                videoNotifier = new QMediaControlNotifier( QMediaVideoControl::name(),this);
                connect( videoNotifier, SIGNAL(valid()), this, SLOT(onVideoValid()) );
                connect( videoNotifier, SIGNAL(invalid()), this, SLOT(onVideoInvalid()) );
                mediaCtx->addObject(videoNotifier);
                mediaCtx->setMediaContent( mediaContent);
            }
        }
        void onMediaError(const QString& s)
        {
            qWarning() << "media error: " << s;
            playerSM->sendEvent(PodcastPlayerSM::MediaError);
        }
        void onMediaValid()
        {
            playerSM->sendEvent(PodcastPlayerSM::MediaValid);
            playerFullScreenButton->setEnabled(false);
        }
        void onVideoValid()
        {
            playerSM->sendEvent(PodcastPlayerSM::VideoValid);
            playerFullScreenButton->setEnabled(true);
        }
        void onMediaInvalid()
        {
            playerSM->sendEvent(PodcastPlayerSM::MediaInvalid);
        }
        void onVideoInvalid()
        {
            playerSM->sendEvent(PodcastPlayerSM::VideoInvalid);
            videoWidget->setGeometry(0,0,1,1);
            videoWidget->deleteLater();
        }
        void createVideoWidget()
        {
            QMediaVideoControl vidControl (mediaContent);
            if (videoWidget)
                delete videoWidget;
            videoWidget = vidControl.createVideoWidget (NULL);
            videoWidget->setParent (this);
            videoWidget->setGeometry(QRect(playerBackgroundLbl->mapToGlobal(QPoint(0,0)),playerBackgroundLbl->size()));
            videoWidget->show();
            connect(videoWidget,SIGNAL(destroyed()),this,SLOT(fullScreenOff()));
        }
        void onPlayerStateChanged(QtopiaMedia::State s)
        {
            if (s == QtopiaMedia::Playing)
            {
                playerSM->sendEvent(PodcastPlayerSM::MediaPlaying);
                playerFullScreenButton->setEnabled(true);
            }
            else
                playerSM->sendEvent(PodcastPlayerSM::MediaPaused);
        }

        void updateDurationLabel ()
        {
            if (mediaGetter->isDone() || mediaGetter->totalBytes() == mediaGetter->loadedBytes() )
            {
                if (mediaControl)
                    durationLbl->setText(QString("%1/%2").arg(dur2str(mediaControl?(mediaControl->position()/1000):0)).arg(dur2str(curMediaDuration)));
                else
                    durationLbl->setText("");
            }
            else
            {
                if (mediaGetter->totalBytes())
                {
                    durationLbl->setText(QString("%1").arg((int)((100*mediaGetter->loadedBytes())/mediaGetter->totalBytes()))+"%");
                }
            }
        }

        void onPosChanged (quint32 millis)
        {
            playerSM->sendEvent(PodcastPlayerSM::MediaProgress);
            progSlider->setValue(millis);
            updateDurationLabel ();
            mediaPos = millis;
        }
        void onLengthChanged (quint32 millis)
        {
//            if (millis > 0)
//                curMediaDuration = millis/1000;
//            progSlider->setMinimum(1);
//            progSlider->setMaximum(curMediaDuration);
        }
        void stopMediaPlayer()
        {
            if (mediaControl)
            {
                mediaControl->stop();
                mediaControl->deleteLater();
                mediaPos = 0;
            }
            progSlider->setValue(0);
        }

        void startMediaPlayer()
        {
            if (!mediaControl)
            {
                mediaControl = new QMediaControl (mediaContent);
                mediaControl->setMuted(mute);
                connect(mediaControl,SIGNAL(playerStateChanged(QtopiaMedia::State)),this,SLOT(onPlayerStateChanged(QtopiaMedia::State)));
                connect(mediaControl,SIGNAL(positionChanged(quint32)),this,SLOT(onPosChanged(quint32)));
                connect(mediaControl,SIGNAL(lengthChanged(quint32)),this,SLOT(onLengthChanged(quint32)));
            }
            mediaControl->start ();
            mediaControl->seek(mediaPos);
        }

        void handleSMStateEnter (PodcastPlayerSM::State st)
        {
            switch (st)
            {
                case PodcastPlayerSM::Error:
                    QMessageBox::warning(this, tr("Error Playing Podcast"),tr("Error Playing Podcast, Please Try Again Later"));
                case PodcastPlayerSM::Pausing:
                case PodcastPlayerSM::PausingToBuffer:
                    if (mediaControl)
                    {
                        mediaPos = mediaControl->position ();
                        mediaControl->pause ();
                    }
                break;
                case PodcastPlayerSM::Idle:
                    if (mediaGetter)
                        mediaGetter->abort ();
                    stopMediaPlayer();
                break;
                case PodcastPlayerSM::TestingBufferInactive:
                    if (testBuffer (false))
                        playerSM->sendEvent(PodcastPlayerSM::BufferOk);
                    else
                        playerSM->sendEvent(PodcastPlayerSM::BufferWait);
                break;
                case PodcastPlayerSM::TestingBufferActive:
                    if (testBuffer (true))
                        playerSM->sendEvent(PodcastPlayerSM::BufferOk);
                    else
                        playerSM->sendEvent(PodcastPlayerSM::BufferWait);
                break;
                case PodcastPlayerSM::InitializingPlayer:
                    initMediaPlayer();
                    break;
                case PodcastPlayerSM::CreatingVideoWidget:
                    createVideoWidget();
                    playerSM->sendEvent(PodcastPlayerSM::VideoWidgetCreated);
                    break;
                case PodcastPlayerSM::StartingPlayer:
                    startMediaPlayer();
                    break;
                default:
                break;
            }
        }

        void podcastClicked(const QModelIndex & mi)
        {
            QList<PodcastLoader*> l = findChildren<PodcastLoader*>();
            if (l.count() > mi.row ())
            {
                PodcastLoader* pl = l[mi.row()];
                if (pl)
                    pl->activate();
            }
        }

        void playMedia (WebFeedLink* lnk, const QString & ttl, const QString & dsc, int duration)
        {
            playerTitleLbl->setText(ttl);
            playerBackgroundLbl->setText(dsc);
            curMediaDuration = duration;
            curMediaFilesize = lnk->length;
            stackWdg->setCurrentWidget(podcastPlayer);
            if (!mediaGetter)
                mediaGetter = new WebLiteClient(this);
            mediaGetter->abort ();
            mediaGetter->setUrl(lnk->href);
            mediaGetter->setDirect(true);
            mediaGetter->load ();
            connect (mediaGetter, SIGNAL(progress()), this, SLOT(onDownloadProgress()));
            connect (mediaGetter, SIGNAL(error()), this, SLOT(onDownloadError()));
            startDownloadTime.restart ();
            playerSM->sendEvent(PodcastPlayerSM::Init);
            if (duration > 0)
            {
                progSlider->setMinimum(1);
                progSlider->setMaximum(duration*1000);
            }
        }

        void onDownloadProgress ()
        {
            playerSM->sendEvent(PodcastPlayerSM::DownloadProgress);
            if (mediaGetter->totalBytes())
            progSlider->setProgressPerc (((double)mediaGetter->loadedBytes() / mediaGetter->totalBytes()));
            updateDurationLabel ();
        }
        void onDownloadError()
        {
            playerSM->sendEvent(PodcastPlayerSM::DownloadError);
        }

        void podcastItemSelected(const QModelIndex & mi)
        {
            PodcastStandardItem* sti = (PodcastStandardItem*)podcastList->item(mi.row());
            int duration = -1;
            QList<WebFeedPodcastInfo*> lp = sti->rssItem->elementsByType<WebFeedPodcastInfo>();
            if (lp.size())
                duration = lp[0]->durationSecs;

            QList<WebFeedLink*> l = sti->rssItem->elementsByType<WebFeedLink>();
            for (int i=0; i < l.count(); ++i)
            {
                if (l[i]->rel == "enclosure" && (l[i]->type == "" ||l[i]->type.startsWith("video/") || l[i]->type.startsWith("audio/")))
                {
                    playMedia(l[i],sti->rssItem->title,sti->rssItem->description,duration);
                }
            }
            playerTitleLbl->setText(sti->rssItem->title);
            playerBackgroundLbl->setText(sti->rssItem->description);
            playPauseButton->setChecked(true);
        }

        void updatePodcastList ()
        {
        }

        void showPending ()
        {
            stackWdg->setCurrentWidget(loadingView);
        }

        void activatePodcast(WebFeedChannel* wfc)
        {
            curPodcast = *wfc;
            stackWdg->setCurrentWidget(podcastView);
            PodcastLoader* pl = (PodcastLoader*)sender ();
            podcastTitleLbl->setText(curPodcast.title);
            podcastDescLbl->setText(curPodcast.description.trimmed());
            QSize s(QScreen::instance()->width()>>2,QScreen::instance()->height()>>2);
            podcastIconLbl->setMaximumSize(s);
            if (pl->imgLoader)
                podcastIconLbl->setPixmap(pl->imgLoader->pixmap().scaled(s,Qt::KeepAspectRatio));
            else
                podcastIconLbl->setPixmap(QPixmap());
            podcastList->clear ();
            QList<WebFeedItem*> items = wfc->elementsByType<WebFeedItem>();
            for (QList<WebFeedItem*>::iterator it = items.begin(); it != items.end(); ++it)
            {
                new PodcastStandardItem(*it,podcastList);
            }
        }
        void showDir()
        {
            stackWdg->setCurrentWidget(mainMenuView);
            QList<PodcastLoader*> l = findChildren<PodcastLoader*>();
            for (QList<PodcastLoader*>::iterator it = l.begin(); it != l.end(); ++it)
            {
                PodcastLoader* pl = *it;
                pl->pending = false;
            }
        }
        void dirLoaded ()
        {
            QList<PodcastLoader*> l = findChildren<PodcastLoader*>();
            for (QList<PodcastLoader*>::iterator it = l.begin(); it != l.end(); ++it)
            {
                (*it)->deleteLater();
            }
            QStandardItemModel* directoryListModel = (QStandardItemModel*)directoryList->model();
            directoryListModel->clear ();
            QList<WebFeedItem*> items = dirLoader->root()->elementsByType<WebFeedItem>();
            int i=0;
            directoryListModel->setColumnCount(items.count()/2);
            directoryListModel->setRowCount(2);
            directoryList->setSpacing(15);
            for (QList<WebFeedItem*>::iterator it = items.begin(); it != items.end(); ++it,++i)
            {
                WebFeedItem* rssItem= *it;
                QStandardItem* stItem = new QStandardItem();
                PodcastLoader* loader = new PodcastLoader (rssItem, stItem, this);
                connect (loader, SIGNAL(updated()), directoryList, SLOT(update()));
                connect (loader, SIGNAL(showPending()),this,SLOT(showPending()));
                connect (loader, SIGNAL(updated()), this, SLOT(updatePodcastList()));
                connect (loader, SIGNAL(activated(WebFeedChannel*)), this, SLOT(activatePodcast(WebFeedChannel*)));
                directoryListModel->setItem(i,stItem);
            }
        }
};

QSXE_APP_KEY
int main (int argc, char** argv)
{
    QtopiaApplication app(argc,argv);
    PodcastViewApp* a = new PodcastViewApp();
    app.setMainWidget(a);
    a->show();
    app.exec();
}

#include "podcastviewer.moc"
