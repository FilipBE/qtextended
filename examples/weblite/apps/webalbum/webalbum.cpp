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

#include "webfeed.h"
#include "mediafeed.h"
#include "webliteimg.h"
#include "math.h"
#include <QScreen>
#include <QtopiaApplication>
#include <QPainter>
#include <QBitmap>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>
#include <QLabel>
#include <QDirectPainter>
#include <QTimer>
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <private/homewidgets_p.h>
#include <unistd.h>
#include <time.h>
//#define ENABLE_VC02
#ifdef ENABLE_VC02
#include "vc02device.h"
#include "vc02application.h"
#endif

class WebAlbum : public QWidget
{
    Q_OBJECT

    public:
        WebAlbum(QWidget* w);
        QPointer<WebLiteImg> imgLoader;
        QPointer<WebFeedLoader> feedLoader;
        QPointer<QDirectPainter> dp;
        bool paused;
        QImage curImage;
        QList<WebFeedItem*> items;
        QList<WebFeedItem*>::iterator curItem;
        WebFeedChannel* curChannel;
        WebMediaFeedParser mediaParser;
        QSize screenSize;
        bool fullScreenMode, transition;
        bool enableTransition, noDisplay;
        void paintEvent(QPaintEvent*);
        QPointer<QTimer> timer;
        QPointer<QTimer> fsTimer;
        bool noAction,imgDone;
        int delay;
#ifdef ENABLE_VC02
        vc02::Device* vc02device;
        vc02::Application* vc02app;
#endif

    public slots:
        void imageLoaded();
        void feedUpdated();
        void nextImage ();
        void prevImage();
        void loadImage();
#ifdef ENABLE_VC02
        void vc02free ()
        {
            if (vc02app)
                vc02device->endApplication(vc02app);
            vc02device->release ();
        }
        void vc02setup()
        {
            if (vc02device->haveDevice())
            {
                if (!vc02app)
                {
                    vc02app = vc02device->loadApplication("editor.vll");
                }
                if (vc02app)
                {
                    imgLoader->setAutoLoad(false);
                    QRect r = fullScreenMode ? QRect(0,0,QDirectPainter::screenWidth(),QDirectPainter::screenHeight()) : QRect(mapToGlobal(QPoint(0,0)),size());
                    vc02app->sendCommand (QString("ed_region display=0 dest=\"%1 %2 %3 %4\" mode=fill layer=7").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height()));
                    vc02app->waitForResult("ed_get_status","state","idle",10000);
                    if (imgLoader && imgLoader->filename().length())
                    {
                        QString newFilename = QDir::tempPath() + "/" +  imgLoader->filename().mid(imgLoader->filename().lastIndexOf("%")+2).replace(".jpeg",".jpg");
                        if (newFilename.endsWith(".jpeg"))
                            newFilename = newFilename.left(newFilename.size()-4) + "jpg";
                        QString sys = QString("ln -s -f %1 %2").arg(imgLoader->filename()).arg(newFilename);
                        system(sys.toAscii().constData());
                        QString cmd = QString ("ed_do pipeline=\"read_still, %1 hresize,%2 vresize,%3 convert,yuv display,display=0\"")
                                .arg(newFilename).arg(r.width()).arg(r.height());
                        vc02app->sendCommand(cmd);
                        vc02app->waitForResult("ed_get_status","state","idle",10000);
                    }
                }
            }
        }
#endif

        void fullScreenOff()
        {
            if (fsTimer)
                delete fsTimer;
            fullScreenMode = false;
            releaseMouse();
            transition = true;
            imageLoaded();
            transition = false;
//            parentWidget()->setFocus(Qt::ActiveWindowFocusReason);
            dp->setGeometry (QRect(mapToGlobal(QPoint(0,0)),size()));
            parentWidget()->update();
//            setFocus(Qt::ActiveWindowFocusReason);
            if (items.size() && curItem != items.end())
                emit nameChanged ((*curItem)->title);
        }

        void fullScreenOn()
        {
            grabMouse ();
            fullScreenMode = true;
            imageLoaded();
            dp->setGeometry (QRect(0,0,QDirectPainter::screenWidth(),QDirectPainter::screenHeight()));
        }

        void pause ()
        {
            paused = true;
            timer->stop ();
            if (fsTimer)
                delete fsTimer;
        }
        void resume ()
        {
            paused = false;
            nextImage ();
            if (fsTimer)
                delete fsTimer;
        }

    protected:
        void mouseReleaseEvent(QMouseEvent*)
        {
            if (fullScreenMode)
                fullScreenOff();
        }
        void showEvent(QShowEvent*)
        {
//            dp->raise();
//            dp->setGeometry (QRect(mapToGlobal(QPoint(0,0)),size()));
//            fullScreenOff();
        }
        bool event(QEvent* e)
        {
            if (e->type() == QEvent::WindowDeactivate || e->type() == QEvent::Hide)
            {
                parentWidget()->close();
            }
//                hide ();
            return QWidget::event(e);
//            fullScreenOff();
        }

    public:
        virtual ~WebAlbum();

    signals:
        void nameChanged (const QString & );
};
void WebAlbum::paintEvent(QPaintEvent*)
{
    QPainter p (this);
    p.fillRect(rect(),QBrush(Qt::black));
    if (curImage.isNull())
        p.drawText(rect(),Qt::AlignHCenter|Qt::AlignVCenter,tr("Loading..."));
    else
        p.drawImage(mapFromGlobal(QPoint(0,0)),curImage);
//        p.drawImage(0,mapFromGlobal(QPoint(0,0)).y(),curImage);
}

void WebAlbum::loadImage()
{
    imgDone = false;
    if (curItem != items.end())
    {
        imgLoader->setUrl
                (bestImageUrlForFeedItem(*curItem,screenSize));
        imgLoader->setScale(screenSize);
        imgLoader->load ();
    }
}

void WebAlbum::nextImage()
{
    if (!items.empty())
    {
        timer->stop ();
        curItem++;
        if (curItem == items.end())
            curItem = items.begin();
        loadImage ();

        QList<WebFeedItem*>::iterator next = curItem+1;
        if (next == items.end())
            next = items.begin();
    }

//    WebLiteClient::loadInBackground(bestImageUrlForFeedItem(*next,screenSize));
}
void WebAlbum::prevImage()
{
    timer->stop();
    if (curItem == items.begin())
        curItem = items.end();
    --curItem;
    loadImage ();
}
void WebAlbum::feedUpdated()
{
    QString curId;
    if (curChannel)
    {
        curId = (*curItem)->uid;
    }
    curChannel = (WebFeedChannel*)feedLoader->root();
    items = curChannel->items();
    curItem = items.end () -1;
    nextImage ();
}
struct FbEffect
{
    int pxWid, scrh,scrw,linestep,numBytes,randnum;
    QRect rct;
    uchar* fb;
//    static uchar* sourceFb;
    const uchar* targetFb;

    inline int offset (int x, int y)
    {
        return ((linestep*y) + (pxWid*x)) % numBytes;
    }

    inline QRgb sourcePixel (int x, int y)
    {
        return *(QRgb*)(fb + offset (x,y));
    }
    inline QRgb targetPixel (int x, int y)
    {
        return *(QRgb*)(targetFb + offset (x,y));
    }

    inline void setPixel (int x, int y, QRgb color)
    {
        memcpy (fb + offset(x,y), &color, sizeof(color));
    }

    inline void copy (bool target, int x, int y, int numPixels = 1)
    {
        if (target)
        {
            int ofs = offset(x,y);
            memcpy (fb + ofs, (targetFb) + ofs, numPixels * pxWid);
        }
    }

    virtual ~FbEffect () {}

    void destroy ()
    {
    }

    void init(const QImage & nImg, const QRect & r)
    {
        rct = r;
        randnum = rand ();
        fb = QDirectPainter::frameBuffer ();
        scrh = QDirectPainter::screenHeight();
        scrw = QDirectPainter::screenWidth();
        pxWid = QDirectPainter::screenDepth() >> 3;
        linestep = scrw * pxWid;
        numBytes = scrh * linestep;
        targetFb = nImg.bits();
    }

    virtual void processPixel (int, int, double)
    {
    }

    virtual void processFrame (double perc)
    {
        for (int y=rct.y(); y < rct.bottom(); ++y)
        {
            for (int x=rct.x(); x < rct.right(); ++x)
            {
                processPixel(x,y,perc);
            }
        }
    }
};
//uchar* FbEffect::sourceFb = NULL;

WebAlbum::~WebAlbum()
{
#ifdef ENABLE_VC02
    vc02free();
#endif
//    if (FbEffect::sourceFb)
//    {
//        delete FbEffect::sourceFb;
//        FbEffect::sourceFb = NULL;
//    }
}

struct FbEffect_Blend : public FbEffect
{
    static const int ACCURACY = 10000;
    int wnPerc;
    double prevPerc;
    FbEffect_Blend() : FbEffect() ,prevPerc(0)
    {
    }
    inline int blend (int o, int n, int factor)
    {
            return (n*factor + o*(ACCURACY-factor))/ACCURACY;
    }

    virtual void processPixel (int x, int y, double)
    {
        QRgb oldPx = sourcePixel(x,y);
        QRgb newPx = targetPixel(x,y);

        QRgb blended = qRgb (
                blend (qRed(oldPx), qRed(newPx), wnPerc),
        blend (qGreen(oldPx), qGreen(newPx), wnPerc),
        blend (qBlue(oldPx), qBlue(newPx), wnPerc));
        setPixel(x,y,blended);
    }

    void processFrame (double fperc)
    {
        double fp = (fperc == 1) ? fperc : fperc*(1-prevPerc);
        wnPerc = (int)(fp*ACCURACY);
        FbEffect::processFrame(fp);
        prevPerc = fperc;
    }
};

struct FbEffect_Checkerboard : public FbEffect
{
    static const int square_length = 8;
    void processPixel(int x, int y, double fperc)
    {
        copy ((x%(square_length*2)>square_length)^(y%(square_length*2)>square_length)|| (x % square_length < (int)(fperc*square_length)),x,y);
    }
};

struct FbEffect_Blinds : public FbEffect
{
    const static int blind_height = 16;
    void processPixel(int x, int y, double fperc)
    {
        copy (((randnum%2?x:y) % blind_height) < (fperc*blind_height),x,y);
    }
};

struct FbEffect_Box : public FbEffect
{
    QImage img;
    void processFrame(double fperc)
    {
        QBitmap bmp (scrw,scrh);
        bmp.fill(Qt::color1);
        {
            QPainter p (&bmp);
            QRectF r = rct;
            r.adjust(fperc*r.width()/2,fperc*r.height()/2,0-(fperc*r.width()/2),0-(fperc*r.height()/2));
            p.fillRect(r,QBrush(Qt::color0));
        }
        img = bmp.toImage ();
        FbEffect::processFrame(fperc);
    }

    void processPixel(int x, int y, double)
    {
        copy (img.pixel(x,y) == Qt::color1,x,y);
    }
};
struct FbEffect_Circle : public FbEffect
{
    QImage img;
    void processFrame(double fperc)
    {
        QBitmap bmp (scrw,scrh);
        bmp.fill(Qt::color1);
        {
            QPainter p (&bmp);
            QRectF r = rct;
            r.adjust(fperc*r.width()/2,fperc*r.height()/2,0-(fperc*r.width()/2),0-(fperc*r.height()/2));
            QPainterPath pp;
            pp.addEllipse(r);
            p.fillPath(pp,QBrush(Qt::color0));
        }
        img = bmp.toImage ();
        FbEffect::processFrame(fperc);
    }

    void processPixel(int x, int y, double)
    {
        copy (img.pixel(x,y) == Qt::color1,x,y);
    }
};

struct FbEffect_Clock : public FbEffect
{
    void processFrame (double fperc)
    {
        QBitmap bmp (scrw*2,scrh*2);
        bmp.fill(Qt::color0);
        {
            QPainter p (&bmp);
            p.setBrush(QBrush(Qt::color1));
            p.drawPie (QRect(0,0,scrw*2,scrh*2),0,(int)(fperc*360*16));
        }
        QImage img = bmp.toImage ();
        for (int y=rct.y(); y < rct.bottom(); ++y)
        {
            for (int x=rct.x(); x < rct.right(); ++x)
            {
                copy((img.pixel(QPoint(x+(scrw>>1),y+(scrh>>1))) & 0xffffff)== 0,x,y);
            }
        }
    }
};

void WebAlbum::imageLoaded()
{
    if (!isVisible())
        return;
    bool loadNext = true;
    if (sender () == imgLoader && timer->isActive())
    {
        imgDone = true;
        return;
    }
    else if (sender() == timer && !imgDone)
        return;
#ifdef ENABLE_VC02
    if (vc02app)
    {
        vc02setup ();
    }
    else
    // didn't work - use CPU
#endif
    {
        if (imgLoader)
            curImage = imgLoader->image ();
        if (!noDisplay)
        {
            if (curImage.isNull())
            {
                QPixmap pxm (QDirectPainter::screenWidth(),QDirectPainter::screenHeight());
                pxm.fill(QColor(0,0,0));
                {
                    QPainter p (&pxm);
                    p.setFont(font());
                    p.drawText(QRect(0,0,QDirectPainter::screenWidth(),QDirectPainter::screenHeight()),Qt::AlignHCenter|Qt::AlignVCenter,tr("Loading..."));

                }
                loadNext = false;
                curImage = pxm.toImage();
            }

            if (!fullScreenMode && (curImage.width() > width() || curImage.height() || height()))
            {
                curImage = curImage.scaled(size(),Qt::KeepAspectRatio);
            }
            QRect scrRect(0,0,QDirectPainter::screenWidth(),QDirectPainter::screenHeight());
            QRect rct (scrRect);
            if (!fullScreenMode)
                    rct &= (QRect(mapToGlobal(QPoint(0,0)),size()));
            QSize ssize(QDirectPainter::screenWidth(),QDirectPainter::screenHeight());
            if (curImage.isNull())
            {
                nextImage ();
                return;
            }
            if (curImage.size() != rct.size())
            {
                QPixmap pxm_new(ssize);
                QPixmap pxm = QPixmap::fromImage(curImage);

                int margin = 4;
                if (curImage.width() > rct.width()-(margin*2) || curImage.height () > rct.height()-(margin*2))
                    pxm = pxm.scaled(QSize(rct.width()-(margin*2),rct.height()-(margin*2)),Qt::KeepAspectRatio);
                {
                    QPainter p (&pxm_new);
                    p.fillRect(QRect(QPoint(0,0),ssize),QBrush(QColor(0,0,0)));
                    QRect pxmRect(rct.x()+((rct.width()-pxm.width())>>1),rct.y()+((rct.height()-pxm.height())>>1),pxm.width(),pxm.height());
                    QRect bgRect (pxmRect);
                    bgRect.adjust (1-margin,1-margin,margin-2,margin-2);
                    p.fillRect(bgRect,QBrush(QColor(0,0,0,0x99)));
                    p.setPen(QPen(QColor(0xff,0xff,0xff,0xcc)));
                    p.drawRect(bgRect);
                    p.drawPixmap (pxmRect.left(),pxmRect.top(),pxm);
                }
                curImage = pxm_new.toImage();
            }
        //    if (fullScreenMode)
            {
                QPoint leftTop (0,0);
                if (leftTop.x() < 0)
                    leftTop = QPoint(0,0);
                dp->startPainting();

                int numFrames = enableTransition?5:1;
                double curve = 0.8;

                FbEffect* fx;
    #define ADD_EFFECT(fx) \
                fx _##fx; \
                v.push_back (&_##fx);

                QVector<FbEffect*> v;
                ADD_EFFECT(FbEffect_Blend)
                ADD_EFFECT(FbEffect_Checkerboard)
    //            ADD_EFFECT(FbEffect_Blinds)
                ADD_EFFECT(FbEffect_Clock)
    //            ADD_EFFECT(FbEffect_Box)
    //            ADD_EFFECT(FbEffect_Circle)

                fx = v[rand()%v.size()];
        //        fx = &fx_blend;
                fx->init(curImage,transition?scrRect:rct);
                QPointer<QWidget> p = this;
                for (int i=0; i < numFrames; ++i)
                {
                    double perc = ((double)(i+1))/numFrames;
                    double fperc = pow(perc,curve);
                    fx->processFrame (fperc);
                    qt_screen->setDirty(rct);
                    qApp->sendPostedEvents ();
                    if (!p)
                    {
                        return;
                    }
                }
                fx->destroy ();
                dp->endPainting();
            }
        //    if (fsTimer)
        //        fsTimer->start (4000);
        }
    }
    if (loadNext)
    {
        if (!fullScreenMode && !items.empty() && curItem != items.end())
            emit nameChanged ((*curItem)->title);
        nextImage();
    }
    if (!paused)
        timer->start (delay);
    update ();
}

WebAlbum::WebAlbum(QWidget* w) : QWidget(w)
        ,paused(false),curChannel(NULL),transition(false),imgDone(false)
#ifdef ENABLE_VC02
        ,vc02device(NULL),vc02app(NULL)
#endif
{
    timer = new QTimer (this);
    timer->setSingleShot(true);
    fsTimer = new QTimer(this);
    fsTimer->setSingleShot(true);
    connect (fsTimer, SIGNAL(timeout()),this,SLOT(fullScreenOn()));
    connect(timer,SIGNAL(timeout()),this,SLOT(imageLoaded()));
    screenSize = QSize(QScreen::instance()->width(),QScreen::instance()->height());
    QSettings sett ("Trolltech","webalbum");
    feedLoader = new WebFeedLoader(this);
    imgLoader = new WebLiteImg(this);
    imgLoader->setTimeout(10000);
    sett.beginGroup("album");
    feedLoader->setUrl (sett.value("feed-url",QString("http://picasaweb.google.com/data/feed/base/user/charleshenri.lefevre/albumid/5117131121066805025?kind=photo&alt=rss&hl=fr&thumbsize=%1").arg(QScreen::instance()->width())).toString());
    delay = sett.value("delay",5000).toInt();
    enableTransition = sett.value("transitions",true).toBool();
    noDisplay = sett.value("no-display",false).toBool();
//    feedLoader->setUrl (sett.value("album/feed-url","http://api.flickr.com/services/feeds/photos_public.gne").toString());
//    feedLoader->setUrl (sett.value("album/feed-url","http://rss.framechannel.com/user=noam/pin=2792").toString());
    connect (feedLoader, SIGNAL(updated()), this, SLOT(feedUpdated()));
    connect (imgLoader, SIGNAL(decoded()), this, SLOT(imageLoaded()));
    connect (imgLoader, SIGNAL(error()), this, SLOT(nextImage()));
    feedLoader->installExtension(&mediaParser);
    feedLoader->load ();
    setWindowState(Qt::WindowMaximized);
    dp = new QDirectPainter (this,QDirectPainter::Reserved);
    fullScreenMode = false;
    if (!noDisplay)
        dp->raise();
    setEnabled(true);
    qApp->installEventFilter(this);
#ifdef ENABLE_VC02
    vc02device = vc02::Device::instance();
    vc02device->request(vc02::Device::Low);
    connect(vc02device,SIGNAL(available()),this,SLOT(vc02setup()));
#endif
}

class WebAlbumApp: public QWidget
{
    Q_OBJECT

    public:
    WebAlbumApp(QWidget* w, Qt::WindowFlags wf) : QWidget(w,wf)
    {
        srand(clock());
        WebAlbum* album = new WebAlbum(NULL);
        QLabel* lbl = new QLabel("Web Album");
        lbl->setWordWrap(true);
        lbl->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        QVBoxLayout* vbl_all = new QVBoxLayout();
        QHBoxLayout* hbl_main = new QHBoxLayout();
        QGridLayout* hbl_top = new QGridLayout();
        HomeActionButton *closeButton = new HomeActionButton(QObject::tr("Close"),QtopiaHome::Red);
        HomeActionButton *zoomButton = new HomeActionButton(QObject::tr("Full\nScreen"),QtopiaHome::Green);
        setLayout(vbl_all);
        hbl_top->addWidget(closeButton,0,0,1,1);
        hbl_top->addWidget(lbl,0,1,1,4);
        hbl_top->addWidget(zoomButton,0,5,1,1);
        hbl_main->setMargin(0);
        vbl_all->setMargin(0);
        QObject::connect(album,SIGNAL(nameChanged(QString)),lbl,SLOT(setText(QString)));
    //    QObject::connect(album,SIGNAL(destroyed()),dlg,SLOT(deleteLater()));
        QObject::connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
        QObject::connect(zoomButton,SIGNAL(clicked()),album,SLOT(fullScreenOn()));
        setWindowState(Qt::WindowMaximized);
        vbl_all->addLayout(hbl_top,0);
        vbl_all->addLayout(hbl_main,1);
        vbl_all->setSpacing(0);
        vbl_all->setMargin(0);
    //    hbl_main->addLayout(vbl_buttons);
        hbl_main->addWidget(album,2);
//        album->setFocus();
//        zoomButton->setFocusPolicy(Qt::NoFocus);
//        closeButton->setFocusPolicy(Qt::NoFocus);
//        album->setFocusPolicy(Qt::StrongFocus);
    }
};

QSXE_APP_KEY
int main (int argc, char** argv)
{
    QtopiaApplication app(argc,argv);
    WebAlbumApp* a = new WebAlbumApp(NULL,0);
    app.setMainWidget(a);
    a->show();
    app.exec();
}

#include "webalbum.moc"
