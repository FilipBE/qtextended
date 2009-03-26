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

#include "qexportedbackground.h"
#include <QtopiaApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QtopiaChannel>
#include <QPainter>
#include <private/qpixmap_x11_p.h>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <custom.h>

#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
static void colorize(QPixmap &, const QPixmap &, const QColor &);

class QExportedBackgroundServerPrivate
{
public:
    QMap<int, QPixmap> backgrounds;
    QList<QExportedBackground *> localInstances;
};
Q_GLOBAL_STATIC(QExportedBackgroundServerPrivate, exportedBackgroundServer);

static Atom makeBackgroundAtom(int screen)
{
    QString atomName("_QTOPIA_BG_");
    atomName += QString::number(screen);
    QByteArray atomName2 = atomName.toLatin1();
    return XInternAtom(QX11Info::display(), atomName2.data(), False);
}

class QExportedBackgroundPrivate : public QtopiaApplication::X11EventFilter
{
public:
    QExportedBackgroundPrivate(QExportedBackground *parent, int screen)
    {
        exportedBackgroundServer()->localInstances.append(parent);
        this->parent = parent;
        this->screen = screen;
        this->rawPixmap = 0;
        backgroundAtom = makeBackgroundAtom(screen);
        QtopiaApplication::instance()->installX11EventFilter(this);
    }
    ~QExportedBackgroundPrivate()
    {
        clearBg();
        QtopiaApplication::instance()->removeX11EventFilter(this);
        exportedBackgroundServer()->localInstances.removeAll(parent);
    }

    QExportedBackground *parent;
    QPixmap bg;
    Pixmap rawPixmap;
    int screen;
    Atom backgroundAtom;

    bool x11EventFilter(XEvent *event);
    void clearBg();
};

bool QExportedBackgroundPrivate::x11EventFilter(XEvent *event)
{
    if (event->xany.type == PropertyNotify &&
        event->xany.window == QX11Info::appRootWindow() &&
        event->xproperty.atom == backgroundAtom) {
        // The exported background for this screen has been updated.
        parent->getPixmaps();
        return true;
    }
    return false;
}

// Nasty - use a friend class to access QPixmap private data
class QX11PaintEngine {
public:
    static QPixmapData *getPixmapData(const QPixmap &pm) {
        return pm.data;
    }
};

void QExportedBackgroundPrivate::clearBg()
{
    if (!bg.isNull()) {
        // Prevent Qt from destroying the foreign pixmap and picture handles.
        // FIXME private
//         QX11PixmapData *pd = static_cast<QX11PixmapData*>(QX11PaintEngine::getPixmapData(bg));
//         pd->hd = 0;
//         pd->picture = 0;
//         pd->deref();
        // Remove extra reference added in getPixmaps().
    }
    bg = QPixmap();
}

QExportedBackground::QExportedBackground(QObject *parent)
    : QObject(parent)
{
    d = new QExportedBackgroundPrivate(this, 0);
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect(sysChannel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(sysMessage(QString,QByteArray)));
    getPixmaps();
}

QExportedBackground::QExportedBackground(int screen, QObject *parent)
    : QObject(parent)
{
    d = new QExportedBackgroundPrivate(this, screen);
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect(sysChannel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(sysMessage(QString,QByteArray)));
    getPixmaps();
}

QExportedBackground::~QExportedBackground()
{
    delete d;
}

QPixmap QExportedBackground::wallpaper() const
{
    return background();
}

const QPixmap &QExportedBackground::background() const
{
    return d->bg;
}

bool QExportedBackground::isAvailable() const
{
    return !d->bg.isNull();
}

QSize QExportedBackground::exportedBackgroundSize(int screen)
{
    return QApplication::desktop()->screenGeometry(screen).size();
}

void QExportedBackground::initExportedBackground(int, int, int screen)
{
    // Clear left-over atoms from previous run of Qtopia.
    clearExportedBackground(screen);
}

void QExportedBackground::clearExportedBackground(int screen)
{
    Atom backgroundAtom = makeBackgroundAtom(screen);
    XDeleteProperty(QX11Info::display(), QX11Info::appRootWindow(), backgroundAtom);
    exportedBackgroundServer()->backgrounds.remove(screen);

    // Update the local instances because we cannot wait for a round-trip.
    foreach(QExportedBackground *bg, exportedBackgroundServer()->localInstances)
        bg->getPixmaps();
}

void QExportedBackground::setExportedBackgroundTint(int)
{
    // Not used.
}

void QExportedBackground::setExportedBackground(const QPixmap &image, int screen)
{
    // Just clear the background if it is null.
    if (image.isNull()) {
        clearExportedBackground(screen);
        return;
    }

    // Get background color direct from settings in case style has
    // messed with it.
    QColor bgCol;
    QSettings config(QLatin1String("Trolltech"),QLatin1String("qpe"));
    config.beginGroup( QLatin1String("Appearance") );
    QString value = config.value("Background", "#EEEEEE").toString();
    if ( value[0] == '#' ) {
        bgCol = QColor(value);
        int alpha = config.value("Background_alpha", "64").toInt();
        bgCol.setAlpha(alpha);
    } else {
        bgCol = QApplication::palette().color(QPalette::Window);
    }

    // Make a copy of the pixmap and tint it.
    QPixmap bg(image.width(), image.height());
    colorize(bg, image, bgCol);
    bg.detach();
    exportedBackgroundServer()->backgrounds[screen] = bg;

    // Set the background as a root window property so that other
    // applications can access it.  We pass both the pixmap and the
    // Xrender picture ID across to the other applications.
    Atom backgroundAtom = makeBackgroundAtom(screen);
    Pixmap pixmaps[2];
    pixmaps[0] = bg.handle();
    pixmaps[1] = bg.x11PictureHandle();
    XChangeProperty(QX11Info::display(), QX11Info::appRootWindow(),
                    backgroundAtom, XA_PIXMAP, 32, PropModeReplace,
                    (unsigned char *)pixmaps, 2);

    // Update the local instances because we cannot wait for a round-trip.
    foreach(QExportedBackground *bg, exportedBackgroundServer()->localInstances)
        bg->getPixmaps();
}

void QExportedBackground::sysMessage(const QString& msg,const QByteArray&)
{
    // backgroundChanged() is not needed here because X11 property
    // notifications are used to advertise changes in the background.
}

void QExportedBackground::getPixmaps()
{
    Atom actualType;
    int actualFormat;
    unsigned long size;
    unsigned long bytesAfterReturn;
    unsigned char *value;
    Pixmap rawPixmap = 0;
    unsigned long rawPicture = 0;
    if (XGetWindowProperty
            (QX11Info::display(), QX11Info::appRootWindow(),
             d->backgroundAtom, 0, 2, False, XA_PIXMAP,
             &actualType, &actualFormat, &size, &bytesAfterReturn,
             &value ) == Success && value ) {
        rawPixmap = (Pixmap)(*((unsigned long *)value));
        rawPicture = ((unsigned long *)value)[1];
        XFree(value);
    }
    if (rawPixmap) {
        if (rawPixmap != d->rawPixmap) {
            Window rootReturn;
            int xReturn, yReturn;
            unsigned int widthReturn, heightReturn;
            unsigned int borderWidthReturn, depthReturn;
            d->clearBg();
            if (XGetGeometry(QX11Info::display(), rawPixmap, &rootReturn,
                             &xReturn, &yReturn, &widthReturn, &heightReturn,
                             &borderWidthReturn, &depthReturn)) {

// FIXME private
//                QX11PixmapData *pd = static_cast<QX11PixmapData*>(QX11PaintEngine::getPixmapData(d->bg));
//                 pd->w = (int)widthReturn;
//                 pd->h = (int)heightReturn;
//                 pd->d = (short)depthReturn;
//                 pd->hd = rawPixmap;
//                 pd->picture = rawPicture;

//                 // Artificially inflate the reference count so that Qt
//                 // will not destroy the foreign pixmap and picture handles.
//                 // We will clear the handles in clearBg() before releasing
//                 // the last reference to the pixmap data.
//                 pd->ref();
            }
            d->rawPixmap = rawPixmap;
        }
    } else {
        // Property is not set, so the background pixmap must be cleared.
        d->clearBg();
        d->rawPixmap = 0;
    }

    // Advertise the background changes.
    emit changed();
    emit changed(background());
    emit wallpaperChanged();
}

static void colorize(QPixmap &dest, const QPixmap &src, const QColor &color)
{
    dest.fill(Qt::black);
    QPainter painter(&dest);
    painter.setOpacity(((qreal)color.alpha())/ 255.0);
    painter.drawPixmap(0, 0, src);
}

#else // !QTOPIA_ENABLE_EXPORTED_BACKGROUNDS

QExportedBackground::QExportedBackground(QObject *parent)
    : QObject(parent)
{
}

QExportedBackground::QExportedBackground(int screen, QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(screen);
}

QExportedBackground::~QExportedBackground()
{
}

QPixmap QExportedBackground::wallpaper() const
{
    return QPixmap();
}

const QPixmap &QExportedBackground::background() const
{
    static QPixmap bg;
    return bg;
}

bool QExportedBackground::isAvailable() const
{
    return false;
}

QSize QExportedBackground::exportedBackgroundSize(int screen)
{
    return QApplication::desktop()->screenGeometry(screen).size();
}

void QExportedBackground::initExportedBackground(int width, int height, int screen )
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(screen);
}

void QExportedBackground::clearExportedBackground(int screen)
{
    Q_UNUSED(screen);
}

void QExportedBackground::setExportedBackgroundTint(int)
{
}

void QExportedBackground::setExportedBackground(const QPixmap &image, int screen)
{
    Q_UNUSED(image);
    Q_UNUSED(screen);
}

void QExportedBackground::sysMessage(const QString&,const QByteArray&)
{
}

void QExportedBackground::getPixmaps()
{
}

#endif // !QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
