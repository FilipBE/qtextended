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

#include <QApplication>
#include <sched.h>
#include "gfxcanvaslist.h"
#include "simplehighlight.h"
#include "photoriver.h"
#include "textedit.h"
#include "camera.h"
#include "timeview.h"
#include <QDirectPainter>
#include "gfx.h"
#include <QKeyEvent>
#include <QDir>
#include "imagecollection.h"
#include <QPainter>
#include <QTime>
#include <QImage>
#include <QMatrix>
#include <QThread>
#include <QPixmap>
#include <QLabel>
#include <QDebug>
#include <GfxPainter>
#include <GfxTimeLine>
#include "gfximageloader.h"
#include <GfxMipImage>
#include "gfxcanvas.h"
#include "gfxmenu.h"
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "softkeybar.h"
#include "tagdialog.h"
#include <QMutex>
#include "header.h"

static bool runThreaded = false;

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    MyWidget();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);

private slots:
    void cameraStateChanged(Camera::State);
    void iconListActivated();
    void showTimeView();
    void showTagView();
    void showPhotoRiver();
    void showGotoDate();
    void showCamera();
    void imageListActivated();
    void showMenu();
    void hideMenu();

private:
    void init();
    void initMenu();

    GfxCanvas canvas;

    GfxCanvasWindow menuwindow;
    GfxCanvasWindow window;
    GfxCanvasWindow tvwindow;
    PhotoRiver *river;
    GfxCanvasReflection reflection;
    GfxCanvasClip collectionClip;
    GfxCanvasList iconList;
    SoftKeyBar skb;
    GfxTimeLine tl;
    GfxMenu menu;
    GfxCanvasColor menuColor;
    TimeView timeView;

    Camera *camera;

    Qt::Key lastKey;

    ImageCollection *collection() const
    {
        return collections.at((iconList.current()!=-1)?iconList.current():0);
    }

    QList<ImageCollection *> collections;
    enum State { None, Expanded, Collapsed, Viewing };
    State state;
    void setState(State);

};

QImage img;
GfxCanvas *globalCanvas = 0;
uchar *fb = 0;

MyWidget::MyWidget()
: canvas(this, runThreaded?GfxCanvas::Threaded:GfxCanvas::Widget), menuwindow(QSize(240, 320), canvas.root()), window(QSize(240, 320), canvas.root()), tvwindow(QSize(240, 320), canvas.root()), river(0), reflection(QRect(0, 240, 240, 40), window.windowItem()), collectionClip(window.windowItem()), iconList(&collectionClip), skb(QSize(240, 30), canvas.root()), menu(menuwindow.windowItem()), menuColor(Qt::black, QSize(240, 320), canvas.root()), timeView(tvwindow.windowItem()), camera(0), lastKey(Qt::Key_unknown)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    Header *h = new Header(canvas.root());
    h->z().setValue(10000);
    tvwindow.visible().setValue(0.);

    iconList.width().setValue(240);
    iconList.height().setValue(260);
    iconList.setHighlight(new SimpleHighlight(&iconList));
    SoftKeyBar::setLabel(&iconList, SoftKeyBar::Middle, "Expand");
    init();
    iconList.setFocused(true);
    state = Collapsed;

    reflection.y().setValue(281);
    reflection.z().setValue(99);
    canvas.addDynamicItem(&reflection);
    skb.y().setValue(320 - 15);
    skb.z().setValue(101);
    collectionClip.y().setValue(20);
    collectionClip.width().setValue(240);
    collectionClip.height().setValue(260);

    skb.z().setValue(100.);
    menuwindow.z().setValue(99.);
    menuwindow.setAttribute("popup");
    menuColor.z().setValue(98.);
    window.z().setValue(97.);

    QObject::connect(&iconList, SIGNAL(activated(GfxCanvasListItem*)), 
                     this, SLOT(iconListActivated()));

    globalCanvas = &canvas;
    window.activate();

    showPhotoRiver();
}

QImage ri;
void repaintCanvas()
{
    QTime timer;
    if(globalCanvas) {
        timer.start();
        QRegion r = globalCanvas->resetDirty();
        if(r.isEmpty())
            return;
        r &= img.rect();
        GfxPainter p(img, r);
        QColor c = Qt::black;
        // XXX  
        p.fillRect(QRect(0, 0, 240, 320), c);
        globalCanvas->paint(p);
        uchar *imgBits = img.bits();
        if(r.isEmpty()) {
            ::memcpy(fb, imgBits, 320 * 240 * 2);
        } else {
        QVector<QRect> rects = r.rects();
        for(int ii = 0; ii < rects.count(); ++ii) {
            QRect r = rects.at(ii);
            int x = r.left();
            int w = r.width();
            for(int yy = r.top(); yy <= r.bottom(); ++yy) {
                ::memcpy(fb + yy * 240 * 2 + x * 2, imgBits + yy * 240 * 2 + x * 2,
                         w * 2);
            }
        }
        }
        globalCanvas->addFrameTime(timer.elapsed(), r);
    }
}

void MyWidget::paintEvent(QPaintEvent *e)
{
    GfxTimeLine::lock()->lock();
    GfxPainter p(this, e);
    p.fillRect(rect(), Qt::black);
    canvas.paint(p);
    GfxTimeLine::lock()->unlock();
}

void MyWidget::keyReleaseEvent(QKeyEvent *e)
{
    if(e->isAutoRepeat())
        return;
    if(e->key() != lastKey)
        return;
    lastKey = Qt::Key_unknown;

    GfxTimeLine::lock()->lock();

    if(e->key() == Qt::Key_Select) {
        skb.release(SoftKeyBar::Middle);
    } else if(e->key() == Qt::Key_Context1) {
        skb.release(SoftKeyBar::Left);
    } else if(e->key() == Qt::Key_Back) {
        skb.release(SoftKeyBar::Right);
    }

    e->setAccepted(false);
    canvas.keyReleaseEvent(e);
    if(e->isAccepted()) {
        GfxTimeLine::lock()->unlock();
        return;
    }

    e->setAccepted(true);

    if(state == Viewing) {
        imageListActivated();
    } else if(e->key() == Qt::Key_Context1 || (menu.visible().value() && e->key() == Qt::Key_Back)) {
        if(menu.visible().value()) {
            hideMenu();
        } else {
            showMenu();
        }
    } else if(e->key() == Qt::Key_Back) {
        if(state == Expanded) {
            setState(Collapsed);
            tl.complete();

            collections.at(iconList.current())->setState(ImageCollection::Collapsed);
            iconList.setActive(true);
            iconList.setFocused(true);
        }
    }
    GfxTimeLine::lock()->unlock();
}

void MyWidget::showTimeView()
{
    tvwindow.activate();
    window.visible().setValue(0.);
    tvwindow.visible().setValue(1.);
    if(camera) { delete camera; camera = 0; }
    if(river) { delete river; river = 0; }

    if(menu.visible().value()) {
        tvwindow.cached().setValue(1.);
        tvwindow.scale().setValue(.9);
    } else {
        tvwindow.cached().setValue(0.);
        tvwindow.scale().setValue(1.);
    }

    hideMenu();
}

void MyWidget::showCamera()
{
    window.visible().setValue(0.);
    tvwindow.visible().setValue(0.);
    if(river) { delete river; river = 0; }
    if(!camera) {
        camera = new Camera(canvas.root());
        camera->x().setValue(120);
        camera->y().setValue(160);
        QObject::connect(camera, SIGNAL(stateChanged(Camera::State)), this, SLOT(cameraStateChanged(Camera::State)));
        cameraStateChanged(camera->state());
        camera->setFocused(true);
    }
    hideMenu();

    camera->activate();
}

void MyWidget::showPhotoRiver()
{
    window.visible().setValue(0.);
    tvwindow.visible().setValue(0.);
    if(!river) 
        river = new PhotoRiver(canvas.root());
    river->visible().setValue(1.);
    river->activate();
    static_cast<GfxCanvasItem *>(river)->setFocused(true);
    if(camera) { delete camera; camera = 0; }

    hideMenu();
}

void MyWidget::showTagView()
{
    window.visible().setValue(1.);
    tvwindow.visible().setValue(0.);
    window.activate();
    if(camera) { delete camera; camera = 0; }
    if(river) { delete river; river = 0; }

    if(menu.visible().value()) {
        window.cached().setValue(1.);
        window.scale().setValue(.9);
    } else {
        window.cached().setValue(0.);
        window.scale().setValue(1.);
    }

    hideMenu();
}

void MyWidget::showGotoDate()
{
    showTimeView();
    timeView.showDateSelector();
}

void MyWidget::showMenu()
{
    if(menu.visible().value() != 0.)
        return;

    GfxCanvasWindow *win = window.visible().value()?&window:&tvwindow;

    menu.scale().setValue(1.2);
    tl.move(menu.visible(), 1., 150);
    tl.move(menu.scale(), 1., 150);
    tl.move(menuColor.visible(), .6, 150);
    win->cached().setValue(1.);
    tl.move(win->scale(), .9, 150);
    menu.setFocused(true);
    menuwindow.activate();
}

void MyWidget::hideMenu()
{
    if(menu.visible().value() == 0.)
        return;

    GfxCanvasWindow *win = window.visible().value()?&window:&tvwindow;

    tl.move(menu.visible(), 0., 150);
    tl.move(menu.scale(), 1.2, 150);
    tl.move(menuColor.visible(), .0, 150);
    tl.move(win->scale(), 1, 150);
    tl.sync(win->cached(), win->scale());
    tl.set(win->cached(), 0.);
    menuwindow.deactivate();

    tl.pause(menu, 150);
    tl.execute(menu.resetEvent());
}

void MyWidget::imageListActivated()
{
    if(state == Expanded) {
        setState(Viewing);
        collection()->setState(ImageCollection::View);
    } else if(state == Viewing) {
        setState(Expanded);
        collection()->setState(ImageCollection::Expanded);
    }
}

void MyWidget::cameraStateChanged(Camera::State s)
{
    if(s == Camera::Live) {
        SoftKeyBar::setLabel(camera, SoftKeyBar::Left, QString());
        SoftKeyBar::setLabel(camera, SoftKeyBar::Middle, "Capture");
        SoftKeyBar::setLabel(camera, SoftKeyBar::Right, QString());
    } else if(s == Camera::Reviewing) {
        SoftKeyBar::setLabel(camera, SoftKeyBar::Left, QString());
        SoftKeyBar::setLabel(camera, SoftKeyBar::Middle, "Live");
        SoftKeyBar::setLabel(camera, SoftKeyBar::Right, "Delete");
    } else if(s == Camera::Deleting) {
        SoftKeyBar::setLabel(camera, SoftKeyBar::Left, "Yes");
        SoftKeyBar::setLabel(camera, SoftKeyBar::Middle, "");
        SoftKeyBar::setLabel(camera, SoftKeyBar::Right, "No");
    }
}

void MyWidget::iconListActivated()
{
    setState(Expanded);
    tl.complete();
    if(collection()->listItem()->current() == -1)
        collection()->listItem()->setCurrent(0);
    collection()->setState(ImageCollection::Expanded);
    iconList.setActive(false);
}

void MyWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->isAutoRepeat())
        return;
    if(lastKey != Qt::Key_unknown)
        return;
    lastKey = (Qt::Key)e->key();

    GfxTimeLine::lock()->lock();

   if(e->key() == Qt::Key_Select) {
        skb.press(SoftKeyBar::Middle);
    } else if(e->key() == Qt::Key_Context1) {
        skb.press(SoftKeyBar::Left);
    } else if(e->key() == Qt::Key_Back) {
        skb.press(SoftKeyBar::Right);
    }

    e->setAccepted(false);
    canvas.keyPressEvent(e);
    if(!e->isAccepted()) {
        e->setAccepted(true);
        
        if(e->key() == Qt::Key_0)
            abort();
        else if(e->key() == Qt::Key_9) 
            canvas.dumpTiming();
        else if(e->isAutoRepeat()) {
        } else if(e->key() == Qt::Key_Asterisk) {
            GfxTimeLine::setSlowMode(!GfxTimeLine::slowMode());
        }
    }

    GfxTimeLine::lock()->unlock();
}

class ICListItem : public GfxCanvasListItem
{
public:
    ICListItem(int idx, ImageCollection *, GfxCanvasItem *);

    virtual QSize size() const;
    virtual void activate();
    virtual void deactivate();
    virtual void focusIn();
    virtual void focusOut();

    GfxTimeLine tl;
    int idx;
};

ICListItem::ICListItem(int i, ImageCollection *c, GfxCanvasItem *p)
: GfxCanvasListItem(p), idx(i)
{
    c->iconItem()->setParent(this);
}

QSize ICListItem::size() const 
{
    return QSize(120, 120);
}

void ICListItem::focusIn()
{
    raised().setValue(1.);
}

void ICListItem::focusOut()
{
    raised().setValue(0.);
}

void ICListItem::activate()
{
    tl.clear();

    if(isFocused()) {
        visible().setValue(1.);
        return;
    }

    tl.move(visible(), 1., 150);
}

void ICListItem::deactivate()
{
    tl.clear();

    if(isFocused()) {
        return;
    }

    tl.move(visible(), 0., 150);
}

void MyWidget::init()
{
    QDir dir("collections/");
    QList<GfxCanvasListItem *> items;

    foreach(QString collection, 
            dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir files("collections/" + collection, "*.png");
        QStringList imgFiles;
        foreach(QString file, files.entryList()) {
            QString ifile = "collections/" + collection + "/" + file;
            imgFiles << ifile;
        }
        files = QDir("collections/" + collection, "*.jpg");
        foreach(QString file, files.entryList()) {
            QString ifile = "collections/" + collection + "/" + file;
            imgFiles << ifile;
        }

        ImageCollection *col = 
            new ImageCollection(&iconList, collection, imgFiles);
        col->setState(ImageCollection::Collapsed);
        col->setListRect(QRect(0, 0, 240, 260));
        col->viewingItem()->setParent(&window);
        col->viewingItem()->x().setValue(119);
        col->viewingItem()->y().setValue(160);
        col->viewingItem()->z().setValue(10000);

        QObject::connect(col->listItem(), 
                         SIGNAL(activated(GfxCanvasListItem*)), 
                         this, 
                         SLOT(imageListActivated()));
        col->z().setValue(-collections.count());
        items << new ICListItem(collections.count(), col, &iconList);
        collections << col;
    }
    iconList.setItems(items);
    setState(Collapsed);
    iconList.setCurrent(0);
    iconList.setActive(true);

    SoftKeyBar::setLabel(canvas.root(), SoftKeyBar::Left, "Options");

    initMenu();
}

void MyWidget::initMenu()
{
    GfxMenuItem *item = new GfxMenuItem(QString(), this);
    GfxMenuItem *tvmi = new GfxMenuItem("Time View", item);
    item->appendItem(tvmi);
    QObject::connect(tvmi, SIGNAL(activated()), this, SLOT(showTimeView()));
    GfxMenuItem *cmi = new GfxMenuItem("Camera", item);
    item->appendItem(cmi);
    QObject::connect(cmi, SIGNAL(activated()), this, SLOT(showCamera()));
    GfxMenuItem *tv = new GfxMenuItem("Tag View", item);
        tv->appendItem(new GfxMenuItem("Names", tv));
        tv->appendItem(new GfxMenuItem("Places", tv));
        tv->appendItem(new GfxMenuItem("Events", tv)); 
        for(int ii = 0; ii < tv->count(); ++ii)
            QObject::connect(tv->item(ii), SIGNAL(activated()), this, SLOT(showTagView()));
    item->appendItem(tv);
//    item->appendItem(new GfxMenuItem("Add Tag", item));
    GfxMenuItem *pr = new GfxMenuItem("Photo River", item);
    QObject::connect(pr, SIGNAL(activated()), this, SLOT(showPhotoRiver()));
    item->appendItem(pr);
    GfxMenuItem *gtd = new GfxMenuItem("Goto Date", item); 
    QObject::connect(gtd, SIGNAL(activated()), this, SLOT(showGotoDate()));
    item->appendItem(gtd);

    menu.y().setValue(110.);
    menu.visible().setValue(0.);
    menuColor.visible().setValue(0.);
    menuColor.x().setValue(120);
    menuColor.y().setValue(160);
    menu.setMenu(item);

    SoftKeyBar::setLabel(&menu, SoftKeyBar::Middle, "Select");
    SoftKeyBar::setLabel(&menu, SoftKeyBar::Right, "");
}

void MyWidget::setState(State s)
{
    state = s;
}

class MyThread : public QThread
{
public:
    virtual void run()
    {
        GfxTimeLine::setClockThread();
    }
};


// #define TEST
#ifdef TEST
bool use_int = false;
class MyWidget2 : public QWidget 
{
    Q_OBJECT
public:
    MyWidget2()
        : canvas(this)
    {
    }

    virtual void keyPressEvent(QKeyEvent *e)
    {
#if 0
        if(e->isAutoRepeat())
            return;
        e->setAccepted(false);
        canvas.keyPressEvent(e);
        e->accept();
#endif
    }

    virtual void keyReleaseEvent(QKeyEvent *e)
    {
#if 0
        if(e->isAutoRepeat())
            return;
        e->setAccepted(false);
        canvas.keyReleaseEvent(e);
        e->accept();
#endif
    }

    void paintEvent(QPaintEvent *e)
    {
        GfxPainter p(this, e);
        p.fillRect(rect(), Qt::black);
        canvas.paint(p);
    }


private:
    GfxCanvas canvas;
};
#endif

#include <QtopiaApplication>
int main(int argc, char ** argv)
{
#ifdef TEST
    QtopiaApplication app(argc, argv);
    if(argc == 2)
        use_int = true;
    MyWidget2 mw;
    mw.showMaximized();
    return app.exec();
#else
    if(!QString(getenv("GFX_USE_MULTITHREADED")).isEmpty())
        runThreaded = true;

    GfxImageLoader::start();
    GfxImageLoader::setCacheSize(10000000);
    if(runThreaded) {

        img = QImage(240, 320, QImage::Format_RGB16);
        QtopiaApplication app(argc, argv);
        fb = QDirectPainter::frameBuffer();

        GfxTimeLine::lock();
        MyThread mt;
        setClockCallback(&repaintCanvas);
        mt.start();

        GfxTimeLine::lock()->lock();
        MyWidget w;
        GfxTimeLine::lock()->unlock();
        w.showMaximized();
        return app.exec();
    
    } else {

        QtopiaApplication app(argc, argv);

        GfxTimeLine::lock();
        GfxTimeLine::lock()->lock();
        MyWidget w;
        GfxTimeLine::lock()->unlock();
        w.showMaximized();
        return app.exec();
    }
#endif
}

#include "main.moc"
