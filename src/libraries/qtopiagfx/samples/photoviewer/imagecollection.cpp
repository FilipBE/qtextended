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

#include "imagecollection.h"
#include <GfxPainter>
#include <QFileInfo>
#include <QMutex>
#include "softkeybar.h"

class ImageListItem : public GfxCanvasListItem
{
public:
    ImageListItem(Image *i, int idx, GfxCanvasItem *);
    ImageListItem(Image *i, int idx, GfxCanvasListItemGroup *);
    virtual QSize size() const;
    virtual QSize focusSize() const;

    virtual void focusIn();
    virtual void focusOut();

    Image *image() const;
    void setImage(Image *);

    int idx() const;
private:
    GfxTimeLine tl;
    Image *_image;
    int _idx;
};

ImageListItem::ImageListItem(Image *i, int idx, GfxCanvasListItemGroup *p)
: GfxCanvasListItem(p), _image(i), _idx(idx)
{
    _image->setParent(this);
}

ImageListItem::ImageListItem(Image *i, int idx, GfxCanvasItem *p)
: GfxCanvasListItem(p), _image(i), _idx(idx)
{
    _image->setParent(this);
}

QSize ImageListItem::size() const
{
    return SMALL_SIZE;
}

QSize ImageListItem::focusSize() const
{
    return ZOOMED_SIZE;
}

Image *ImageListItem::image() const
{
    return _image;
}

void ImageListItem::setImage(Image *img)
{
    _image = img;
}

int ImageListItem::idx() const
{
    return _idx;
}

void ImageListItem::focusIn()
{
    if(!_image)
        return;
    tl.reset(_image->scale());
    tl.pause(_image->scale(), 150);
    tl.set(raised(), 1.);
    tl.move(_image->scale(), ZOOMED_ZOOM, 150);
}

void ImageListItem::focusOut()
{
    if(!_image)
        return;
    tl.reset(_image->scale());
    tl.move(_image->scale(), SMALL_ZOOM, 150);
    tl.sync(raised());
    tl.set(raised(), 0.);
}

ImageCollection::ImageCollection(GfxCanvasItem *defaultParent,
                                 const QString &name,
                                 const QStringList &imgs,
                                 bool showDates)
: GfxCanvasItem(defaultParent), _listItem(defaultParent),
    _iconItem(defaultParent), _timeItem(defaultParent), 
    _subTimeItem(&_timeItem),
    _viewingItem(defaultParent), _imageFiles(imgs),
    _name(name), _state(Hidden), 
    _collapseLayer(QSize(70, 70), &_iconItem),
    _nameItem(0)
{
    _collapseLayer.x().setValue(-35);
    _collapseLayer.y().setValue(-35);
    _collapseLayer.z().setValue(-10000.0);

    _images.resize(imgs.count());

    QMap<QDate, QList<int> > sorted;
    for(int ii = 0; showDates && ii < imgs.count(); ++ii) {
        QFileInfo fi(imgs.at(ii));
        sorted[fi.lastModified().date()].append(ii);
    }
    QList<GfxCanvasListItem *> items;
    if(sorted.count() > 1 && showDates) {
        for(QMap<QDate, QList<int> >::Iterator iter = sorted.begin();
            iter != sorted.end(); ++iter) {

            GfxCanvasListStringGroup *grp = new GfxCanvasListStringGroup(iter.key().toString(), this);
            items << grp;
            for(int ii = 0; ii < iter->count(); ++ii) {
                Image *i = image(iter->at(ii));
                GfxCanvasListItem *li = new ImageListItem(i, iter->at(ii), grp);
                i->setListItem(li);
            }
        }
    } else {
        for(int ii = 0; ii < imgs.count(); ++ii)  { 
            Image *i = image(ii);
            GfxCanvasListItem *li = new ImageListItem(i, ii, this);
            i->setListItem(li);
            items << li;
        }
    }
    _listItem.height().setValue(260.);
    _listItem.width().setValue(240.);
    _listItem.setSpacing(0, 5);
    _listItem.setItems(items);

    SoftKeyBar::setLabel(&_listItem, SoftKeyBar::Middle, "View");
    SoftKeyBar::setLabel(&_listItem, SoftKeyBar::Right, "Back");
    SoftKeyBar::setLabel(&_viewingItem, SoftKeyBar::Left, "");
    SoftKeyBar::setLabel(&_viewingItem, SoftKeyBar::Middle, "List");
    SoftKeyBar::setLabel(&_viewingItem, SoftKeyBar::Right, "");
}

GfxCanvasItem *ImageCollection::timeItem()
{
    return &_timeItem;
}

GfxCanvasItem *ImageCollection::subTimeItem()
{
    return &_subTimeItem;
}

GfxCanvasList *ImageCollection::listItem()
{
    return &_listItem;
}

GfxCanvasItem *ImageCollection::iconItem()
{
    return &_iconItem;
}

GfxCanvasItem *ImageCollection::viewingItem()
{
    return &_viewingItem;
}

int ImageCollection::focused() const
{
    ImageListItem *i = static_cast<ImageListItem *>(_listItem.currentItem());
    if(!i)
        return -1;
    else
        return i->idx();
}

GfxCanvasItem *ImageCollection::collapseParent() 
{
    return &_collapseLayer;
}

bool ImageCollection::isOnScreen(int ii)
{
    return image(ii)->listItem()->isOnScreen();
}

void ImageCollection::setFocused(int f)
{
    _listItem.setCurrent(f);
}

QPoint ImageCollection::collapsePoint() const
{
    return QPoint(0, 0);
}

ImageCollection::State ImageCollection::state() const
{
    return _state;
}

void ImageCollection::setState(State state)
{
    if(_state == state && state != Hidden)
        return;

    if(state == Hidden) {
        listItem()->visible().setValue(0.);
        iconItem()->visible().setValue(0.);
        viewingItem()->visible().setValue(0.);
        timeItem()->visible().setValue(0.);
    } else if(state == Time) {
        timeItem()->visible().setValue(1.);
        if(_state == Expanded) {
            layoutTime();
        } else {
            layoutImmTime();
        }
    } else if(_state == Hidden) {
        Q_ASSERT(state == Collapsed || state == Expanded);
        if(state == Expanded) {
            listItem()->setActive(true);
            listItem()->visible().setValue(1.);
            layoutImmExpanded();
        } else if(state == Collapsed) {
            iconItem()->visible().setValue(1.);
            layoutImmCollapsed();
            listItem()->setActive(false);
        } 
    } else if(state == Expanded) {
        Q_ASSERT(_state == Collapsed || _state == View || _state == Time);
        listItem()->setActive(true);
        listItem()->setFocused(true);
        listItem()->visible().setValue(1.);
        if(_state == Time) {
            layoutExpandedFromTime();
        } else if(_state == View) {
            layoutExpandedFromView();
        } else {
            layoutExpanded();
        }
    } else if(state == Collapsed) {
        Q_ASSERT(_state == Expanded);
        layoutCollapsed();
        listItem()->setActive(false);
    } else if(state == View) {
        Q_ASSERT(_state == Expanded);
        layoutView();
    }

    _state = state;
}

void ImageCollection::layoutExpandedFromView()
{
    tl.complete();
    ImageListItem *i = static_cast<ImageListItem *>(_listItem.currentItem());
    Image *img = image(i->idx());

    img->moveToParent(i);
    tl.move(img->x(), 0, 150);
    tl.move(img->y(), 0, 150);
    tl.move(img->scale(), ZOOMED_ZOOM, 150);
    tl.move(img->rotate(), 0., 150);
}

void ImageCollection::layoutView()
{
    tl.complete();

    ImageListItem *ili = static_cast<ImageListItem *>(listItem()->currentItem());
    if(!ili) return;
    Image *img = ili->image();
    if(!img) return;
    img->moveToParent(viewingItem());

    tl.move(img->scale(), ZOOMED_ZOOM * .7, 150);
    tl.move(img->scale(), ZOOMED_ZOOM, 150);
    tl.sync();
    tl.move(img->x(), 0, 150);
    tl.move(img->y(), 0, 150);
    tl.move(img->scale(), 1., 150);
    tl.move(img->rotate(), 90., 150);
    viewingItem()->setFocused(true);
}

void ImageCollection::layoutImmExpanded()
{
    tl.complete();

    GfxCanvasItem *i = nameItem(false);
    if(i) i->visible().setValue(0.);
    
    int f = focused();
    for(int ii = 0; ii < count(); ++ii) {
        Image *img = image(ii);
        img->imageParent().setValue(0.);
        img->x().setValue(0);
        img->y().setValue(0);
        img->visible().setValue(1.);

        if(f == ii)
            img->scale().setValue(ZOOMED_ZOOM);
        else
            img->scale().setValue(SMALL_ZOOM);

        img->rotate().setValue(0.);
    }
}

void ImageCollection::layoutExpandedFromTime()
{
    tl.complete();
    int f = focused();
    int pauses = 0;
    for(int ii = 0; ii < count(); ++ii) {
        Image *img = image(ii);

        if(!isOnScreen(ii)) {
            img->imageParent().setValue(0.);
            img->visible().setValue(1.);
            img->quality().setValue(0.);
            img->x().setValue(0);
            img->y().setValue(0);
            img->scale().setValue( SMALL_ZOOM );
            img->rotate().setValue(0);
        } else {
            int pauseTime = pauses * 50;
            pauses++;
            if(ii)
                img->setParent(image(0));
            img->z().setValue(-ii);
            tl.pause(img->visible(), pauseTime);
            tl.pause(img->x(), pauseTime);
            tl.pause(img->y(), pauseTime);
            tl.pause(img->rotate(), pauseTime);
            tl.pause(img->scale(), pauseTime);
            tl.pause(img->quality(), pauseTime);
            tl.pause(img->imageParent(), pauseTime);
            tl.set(img->imageParent(), 0.);
            tl.set(img->visible(), 1.);
            tl.set(img->quality(), 0.);
            tl.move(img->x(), 0, 150);
            tl.move(img->y(), 0, 150);

            if(f == ii) {
                tl.move(img->scale(), SMALL_ZOOM, 150);
                tl.sync(img->z(), img->scale());
                tl.set(img->z(), 1.);
                tl.move(img->scale(), ZOOMED_ZOOM, 50);
            } else {
                tl.move(img->scale(), SMALL_ZOOM, 150);
            }
            tl.move(img->rotate(), 0, 150);
        }
    }
}

void ImageCollection::layoutExpanded()
{
    tl.complete();

    _collapseLayer.moveToParent(&_listItem);
    _collapseLayer.z().setValue(1.);
    GfxCanvasItem *i = nameItem(false);
    if(i) i->visible().setValue(0.);
    int pauses = 0;

    int f = focused();
    for(int ii = count() - 1; ii >= 0; --ii) {

        Image *img = image(ii);

        if(!isOnScreen(ii)) {
            img->imageParent().setValue(0.);
            img->visible().setValue(1.);
            img->quality().setValue(0.);
            img->x().setValue(0);
            img->y().setValue(0);
            img->scale().setValue( SMALL_ZOOM );
            img->rotate().setValue(0);
        } else {
            int pauseTime = pauses * 50 + 150;
            pauses++;
            tl.pause(img->visible(), pauseTime);
            tl.pause(img->x(), pauseTime);
            tl.pause(img->y(), pauseTime);
            tl.pause(img->rotate(), pauseTime);
            tl.pause(img->scale(), pauseTime);
            tl.pause(img->quality(), pauseTime);
            tl.pause(img->imageParent(), pauseTime);
            tl.set(img->imageParent(), 0.);
            tl.set(img->visible(), 1.);
            tl.set(img->quality(), 0.);
            tl.move(img->x(), 0, 150);
            tl.move(img->y(), 0, 150);

            if(f == ii) {
                tl.move(img->scale(), SMALL_ZOOM, 150);
                tl.sync(img->z(), img->scale());
                tl.set(img->z(), 1.);
                tl.move(img->scale(), ZOOMED_ZOOM, 50);
            } else {
                tl.move(img->scale(), SMALL_ZOOM, 150);
            }
            tl.move(img->rotate(), 0, 150);
        }
    } 
}

void ImageCollection::layoutCollapsed()
{
    tl.complete();

    _collapseLayer.moveToParent(&_iconItem);

    QPoint p = collapsePoint();

    int seenOnScreen = 0;
    int f = focused();

    _collapseLayer.z().setValue(-10000.);
    for(int ii = count() - 1; ii >= 0; --ii) {

        Image *img = image(ii);
        img->moveToParent(iconItem());

        if(!isOnScreen(ii))
            continue;

        seenOnScreen++;

        int pauseTime = (seenOnScreen - 1) * 50;
        tl.pause(img->x(), pauseTime);
        tl.pause(img->y(), pauseTime);
        tl.pause(img->rotate(), pauseTime);
        img->z().setValue(-ii);

        if(ii == f) {
            tl.move(img->scale(), SMALL_ZOOM, 50);
            tl.sync(img->z(), img->scale());
            img->z().setValue(1);
            tl.set(img->z(), -ii);

            if(pauseTime > 50)
                tl.pause(img->scale(), pauseTime - 50);
        } else {
            tl.pause(img->scale(), pauseTime);
        }
    }
    seenOnScreen = 0;
    for(int ii = 0; ii < count(); ++ii) {
        if(!isOnScreen(ii))
            continue;
        seenOnScreen++;

        Image *img = image(ii);

#define PHOTO_OFFSET 7
        int idx = seenOnScreen;
        switch(idx) {
            case 0:
                tl.move(img->x(), p.x() - PHOTO_OFFSET, 150);
                tl.move(img->y(), p.y() - PHOTO_OFFSET, 150);
                tl.sync(img->quality(), img->x());
                tl.set(img->quality(), 1.);
                break;
            case 1:
                tl.move(img->x(), p.x() + PHOTO_OFFSET, 150);
                tl.move(img->y(), p.y() - PHOTO_OFFSET, 150);
                tl.sync(img->quality(), img->x());
                tl.set(img->quality(), 1.);
                break;
            case 2:
                tl.move(img->x(), p.x() + PHOTO_OFFSET, 150);
                tl.move(img->y(), p.y() + PHOTO_OFFSET, 150);
                tl.sync(img->quality(), img->x());
                tl.set(img->quality(), 1.);
                break;
            case 3:
                tl.move(img->x(), p.x() - PHOTO_OFFSET, 150);
                tl.move(img->y(), p.y() + PHOTO_OFFSET, 150);
                tl.sync(img->quality(), img->x());
                tl.set(img->quality(), 1.);
                break;
            default:
                tl.move(img->x(), p.x(), 150);
                tl.move(img->y(), p.y(), 150);
                //tl.set(img->quality(), 0.);
                break;
        }
        int rotate = (qrand() % 40) - 20;
        if(isOnScreen(ii)) {
            tl.move(img->scale(), SMALL_ZOOM * .7, 150);
            tl.move(img->rotate(), rotate, 150);
            tl.sync(image(ii)->imageParent(), image(ii)->x());
            tl.set(image(ii)->imageParent(), 1.);
        } else {
            img->scale().setValue(SMALL_ZOOM * .7);
            img->rotate().setValue(rotate);
            img->visible().setValue(0.);
            img->imageParent().setValue(1.);
        }
    } 
    tl.sync();
    seenOnScreen = 0;
    for(int ii = 0; ii < count(); ++ii) {
        if(!isOnScreen(ii))
            continue;
        seenOnScreen++;

        if(seenOnScreen > 4) 
            tl.set(image(ii)->visible(), 0.);
    }
    GfxCanvasItem *i = nameItem();
    tl.move(i->visible(), 1., 300);
}

void ImageCollection::layoutTime()
{
    tl.complete();

    // Collapse focus
    if(focused() != -1) {
        Image *foc = image(focused());
        tl.move(foc->scale(), SMALL_ZOOM, 50);
    }
    tl.sync();
    int seen = 0;
    for(int ii = 0; ii < count(); ++ii) {
        Image *img = image(ii);

        if(!isOnScreen(ii)) {
            img->imageParent().setValue(2.);
            img->visible().setValue(0.);
            img->scale().setValue(SMALL_ZOOM);
        } else {
            if(!seen) {
                QPoint p = img->mapTo(&_timeItem, QPoint(0, 0));
                _subTimeItem.x().setValue(p.x());
                _subTimeItem.y().setValue(p.y());
                tl.move(img->scale(), TIMEVIEW_ZOOM, 200);
                tl.move(_subTimeItem.x(), 0, 200);
                tl.move(_subTimeItem.y(), 0, 200);
            } 

            tl.pause(img->x(), seen * 20);
            tl.pause(img->y(), seen * 20);
            tl.pause(img->imageParent(), seen * 20);
            tl.set(img->imageParent(), 2.);
            tl.move(img->x(), 0, 150);
            tl.move(img->y(), 0, 150);
            if(seen) {
                tl.sync(img->visible(), img->y());
                tl.set(img->visible(), 0.);
            }

            seen++;
        }
    }
    GfxCanvasItem *i = nameItem();
    i->visible().setValue(0.);
}

void ImageCollection::layoutImmTime()
{
    tl.complete();

    for(int ii = 0; ii < count(); ++ii) {

        Image *img = image(ii);
        img->imageParent().setValue(2.);

        img->x().setValue(0);
        img->y().setValue(0);
        img->rotate().setValue(0);

        if(ii) {
            img->visible().setValue(0.);
            img->scale().setValue(SMALL_ZOOM);
        } else {
            img->visible().setValue(1.);
            img->scale().setValue(TIMEVIEW_ZOOM);
        }
    } 
    GfxCanvasItem *i = nameItem();
    i->visible().setValue(0.);
}

void ImageCollection::layoutImmCollapsed()
{
    tl.complete();

    _collapseLayer.moveToParent(&_iconItem);
    QPoint p = collapsePoint();

    for(int ii = 0; ii < count(); ++ii) {

        Image *img = image(ii);
        img->imageParent().setValue(0.);
        img->moveToParent(iconItem());

#define PHOTO_OFFSET 7
        switch(ii) {
            case 0:
                img->x().setValue( p.x() - PHOTO_OFFSET);
                img->y().setValue( p.y() - PHOTO_OFFSET);
                img->quality().setValue( 1.);
                img->visible().setValue( 1.);
                break;
            case 1:
                img->x().setValue( p.x() + PHOTO_OFFSET);
                img->y().setValue( p.y() - PHOTO_OFFSET);
                img->quality().setValue( 1.);
                img->visible().setValue( 1.);
                break;
            case 2:
                img->x().setValue( p.x() + PHOTO_OFFSET);
                img->y().setValue( p.y() + PHOTO_OFFSET);
                img->quality().setValue( 1.);
                img->visible().setValue( 1.);
                break;
            case 3:
                img->x().setValue( p.x() - PHOTO_OFFSET);
                img->y().setValue( p.y() + PHOTO_OFFSET);
                img->quality().setValue( 1.);
                img->visible().setValue( 1.);
                break;
            default:
                img->x().setValue( p.x());
                img->y().setValue( p.y());
                img->visible().setValue( 0.);
                break;
        }
        img->imageParent().setValue(1.);
        img->scale().setValue( SMALL_ZOOM * .7);

        int rotate = (qrand() % 40) - 20;
        img->rotate().setValue(rotate);
        img->imageParent().setValue(1.);
    } 
    GfxCanvasItem *i = nameItem();
    i->visible().setValue(1.);
}

QRect ImageCollection::listRect() const
{
    return _listRect;
}

void ImageCollection::setListRect(const QRect &r)
{
    _listRect = r;
}

QString ImageCollection::name() const
{
    return _name;
}

int ImageCollection::count() const
{
    return _images.count();
}

bool ImageCollection::isImageCreated(int ii) const
{
    return 0 != _images.at(ii);
}

Image *ImageCollection::image(int ii)
{
    if(!_images.at(ii)) {
        Image *img = new Image(_imageFiles.at(ii), ii, this);
        img->scale().setValue(SMALL_ZOOM);
        _images[ii] = img;
    }
    return _images.at(ii);
}

GfxCanvasItem *ImageCollection::nameItem(bool create)
{
    if(!_nameItem && create) {
        _nameItem = new GfxCanvasImage(nameImg(), iconItem());
        _nameItem->visible().setValue(35);
        _nameItem->y().setValue(50);
    }
    
    return _nameItem;
}

QImage &ImageCollection::nameImg()
{
    if(_nameImg.isNull()) 
        _nameImg = GfxPainter::string(_name, Qt::white);
    return _nameImg;
}

QImage Image::_defMaximized;
QImage Image::_defTimeView;
QImage Image::_defSmall;
QImage Image::_defZoomed;

void Image::filter(QList<QImage> &imgs)
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
    QImage timeView = maximized.scaled(TIMEVIEW_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);

    imgs.clear();
    imgs << maximized << small << zoomed << timeView;
}

void Image::images(const QList<QImage> &imgs)
{
    GfxTimeLine::lock()->lock();
    _maximized = imgs.at(0);
    _small = imgs.at(1);
    _zoomed = imgs.at(2);
    _timeView = imgs.at(3);

    tl.complete();
    tl.move(_color.visible(), 1., 200);
    tl.sync(_def);
    tl.set(_def, 2.);
    tl.move(_color.visible(), 0., 200);
    GfxTimeLine::lock()->unlock();
}

void Image::valueChanged(GfxValue *, qreal, qreal newValue)
{
    ImageListItem *item = static_cast<ImageListItem *>(_listItem);
    if(newValue == 0.) {
        moveToParent(item);
        item->setImage(this);
    } else if(newValue == 1.) {
        moveToParent(_collection->collapseParent());
        item->setImage(0);
    } else if(newValue == 2.) {
        moveToParent(_collection->subTimeItem());
        item->setImage(0);
    }
}

void Image::paint(GfxPainter &p)
{

    if(!_collection->isOnScreen(_idx))
        return;

    if(!_loaded) {
        loadImage(_str, QList<QSize>() << QSize(320, 240));
        _loaded = true;
    }

    if(_def.value() == 2. && !_maximized.isNull()) {
        clear();
        addImage(_maximized);
        addImage(_small);
        addImage(_zoomed);
        addImage(_timeView);
        _maximized = _small = _zoomed = _timeView = QImage();
    }

    _color.rotate().setValue(rotate().value());
    _color.quality().setValue(quality().value());

    GfxCanvasMipImage::paint(p);
}

GfxValue &Image::imageParent()
{
    return _imageParent;
}

Image::Image(const QString &str, int idx, ImageCollection *w)
: GfxCanvasMipImage(MAXIMIZED_SIZE, w), _listItem(0), _color(Qt::white, MAXIMIZED_SIZE, this), _idx(idx), _loaded(false), _str(str), _collection(w), _imageParent(this)
{
    _color.visible().setValue(0.);
    if(_defMaximized.isNull()) {

        _defMaximized = QImage(MAXIMIZED_SIZE, QImage::Format_RGB16);
        _defMaximized.fill(0xFFFFFFFF);
        QPainter p(&_defMaximized);
        p.fillRect(BORDER_THICKNESS, BORDER_THICKNESS, MAXIMIZED_SIZE.width() - 2 * BORDER_THICKNESS, MAXIMIZED_SIZE.height() - 2 * BORDER_THICKNESS, Qt::black);
        p.setPen(Qt::white);
        p.setBrush(Qt::white);
        QFont f;
        f.setPixelSize(MAXIMIZED_SIZE.height());
        p.setFont(f);
        p.drawText(_defMaximized.rect(), Qt::AlignVCenter | Qt::AlignCenter, "?");

        _defSmall = _defMaximized.scaled(SMALL_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);
        _defZoomed = _defMaximized.scaled(ZOOMED_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);
        _defTimeView = _defMaximized.scaled(TIMEVIEW_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16);
 
    }

    _def.setValue(1.);
    addImage(_defMaximized);
    addImage(_defTimeView);
    addImage(_defSmall);
    addImage(_defZoomed);
}

GfxCanvasListItem *Image::listItem() const
{
    return _listItem;
}

void Image::setListItem(GfxCanvasListItem *li)
{
    _listItem = li;
}

const QImage &Image::maximized() const
{
    if(_def.value() == 0.)
        return _maximized;
    else
        return _defMaximized;
}

const QImage &Image::small() const
{
    if(_def.value() == 0.)
        return _small;
    else
        return _defSmall;
}

const QImage &Image::zoomed() const
{
    if(_def.value() == 0.)
        return _zoomed;
    else
        return _defZoomed;
}

void Image::setPos(const QPoint &p)
{
    _pos = p;
    x().setValue(p.x());
    y().setValue(p.y());
}


QPoint Image::origPos() const
{
    return _pos;
}

QPoint Image::pos() const
{
    return QPoint(int(x().value()), int(y().value()));
}

