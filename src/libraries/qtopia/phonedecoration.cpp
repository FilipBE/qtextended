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

#include "phonedecoration_p.h"
#include "qtopiaapplication.h"
#include "qexportedbackground.h"
#include <custom.h>

#include <QSettings>
#include <QPainter>
#include <QPaintEngine>
#include <QBitmap>
#include <QPixmapCache>
#include <QDebug>
#include <QDesktopWidget>

static const QWidget *topChild(const QWidget *w, bool bottom)
{
    if (w && w->isVisible()) {
        QList<QObject*>::const_iterator it = w->children().constEnd();
        while (it != w->children().constBegin()) {
            --it;
            QObject *o = *it;
            if (o->isWidgetType()) {
                QWidget *cw = (QWidget*)o;
                if (cw->isVisible() && !cw->testAttribute(Qt::WA_NoSystemBackground)) {
                    if ((!bottom && cw->y() < 3 && cw->x() < 20) ||
                        (bottom && w->height() - cw->geometry().bottom() - 1 < 3 && cw->x() < 20)) {
                            return topChild(cw, bottom);
                    }
                }
            }
        }
    }

    return w;
}

static inline QRgb blendRgb(QRgb rgb, int sr, int sg, int sb)
{
    int tmp = (rgb >> 16) & 0xff;
    int r = ((sr+tmp)/2);
    tmp = (rgb >> 8) & 0xff;
    int g = ((sg+tmp)/2);
    tmp = rgb & 0xff;
    int b = ((sb+tmp)/2);
    return qRgba(r, g, b, qAlpha(rgb));
}

static QPixmap colorizeImage(QImage img, const QColor &col)
{
    int count;
    int sr, sg, sb;
    col.getRgb(&sr, &sg, &sb);

    if (img.depth() == 32) {
        QRgb *rgb = (QRgb*)img.bits();
        count = img.bytesPerLine()/sizeof(QRgb)*img.height();
        for (int r = 0; r < count; r++, rgb++)
            *rgb = blendRgb(*rgb, sr, sg, sb);
    } else {
        QVector<QRgb> ctable = img.colorTable();
        for (int i = 0; i < ctable.count(); ++i)
            ctable[i] = blendRgb(ctable[i], sr, sg, sb);
        img.setColorTable(ctable);
    }
    return QPixmap::fromImage(img);
}

static const struct {
    const char *name;
    QPalette::ColorRole role;
} colorTable[] = {
    { "Foreground", QPalette::WindowText },
    { "WindowText", QPalette::WindowText },
    { "Button", QPalette::Button },
    { "Light", QPalette::Light },
    { "Midlight", QPalette::Midlight },
    { "Dark", QPalette::Dark },
    { "Mid", QPalette::Mid },
    { "Text", QPalette::Text },
    { "BrightText", QPalette::BrightText },
    { "ButtonText", QPalette::ButtonText },
    { "Base", QPalette::Base },
    { "Background", QPalette::Window },
    { "Window", QPalette::Window },
    { "Shadow", QPalette::Shadow },
    { "Highlight", QPalette::Highlight },
    { "HighlightedText", QPalette::HighlightedText },
    { "None", QPalette::NColorRoles },
    { 0, QPalette::Foreground }
};

static int parseColor(const QString &val, QColor &col)
{
    int role = QPalette::NColorRoles;
    if (!val.isEmpty()) {
        int i = 0;
        while (colorTable[i].name) {
            if (QString(colorTable[i].name).toLower() == val.toLower()) {
                role = colorTable[i].role;
                break;
            }
            i++;
        }
        if (!colorTable[i].name) {
            role = QPalette::NColorRoles+1;
            col.setNamedColor(val);
        }
    }

    return role;
}

static QColor getColor(const QPalette &pal, int role, const QColor &col)
{
    if (role < QPalette::NColorRoles)
        return pal.color((QPalette::ColorRole)role);
    else if (role == QPalette::NColorRoles)
        return QColor();
    else
        return col;
}

static const char *metricTable[] = {
    "TitleHeight",
    "LeftBorder",
    "RightBorder",
    "TopBorder",
    "BottomBorder",
    "OKWidth",
    "CloseWidth",
    "HelpWidth",
    "MaximizeWidth",
    "CornerGrabSize",
    0
};

class DecorationBorderData
{
public:
    QPixmap pix;
    QRegion rgn[3];
    int offs[4];

    void read(QSettings &config, const QString &base, const QString &name, Qt::Orientation, bool colorize=false);
};

void DecorationBorderData::read(QSettings &cfg, const QString &base, const QString &name, Qt::Orientation orient, bool colorize)
{
    QString val = cfg.value(name+"Pixmap").toString();
    if (val.isEmpty())
        return;
    QString cv = cfg.value(name+"Color").toString();
    if (colorize || !cv.isEmpty()) {
        QColor col = QApplication::palette().color(QPalette::Active, QPalette::Highlight);
        if (!cv.isEmpty()) {
            int role = parseColor(cv, col);
            col = getColor(QApplication::palette(), role, col);
        }
        QString key = base+val+col.name();
        // Cache only makes sense with shared pixmap cache enabled.
        if (!QPixmapCache::find(key, pix)) {
            pix = colorizeImage(QImage(":image/"+base+val), col);
            QPixmapCache::insert(key, pix);
        }
    } else {
        pix = QPixmap(":image/"+base+val);
    }

    QStringList offlist = cfg.value(name+"Offsets").toString().split(',', QString::SkipEmptyParts);
    offs[0] = 0;
    if (offlist.count()) {
        for (int i = 0; i < (int)offlist.count() && i < 3; i++)
            offs[i+1] = offlist[i].toInt();
    } else {
        offs[1] = 0;
        offs[2] = (orient == Qt::Vertical ? pix.height() : pix.width());
    }
    offs[3] = (orient == Qt::Vertical ? pix.height() : pix.width());

    QBitmap mask = pix.hasAlphaChannel() ? QBitmap() : pix.mask();
    val = cfg.value(name+"Mask").toString();
    if (!val.isEmpty())
        mask = QBitmap(":image/"+base+val);

    if (!mask.isNull()) {
        QRegion r = QRegion(mask);
        if (orient == Qt::Horizontal) {
            for (int i=0; i < 3; i++ )
                rgn[i] = r & QRegion(offs[i],0,offs[i+1]-offs[i],mask.height());
        } else {
            for (int i=0; i < 3; i++ )
                rgn[i] = r & QRegion(0, offs[i],mask.width(),offs[i+1]-offs[i]);
        }
    }
}

class DecorationData
{
public:
    DecorationData();

    void read(QSettings &config, const QString &base);

    enum Type { Title, Overlay, Left, Bottom, Right };
    const DecorationBorderData &area(Type t) const {
        return data[t];
    }

    int metrics[QWindowDecorationInterface::CornerGrabSize+1];

    QRect titleTextRect(int width) const;
    int titleTextAlignment() const;

    QColor textColor(const QPalette &pal) const;
    QColor titleColor(const QPalette &pal) const;
    QColor borderColor(const QPalette &pal) const;

private:
    DecorationBorderData data[5];
    int titleColorRole;
    QColor titleColorVal;
    int textColorRole;
    QColor textColorVal;
    int borderColorRole;
    QColor borderColorVal;
    QPoint tlOffset;
    QPoint brOffset;
    int titleAlign;
};

DecorationData::DecorationData()
{
}

void DecorationData::read(QSettings &cfg, const QString &base)
{
    int i = 0;
    const QChar percent('%');
    const QString pt("pt");
    while (metricTable[i]) {
        QVariant v = cfg.value(metricTable[i], 0);
        QDesktopWidget *desktop = QApplication::desktop();
        if (v.toString().contains(percent)) {
            metrics[i] = (int)(v.toString().remove(percent).toFloat() * desktop->height() / 100.0);
        }
        else if (v.toString().contains(pt)) {
            int dpi = desktop->screen()->physicalDpiY();
            metrics[i] = (int)(v.toString().remove(pt).toFloat() * dpi / 72.0);
        }
        else
            metrics[i] = v.toInt();
        i++;
    }

    data[Title].read(cfg, base, "Title", Qt::Horizontal);
    data[Overlay].read(cfg, base, "Overlay", Qt::Horizontal, true);
    data[Left].read(cfg, base, "Left", Qt::Vertical);
    data[Right].read(cfg, base, "Right", Qt::Vertical);
    data[Bottom].read(cfg, base, "Bottom", Qt::Horizontal);

    QString tc = cfg.value("BorderColor", "Window").toString();
    borderColorRole = parseColor(tc, borderColorVal);

    tc = cfg.value("TitleColor", "Highlight").toString();
    titleColorRole = parseColor(tc, titleColorVal);

    tc = cfg.value("TitleTextColor", "HighlightedText").toString();
    textColorRole = parseColor(tc, textColorVal);

    titleAlign = Qt::AlignHCenter | Qt::AlignVCenter;
    QString val = cfg.value("TitleTextAlignment").toString();
    if (!val.isEmpty()) {
        QStringList list = val.split(',');
        QStringList::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            val = *it;
            if (val == "right")
                titleAlign = (titleAlign & 0x38) | Qt::AlignRight;
            else if (val == "left")
                titleAlign = (titleAlign & 0x38) | Qt::AlignLeft;
            else if (val == "bottom")
                titleAlign = (titleAlign & 0x07) | Qt::AlignBottom;
            else if (val == "top")
                titleAlign = (titleAlign & 0x07) | Qt::AlignTop;
        }
    }

    val = cfg.value("TitleTextPosition").toString();
    if (!val.isEmpty()) {
        QStringList list = val.split(',');
        tlOffset.setX(list[0].toInt());
        tlOffset.setY(list[1].toInt());
        brOffset.setX(list[2].toInt());
        brOffset.setY(list[3].toInt());
    }
}


QColor DecorationData::titleColor(const QPalette &pal) const
{
    return getColor(pal, titleColorRole, titleColorVal);
}

QColor DecorationData::textColor(const QPalette &pal) const
{
    return getColor(pal, textColorRole, textColorVal);
}

QColor DecorationData::borderColor(const QPalette &pal) const
{
    return getColor(pal, borderColorRole, borderColorVal);
}

QRect DecorationData::titleTextRect(int width) const
{
    QRect r;

    r.setLeft(tlOffset.x());
    r.setTop(tlOffset.y());
    r.setRight(width-brOffset.x());
    r.setBottom(metrics[QWindowDecorationInterface::TitleHeight]-brOffset.y());

    return r;
}

int DecorationData::titleTextAlignment() const
{
    return titleAlign;
}

class PhoneDecorationPrivate
{
public:
    PhoneDecorationPrivate() {
        QSettings cfg("Trolltech","qpe");
        cfg.beginGroup("Appearance");
        QString theme = cfg.value("DecorationTheme", "qtopia/decorationrc").toString();
        QString themeFile = Qtopia::qtopiaDir() + "etc/themes/" + theme;

        // Search for decoration config
        QStringList instPaths = Qtopia::installPaths();
        foreach (QString path, instPaths) {
            QString themeDataPath(path + QLatin1String("etc/themes/") + theme);
            if (QFile::exists(themeDataPath)) {
                themeFile = themeDataPath;
                break;
            }
        }

        QSettings dcfg(themeFile, QSettings::IniFormat);
        dcfg.beginGroup("Decoration");
        QString base = dcfg.value("Base").toString();
        if (!base.isEmpty() && base[base.length()-1] != '/')
            base += '/';

        dcfg.endGroup();

        dcfg.beginGroup("Maximized");
        maxData.read(dcfg, base);
        dcfg.endGroup();
        dcfg.beginGroup("Normal");
        normalData.read(dcfg, base);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
        bgExport = new QExportedBackground(0);
#endif
    }

    ~PhoneDecorationPrivate() {
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
        delete bgExport;
#endif
    }

    const DecorationData &data(const QWindowDecorationInterface::WindowData *wd) {
        return (wd->flags & QWindowDecorationInterface::WindowData::Maximized) ? maxData : normalData;
    }

#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    QExportedBackground *bgExport;
#endif

private:
    DecorationData maxData;
    DecorationData normalData;
};

PhoneDecoration::PhoneDecoration()
{
    d = new PhoneDecorationPrivate;
}

PhoneDecoration::~PhoneDecoration()
{
    delete d;
}

int PhoneDecoration::metric( Metric m, const WindowData *wd ) const
{
    return d->data(wd).metrics[m];
}

QTOPIA_EXPORT int qtopia_background_brush_rotation(int screen);

void PhoneDecoration::drawArea( Area a, QPainter *p, const WindowData *wd ) const
{
    int th = metric( TitleHeight, wd );
    QRect r = wd->rect;

    QBrush bgBrush;
    bool paintBg = false;
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        QPixmap bgPm = d->bgExport->background();
        if (!bgPm.isNull()) {
            paintBg = true;
            bgBrush = QBrush(wd->palette.color(QPalette::Background), bgPm);
            int screen = QApplication::desktop()->screenNumber(wd->window);
            int rotation = qtopia_background_brush_rotation(screen);
            if (rotation != 0) {
                QTransform transform = bgBrush.transform();
                transform.rotate(rotation);
                bgBrush.setTransform(transform);
            }
        }
    }
#endif

    switch ( a ) {
        case Border:
            {
                QPalette pal = wd->palette;
                p->setBrush(d->data(wd).borderColor(pal));
                p->setPen( pal.color(QPalette::Foreground) );
                int lb = metric(LeftBorder,wd);
                int rb = metric(RightBorder,wd);
                int bb = metric(BottomBorder,wd);
                /*
                int tb = metric(TopBorder,wd);
                p->drawRect( r.x()-lb, r.y()-tb-th, r.width()+lb+rb,
                             r.height()+th+tb );
                */

                QBrush b = pal.brush(QPalette::Background);
                const DecorationBorderData *data;

                // left border
                if (lb) {
                    data = &d->data(wd).area(DecorationData::Left);
                    QRect tr(r.x()-lb, r.y(), lb, r.height());
                    if (paintBg) {
                        p->setBrushOrigin(-wd->window->geometry().topLeft());
                        p->fillRect(tr, bgBrush);
                        p->setBrushOrigin(0,0);
                    }
                    if (!data->pix.isNull())
                        drawStretch(p, tr, data, b, Qt::Vertical);
                }

                // right border
                if (rb) {
                    data = &d->data(wd).area(DecorationData::Right);
                    QRect tr(r.right()+1, r.y(), rb, r.height());
                    if (paintBg) {
                        p->setBrushOrigin(-wd->window->geometry().topLeft());
                        p->fillRect(tr, bgBrush);
                        p->setBrushOrigin(0,0);
                    }
                    if (!data->pix.isNull())
                        drawStretch(p, tr, data, b, Qt::Vertical);
                }

                // bottom border
                data = &d->data(wd).area(DecorationData::Bottom);
                if (bb) {
                    QRect tr(r.x()-lb, r.height(), r.width()+lb+rb, bb);
                    if (paintBg) {
                        p->setBrushOrigin(-wd->window->geometry().topLeft());
                        p->fillRect(tr, bgBrush);
                        p->setBrushOrigin(0,0);
                    }
                    QColor c = d->data(wd).borderColor(pal);
                    if (!c.isValid()) {
                        const QWidget *tc = topChild(wd->window, true);
                        switch (tc->backgroundRole()) {
                            case QPalette::Background:
                                if (paintBg)
                                    b = QBrush(Qt::NoBrush);
                                else
                                    b = pal.brush(QPalette::Background);
                                break;
                            case QPalette::Base:
                                b = pal.brush(QPalette::Base);
                                break;
                            case QPalette::Button:
                                b = pal.brush(QPalette::Button);
                                break;
                            default:
                                qWarning("Unhandled background mode in decoration");
                                break;
                        }
                    } else {
                        b = QBrush(c);
                    }
                    if (!data->pix.isNull())
                        drawStretch(p, tr, data, b, Qt::Horizontal);
                    else
                        p->fillRect(tr, b);
                }
            }
            break;
        case Title:
            if (r.height() >= 2) {
                int lb = metric(LeftBorder,wd);
                int rb = metric(RightBorder,wd);

                const DecorationBorderData *data = &d->data(wd).area(DecorationData::Title);
                const DecorationBorderData *odata = &d->data(wd).area(DecorationData::Overlay);

                QRect tr(r.x()-lb,r.y()-th,r.width()+lb+rb,th);
                if (paintBg) {
                    p->setBrushOrigin(-wd->window->geometry().topLeft());
                    p->fillRect(tr, bgBrush);
                    p->setBrushOrigin(0,0);
                }
                QPalette pal = wd->palette;
                QBrush b = pal.brush(QPalette::Background);
                const QWidget *tc = topChild(wd->window, false);
                pal = tc->palette();
                switch (tc->backgroundRole()) {
                    case QPalette::Background:
                        if (paintBg)
                            b = QBrush(Qt::NoBrush);
                        else
                            b = pal.brush(QPalette::Background);
                        break;
                    case QPalette::Base:
                        b = pal.brush(QPalette::Base);
                        break;
                    case QPalette::Button:
                        b = pal.brush(QPalette::Button);
                        break;
                    default:
                        break;
                }
                if (!data->pix.isNull() || !odata->pix.isNull()) {
                    if (!data->pix.isNull()) {
                        drawStretch(p, tr, data, b, Qt::Horizontal);
                        b = QBrush(Qt::NoBrush);
                    }
                    if (!odata->pix.isNull())
                        drawStretch(p, tr, odata, b, Qt::Horizontal);
                } else {
                    p->fillRect( tr, b );
                    const QPalette &pal = wd->palette;
                    QColor c(d->data(wd).titleColor(pal));
                    if (c.isValid())
                        p->fillRect( tr, c );
                }
            } else if ( r.height() < 2 ) {
                QWindowDecorationInterface::drawArea( a, p, wd );
            }
            break;
        case TitleText:
            if (!(wd->flags & QWindowDecorationInterface::WindowData::Maximized)) {
                //use current font or we run into trouble if non Western language
                QFont f( QApplication::font() );
                f.setWeight( QFont::Bold );
                f.setPointSize( 7 );
                p->setFont(f);
                if ( !(wd->flags & WindowData::Active) )
                    p->setPen(d->data(wd).textColor(wd->palette));
                else
                    p->setPen(d->data(wd).textColor(wd->palette));
                QRect tr = d->data(wd).titleTextRect(r.width());
                p->drawText( r.left()+tr.left(), r.top()-th+tr.top(),
                             tr.width(), tr.height(),
                             d->data(wd).titleTextAlignment(), wd->caption );
            }
            break;
        default:
            QWindowDecorationInterface::drawArea( a, p, wd );
            break;
    }
}

void PhoneDecoration::drawButton( Button , QPainter* , const WindowData* ,
    int , int , int /* w */, int /* h */, QDecoration::DecorationState ) const
{
}

QRegion PhoneDecoration::mask( const WindowData *wd ) const
{
    QRegion mask;

    int th = metric( TitleHeight, wd );
    int lb = metric( LeftBorder, wd );
    int rb = metric( RightBorder, wd );
    int bb = metric( BottomBorder, wd );
    QRect r(wd->rect);

    // top
    const DecorationBorderData *data = &d->data(wd).area(DecorationData::Title);
    if (data->pix.isNull())
        data = &d->data(wd).area(DecorationData::Overlay);
    QRect tr(r.x()-lb,r.y()-th,r.width()+rb+lb,th);
    mask += maskStretch(tr, data, Qt::Horizontal);

    // left
    if (lb) {
        data = &d->data(wd).area(DecorationData::Left);
        tr.setRect(r.x()-lb,r.y(),lb,r.height());
        mask += maskStretch(tr, data, Qt::Vertical);
    }

    // right
    if (rb) {
        data = &d->data(wd).area(DecorationData::Right);
        tr.setRect(r.right()+1,r.y(),rb,r.height());
        mask += maskStretch(tr, data, Qt::Vertical);
    }

    // bottom
    if (bb) {
        data = &d->data(wd).area(DecorationData::Bottom);
        tr.setRect(r.x()-lb, r.bottom()+1, r.width()+lb+rb, bb);
        mask += maskStretch(tr, data, Qt::Horizontal);
    }

    return mask;
}

QString PhoneDecoration::name() const
{
    return qApp->translate( "Decoration", "Phone" );
}

QPixmap PhoneDecoration::icon() const
{
    return QPixmap();
}

void PhoneDecoration::drawStretch(QPainter *p, const QRect &r, const DecorationBorderData *data,
                                const QBrush &b, Qt::Orientation orient) const
{
    p->fillRect(r, b);
    if (data->pix.isNull())
        return;

    const int *o = data->offs;
    int ss = o[2]-o[1];
    if (orient == Qt::Horizontal) {
        int h = data->pix.height();
        p->drawPixmap(r.x(), r.y(), data->pix, 0, 0, o[1], h);
        int w = r.width() - o[1] - (o[3]-o[2]);
        int x = 0;
        if (ss) {
            for (; x < w-ss; x+=ss)
                p->drawPixmap(r.x()+o[1]+x, r.y(), data->pix, o[1], 0, ss, h);
        }
        p->drawPixmap(r.x()+o[1]+x, r.y(), data->pix, o[1], 0, w-x, h);
        p->drawPixmap(r.x()+r.width()-(o[3]-o[2]), r.y(), data->pix, o[2], 0, o[3]-o[2], h);
    } else {
        int w = data->pix.width();
        p->drawPixmap(r.x(), r.y(), data->pix, 0, 0, w, o[1]);
        int h = r.height() - o[1] - (o[3]-o[2]);
        int y = 0;
        if (ss) {
            for (; y < h-ss; y+=ss) {
                p->drawPixmap(r.x(), r.y()+o[1]+y, data->pix, 0, o[1], w, ss);
            }
        }
        p->drawPixmap(r.x(), r.y()+o[1]+y, data->pix, 0, o[1], w, h-y);
        p->drawPixmap(r.x(), r.y()+r.height()-(o[3]-o[2]), data->pix, 0, o[2], w, o[3]-o[2]);
    }
}

QRegion PhoneDecoration::maskStretch(const QRect &r, const DecorationBorderData *data, Qt::Orientation orient) const
{
    if (data->rgn[0].isEmpty() && data->rgn[1].isEmpty() && data->rgn[2].isEmpty())
        return QRegion(r);

    const int *o = data->offs;
    QRegion mask;
    QRegion trgn;

    if (orient == Qt::Horizontal) {
        trgn = data->rgn[0].isEmpty() ? QRect(0, 0, o[1], data->pix.height()) : data->rgn[0];
        trgn.translate(r.x(), r.y());
        mask = trgn;

        int w = r.width() - o[1] - (o[3]-o[2]);
        if (!data->rgn[1].isEmpty()) {
            QRegion tmp(data->rgn[1]);
            tmp.translate(r.x(), r.y());
            for (int x = 0; x < w; x+=o[2]-o[1]) {
                trgn += tmp;
                tmp.translate(o[2]-o[1], 0);
            }
            trgn &= QRect(r.x()+o[1], r.y(), w, r.height());
        } else {
            trgn = QRect(r.x()+o[1], r.y(), w, r.height());
        }
        mask += trgn;

        trgn = data->rgn[2].isEmpty() ? QRect(o[2], 0, o[3]-o[2], data->pix.height()) : data->rgn[2];
        trgn.translate(r.x()+r.width()-o[3],r.y());
        mask += trgn;
    } else {
        trgn = data->rgn[0].isEmpty() ? QRect(0, 0, data->pix.width(), o[1]) : data->rgn[0];
        trgn.translate(r.x(), r.y());
        mask = trgn;

        int h = r.height() - o[1] - (o[3]-o[2]);
        if (!data->rgn[1].isEmpty()) {
            QRegion tmp(data->rgn[1]);
            tmp.translate(r.x(), r.y());
            for (int y = 0; y < h; y+=o[2]-o[1]) {
                trgn += tmp;
                tmp.translate(0, o[2]-o[1]);
            }
            trgn &= QRect(r.x(), r.y()+o[1], r.width(), h);
        } else {
            trgn = QRect(r.x(), r.y()+o[1], r.width(), h);
        }
        mask += trgn;

        trgn = data->rgn[2].isEmpty() ? QRect(0, o[2], data->pix.width(), o[3]-o[2]) : data->rgn[2];
        trgn.translate(r.x(), r.y()+r.height()-o[3]);
        mask += trgn;
    }

    return mask;
}

