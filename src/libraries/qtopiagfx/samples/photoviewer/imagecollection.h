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

#ifndef IMAGECOLLECTION_H
#define IMAGECOLLECTION_H

#include "gfxcanvas.h"
#include "gfxcanvaslist.h"
#include "gfximageloader.h"

#define BORDER_THICKNESS 9
#define SMALL_SIZE QSize(67, 51)
#define ZOOMED_SIZE QSize(98, 75)
#define TIMEVIEW_SIZE QSize(220, 167)
#define MAXIMIZED_SIZE QSize(320 + 2 * BORDER_THICKNESS, 240 + 2 * BORDER_THICKNESS)
#define SMALL_ZOOM (qreal(67) / qreal(320 + 2 * BORDER_THICKNESS))
#define ZOOMED_ZOOM (qreal(98) / qreal(320 + 2 * BORDER_THICKNESS))
#define TIMEVIEW_ZOOM (qreal(220) / qreal(320 + 2 * BORDER_THICKNESS))

class Image;
class ImageCollection : public GfxCanvasItem
{
public:
    ImageCollection(GfxCanvasItem *defaultParent,
                    const QString &name,
                    const QStringList &imgs,
                    bool showDates = true);


    GfxCanvasList *listItem();
    GfxCanvasItem *iconItem();
    GfxCanvasItem *viewingItem();
    GfxCanvasItem *timeItem();

    QRect listRect() const;
    void setListRect(const QRect &);

    QString name() const;
    int count() const;
    bool isImageCreated(int ii) const;
    Image *image(int ii); 

    enum State { Collapsed, Expanded, View, Time, Hidden };
    State state() const;
    void setState(State);

    int focused() const;
    void setFocused(int);

    bool isOnScreen(int ii);

    GfxCanvasItem *collapseParent();
    GfxCanvasItem *subTimeItem();

private:
    QPoint collapsePoint() const;

    GfxCanvasList _listItem;

    GfxCanvasItem _iconItem;
    GfxCanvasItem _viewingItem;
    GfxCanvasItem _timeItem;
    GfxCanvasItem _subTimeItem;

    void layoutView();
    void layoutExpandedFromView();
    void layoutExpandedFromTime();
    void layoutImmExpanded();
    void layoutExpanded();
    void layoutImmCollapsed();
    void layoutCollapsed();
    void layoutImmTime();
    void layoutTime();

    QVector<Image *> _images;
    QStringList _imageFiles;
    QString _name;
    State _state;

    GfxTimeLine tl;

    GfxCanvasCacheLayer _collapseLayer;

    GfxCanvasItem *nameItem(bool create = true);
    QImage &nameImg();
    GfxCanvasItem *_nameItem;
    QImage _nameImg;

    QRect _listRect;
};

class Image : public GfxCanvasMipImage, public GfxImageLoader, public GfxValueSink
{ 
public: 
    Image(const QString &, int idx, ImageCollection *w);

    const QImage &maximized() const;
    const QImage &small() const;
    const QImage &zoomed() const;

    void setPos(const QPoint &p);
    QPoint origPos() const;
    QPoint pos() const;

    virtual void images(const QList<QImage> &imgs);
    virtual void filter(QList<QImage> &imgs);

    virtual void paint(GfxPainter &p);

    GfxValue &imageParent();

    void setListItem(GfxCanvasListItem *);
    GfxCanvasListItem *listItem() const;

protected:
    virtual void valueChanged(GfxValue *, qreal old, qreal newValue);

private:
    GfxCanvasListItem *_listItem;
    QImage _timeView;
    QImage _maximized;
    QImage _small;
    QImage _zoomed;
    QPoint _pos;

    QRect ur;

    static QImage _defTimeView;
    static QImage _defMaximized;
    static QImage _defSmall;
    static QImage _defZoomed;

    GfxValue _def;

    GfxTimeLine tl;
    GfxCanvasColor _color;

    int _idx;
    bool _loaded;
    QString _str;

    ImageCollection *_collection;

    GfxValueNotifying _imageParent;
};

#endif
