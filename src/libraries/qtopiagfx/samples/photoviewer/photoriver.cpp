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

#include "photoriver.h"
#include <QDir>
#include <GfxPainter>
#include <QMutex>
#include "gfximageloader.h"
#include "imagecollection.h"
#include "softkeybar.h"

#define BORDER_THICKNESS 9

class Photo : public GfxCanvasItem, public GfxImageLoader
{
public:
    Photo(const QString &str, int row, PhotoRiver *river, GfxCanvasItem *parent)
    : GfxCanvasItem(parent), pr(river), imaged(false), _row(row), _r(-100000), 
      deployed(false), mipimg(0), ccolor(0)
    {
        int width = qrand() % 40;
        color = QColor(0, 0, 0, (90 * (40 - width)) / 40);
        z().setValue(width);
        width += 55;
        size = QSize(width, (3 * width) / 4);

        int span_size =  20 + (280 / 4);
        int top = row * 280 / 4 - 10;
        int bottom = top + span_size;
        top += size.height() / 2;
        bottom -= size.height() / 2;

        center = top + qrand() % (bottom - top);

        x().setValue(-width - 1);
        y().setValue(center);
        visible().setValue(0.);

        loadImage(str, QList<QSize>() << QSize(320, 240));

        int t = 25000 + qrand() % 20000;
        velocity = 400. / qreal(t);

        mipimg = new GfxCanvasMipImage(MAXIMIZED_SIZE, this);
        mipimg->scale().setValue(qreal(size.width()) / qreal(MAXIMIZED_SIZE.width()));
        ccolor = new GfxCanvasColor(color, MAXIMIZED_SIZE + QSize(12, 12), mipimg);
    }

    int row() const
    {
        return _row;
    }

    GfxEvent completeEvent()
    {
        return GfxEvent(this, this, &Photo::complete);
    }

    virtual void images(const QList<QImage> &imgs)
    {
        GfxTimeLine::lock()->lock();
        for(int ii = 0; ii < imgs.count(); ++ii)
            mipimg->addImage(imgs.at(ii));
        imaged = true;
        if(imaged && deployed)
            go();
        GfxTimeLine::lock()->unlock();
    }

    void filter(QList<QImage> &imgs)
    {
        QImage fullscreen = imgs.first();

        QImage maximized = QImage(MAXIMIZED_SIZE, QImage::Format_RGB16);
        maximized.fill(0);
        QPainter p(&maximized);
        p.setPen(Qt::NoPen);
        p.fillRect(0, 0, maximized.width(), BORDER_THICKNESS, Qt::white);
        p.fillRect(0, 0, BORDER_THICKNESS, maximized.height(), Qt::white);
        p.fillRect(maximized.width() - BORDER_THICKNESS - 1, 0, BORDER_THICKNESS + 1, maximized.height(), Qt::white);
        p.fillRect(0, maximized.height() - BORDER_THICKNESS - 1, maximized.width(), BORDER_THICKNESS + 1, Qt::white);
        p.drawRect(0, 0, maximized.width(), maximized.height());
        p.drawImage(BORDER_THICKNESS, BORDER_THICKNESS, fullscreen);
        QImage small = maximized.scaled(SMALL_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);
        QImage zoomed = maximized.scaled(ZOOMED_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);
        QImage floating = maximized.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);

        imgs.clear();
        imgs << maximized << small << zoomed << floating;
    }

    void deploy()
    {
        if(deployed)
            return;
        deployed = true;
        if(imaged && deployed)
            go();
    }

private:
    void complete()
    {
        pr->complete(this);
    }

    void go()
    {
        pr->go(this);
    }

public:
    bool imaged;
    bool deployed;
    PhotoRiver *pr;
    QColor color;
    int _row;
    qreal _r;
    QSize size;
    qreal velocity;
    GfxCanvasMipImage *mipimg;
    GfxCanvasColor *ccolor;
    int center;
    QPoint savePoint;
};

class PhotoListItem : public GfxCanvasListItem
{
public:
    PhotoListItem(GfxCanvasItem *parent)
        : GfxCanvasListItem(parent), photo(0)
    {
    }

    virtual QSize focusSize() const
    {
        return ZOOMED_SIZE;
    }

    virtual QSize size() const
    {
        return SMALL_SIZE;
    }

    virtual void focusIn()
    {
        if(photo) {
            tl.clear();
            tl.pause(raised(), 150);
            tl.sync();
            tl.set(raised(), 1.);
            tl.move(photo->mipimg->scale(), ZOOMED_ZOOM, 150);
        }
    }

    virtual void focusOut()
    {
        if(photo) {
            tl.clear();
            tl.move(photo->mipimg->scale(), SMALL_ZOOM, 150);
            tl.sync();
            tl.set(raised(), 0.);
        }
    }

    GfxTimeLine tl;
    Photo *photo;
};

void PhotoRiver::go(Photo *p)
{
    if(catched.count())
        return;

    p->visible().setValue(1.);

    int time = (300. - p->x().value()) / p->velocity;
    tl.move(p->x(), 300, time);
    tl.pause(*p, time);
    tl.execute(p->completeEvent());
    tl.execute(p->destroyEvent());
    p->mipimg->setSubPixelPlacement(true);

    int pa = time / 3;
    int row = p->row();
    tls[row].clear();
    tls[row].pause(objs[row], pa);
    switch(row) {
        case 0:
            tls[row].execute(GfxEvent(&objs[row], this, &PhotoRiver::go1));
            break;
        case 1:
            tls[row].execute(GfxEvent(&objs[row], this, &PhotoRiver::go2));
            break;
        case 2:
            tls[row].execute(GfxEvent(&objs[row], this, &PhotoRiver::go3));
            break;
        case 3:
            tls[row].execute(GfxEvent(&objs[row], this, &PhotoRiver::go4));
            break;
    }
}

PhotoRiver::PhotoRiver(GfxCanvasItem *parent)
: GfxCanvasWindow(QSize(240, 320), parent), clip(this), list(this),
    viewing(false), viewItem(this)
{
    clip.width().setValue(240);
    clip.height().setValue(280);

    QDir dir("collections/");
    foreach(QString collection, 
            dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir dfiles("collections/" + collection, "*.png");
        foreach(QString file, dfiles.entryList())
            files << "collections/" + collection + "/" + file;
        dfiles = QDir("collections/" + collection, "*.jpg");
        foreach(QString file, dfiles.entryList())
            files << "collections/" + collection + "/" + file;
    }

    for(int ii = 0; ii < 4; ++ii) {
        Photo *photo = 
            new Photo(files.at(qrand() % files.count()), ii, this, &clip);
        photos[ii] << photo;
        photo->deploy();
    }

    for(int ii = 0; ii < 16; ++ii) {
        Photo *photo = 
            new Photo(files.at(qrand() % files.count()), ii % 4, this, &clip);
        photos[ii % 4] << photo;
    }

    ref = new GfxCanvasReflection(QSize(240, 40), 0, -41, this);
    canvas()->addDynamicItem(ref);
    ref->y().setValue(281);
    ref->z().setValue(100);


    date = new GfxCanvasText(QSize(240, 20), this);
    QFont f;
    f.setPointSize(14);
    date->setFont(f);
    date->setAlignmentFlags(Qt::AlignRight);
    date->y().setValue(250);
    date->x().setValue(110);
    date->z().setValue(99);

    time = new GfxCanvasText(QSize(240, 40), this);
    f.setPointSize(28);
    time->setFont(f);
    time->setAlignmentFlags(Qt::AlignRight);
    time->y().setValue(270);
    time->x().setValue(110);
    time->z().setValue(99);

    startTimer(10000);


    SoftKeyBar::setLabel(this, SoftKeyBar::Left, "Options");
    SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "Catch");

    list.setSpacing(0, 5);
    list.y().setValue(20);
    list.width().setValue(240);
    list.height().setValue(280);
    QObject::connect(&list, SIGNAL(activated(GfxCanvasListItem*)), 
                     this, SLOT(itemActivated()));

    viewItem.z().setValue(10001);
}

PhotoRiver::~PhotoRiver()
{
    canvas()->remDynamicItem(ref);
}

void PhotoRiver::timerEvent(QTimerEvent *)
{
    GfxTimeLine::lock()->lock();
    QDateTime dt = QDateTime::currentDateTime();
    if(dt != datetime) {
        datetime = dt;
        time->setText(datetime.time().toString("H:mm"));
        date->setText(datetime.date().toString("ddd d MMM"));
    }
    GfxTimeLine::lock()->unlock();
}

void PhotoRiver::keyReleaseEvent(QKeyEvent *e)
{
    if(viewing) {
        e->accept();
        PhotoListItem *item = static_cast<PhotoListItem *>(list.currentItem());
        tl.reset(item->photo->x());
        tl.reset(item->photo->y());
        tl.reset(item->photo->mipimg->rotate());
        tl.reset(item->photo->mipimg->scale());

        item->photo->moveToParent(item);
        tl.move(item->photo->x(), 0., 150);
        tl.move(item->photo->y(), 0., 150);
        tl.move(item->photo->mipimg->scale(), ZOOMED_ZOOM, 150);
        tl.move(item->photo->mipimg->rotate(), 0., 150);
        viewing = false;
        list.setFocused(true);
        SoftKeyBar::setLabel(this, SoftKeyBar::Left, "");
        SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "View");
        SoftKeyBar::setLabel(this, SoftKeyBar::Right, "Release");
    } else if(e->key() == Qt::Key_Select && catched.isEmpty()) {
        SoftKeyBar::setLabel(this, SoftKeyBar::Left, "");
        SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "View");
        SoftKeyBar::setLabel(this, SoftKeyBar::Right, "Release");
        e->accept();
        tl.clear();
        for(int ii = 0; ii < 4; ++ii)
            tls[ii].clear();

        QList<GfxCanvasListItem *> photos;
        QList<GfxCanvasListItem *> olditems = list.items();
        for(int ii = 0; ii < 12; ++ii) {
            GfxCanvasListItem *item = new PhotoListItem(this);
            photos << item;
        }
        list.setItems(photos);
        list.setCurrent(0);
        qDeleteAll(olditems);

        catched.clear();
        for(int ii = 0; ii < 4; ++ii) {
            for(int jj = 2; jj >= 0; --jj) {
                PhotoListItem *item = static_cast<PhotoListItem *>(photos.at(ii * 3 + (2 - jj)));
                Photo *p = this->photos[ii][jj];
                item->photo = p;
                p->savePoint = QPoint(int(p->x().value()), int(p->y().value()));
                p->deployed = true;
                p->moveToParent(item);
                p->visible().setValue(1.);
                tl.move(p->x(), 0., 150);
                tl.move(p->y(), 0., 150);
                catched << p;
                tl.move(p->ccolor->visible(), 0., 150);
                p->mipimg->setSubPixelPlacement(false);
                if(catched.count() == 1) {
                    tl.move(p->mipimg->scale(), ZOOMED_ZOOM, 150);
                } else {
                    tl.move(p->mipimg->scale(), SMALL_ZOOM, 150);
                }
            }
        }
        for(int ii = 0; ii < 4; ++ii) {
            for(int jj = 3; jj < this->photos[ii].count(); ++jj) {
                Photo *p = this->photos[ii][jj];
                if(p->deployed) {
                    p->deployed = false;
                    tl.move(p->x(), -p->size.width() - 1, 150);
                    tl.pause(p->visible(), 150);
                    tl.set(p->visible(), 0);
                }
            }
        }
        list.setFocused(true);

    } else if(e->key() == Qt::Key_Back && !catched.isEmpty()) {
        QList<Photo *> catched = this->catched;
        this->catched.clear();
        for(int ii = 0; ii < catched.count(); ++ii) {
            Photo *p = catched.at(ii);
            p->moveToParent(this);
            tl.move(p->ccolor->visible(), 1., 150);
            tl.move(p->mipimg->scale(), qreal(p->size.width()) / qreal(MAXIMIZED_SIZE.width()), 150);
            tl.move(p->x(), p->savePoint.x(), 150);
            tl.move(p->y(), p->savePoint.y(), 150);
            go(p);
        }
        SoftKeyBar::setLabel(this, SoftKeyBar::Left, "Options");
        SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "Catch");
        SoftKeyBar::setLabel(this, SoftKeyBar::Right, QString());

        setFocused(true);
        e->accept();
    }
}

void PhotoRiver::itemActivated()
{
    PhotoListItem *item = static_cast<PhotoListItem *>(list.currentItem());

    item->photo->moveToParent(&viewItem);
    tl.reset(item->photo->x());
    tl.reset(item->photo->y());
    tl.reset(item->photo->mipimg->rotate());
    tl.reset(item->photo->mipimg->scale());

    tl.move(item->photo->mipimg->scale(), ZOOMED_ZOOM * .7, 150);
    tl.move(item->photo->mipimg->scale(), ZOOMED_ZOOM, 150);
    tl.pause(item->photo->x(), 300);
    tl.pause(item->photo->y(), 300);
    tl.pause(item->photo->mipimg->rotate(), 300);
    tl.move(item->photo->x(), 119., 150);
    tl.move(item->photo->y(), 160., 150);
    tl.move(item->photo->mipimg->scale(), 1., 150);
    tl.move(item->photo->mipimg->rotate(), 90., 150);
    viewing = true;
    setFocused(true);
    SoftKeyBar::setLabel(this, SoftKeyBar::Left, "");
    SoftKeyBar::setLabel(this, SoftKeyBar::Middle, "List");
    SoftKeyBar::setLabel(this, SoftKeyBar::Right, "");
}

void PhotoRiver::complete(Photo *p)
{
    Photo *photo = new Photo(files.at(qrand() % files.count()), p->row(), this, &clip);
    photos[p->row()].removeAll(p);
    photos[p->row()] << photo;
}

void PhotoRiver::go1()
{
    int row = 0;
    for(int ii = 0; ii < photos[row].count(); ++ii) {
        if(!photos[row][ii]->deployed) {
            photos[row][ii]->deploy();
            break;
        }
    }
}

void PhotoRiver::go2()
{
    int row = 1;
    for(int ii = 0; ii < photos[row].count(); ++ii) {
        if(!photos[row][ii]->deployed) {
            photos[row][ii]->deploy();
            break;
        }
    }
}

void PhotoRiver::go3()
{
    int row = 2;
    for(int ii = 0; ii < photos[row].count(); ++ii) {
        if(!photos[row][ii]->deployed) {
            photos[row][ii]->deploy();
            break;
        }
    }
}

void PhotoRiver::go4()
{
    int row = 3;
    for(int ii = 0; ii < photos[row].count(); ++ii) {
        if(!photos[row][ii]->deployed) {
            photos[row][ii]->deploy();
            break;
        }
    }
}

