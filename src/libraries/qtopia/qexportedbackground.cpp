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

#include <qtopiachannel.h>
#include <QPixmap>
#include <QDebug>
#include <qtopiaipcenvelope.h>
#include <qscreen_qws.h>
#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QDesktopWidget>
#include <QFile>
#include <QStyle>

#include "qglobalpixmapcache.h"
#include "qexportedbackground.h"

static const int MaxScreens = 2;

#define USE_PIXMAP_QWS_BITS // XXX may not work on some display drivers

#ifndef USE_PIXMAP_QWS_BITS
static void colorize(QImage &, const QImage &, const QColor &);
#else
static void colorize(QPixmap &, const QPixmap &, const QColor &);
#endif

class ExportedBackground {
public:
    ExportedBackground() : bgPm(0), bgState(0),
        exportedBackgroundAvailable(false) {}
    QPixmap *bgPm;
    QPixmap *bgState;
    bool exportedBackgroundAvailable;
};

struct ExportedBackgroundLocalInfo {
    ExportedBackgroundLocalInfo()
        : tintAmount(2) {}

    int tintAmount;
    ExportedBackground exportedBg[MaxScreens];
    QSet<QExportedBackground *> localInstance;
};

Q_GLOBAL_STATIC(ExportedBackgroundLocalInfo, localInfo);

class QExportedBackgroundPrivate
{
public:
    QPixmap bg;
    QPixmap state;
    int screen;
};


/*!
    \class QExportedBackground
    \inpublicgroup QtBaseModule

    \brief The QExportedBackground class provides access to the system
    background.

    Qt Extended provides a global background to all windows in order to
    give the impression that windows are transparent.  This
    background is automatically available in all windows due to
    QPhoneStyle setting appropriate palettes for all widgets.  In some
    cases it is desireable to use the background directly, or to be
    notified that the background pixmap has changed.  QExportedBackground
    provides this functionality.
*/

/*!
    \fn void QExportedBackground::changed()

    This signal is emitted when the exported background changes.
*/

/*!
    \fn void QExportedBackground::changed(const QPixmap &background)

    This signal is emitted when the exported background changes.  The
    \a background argument is the new background.
*/

/*!
    \fn void QExportedBackground::wallpaperChanged()
    \deprecated
    This signal is emitted when the wallpaper changes.

    \sa wallpaper()
*/

/*!
    Constructs a QExportedBackground for screen number \a screen with
    the given \a parent.

    Qt supports multiple screens.  This is commonly seen in flip phones
    with both internal and external screens.  The background for each screen
    may be different.  Qt Extended assumes that the primary screen
    is screen number \c 0 and the secondary screen is screen number \c 1.

    \sa QDesktopWidget
*/
QExportedBackground::QExportedBackground(int screen, QObject *parent)
    : QObject(parent)
{
    d = new QExportedBackgroundPrivate;
    d->screen = screen;
    if (screen >= 0 && screen < MaxScreens) {
        QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
        connect( sysChannel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(sysMessage(QString,QByteArray)) );
        getPixmaps();
    }
    localInfo()->localInstance.insert(this);
}

/*!
    Constructs a QExportedBackground for the default screen with the
    given \a parent.
*/
QExportedBackground::QExportedBackground(QObject *parent)
    : QObject(parent)
{
    int scr = 0;
    d = new QExportedBackgroundPrivate;
    d->screen = scr;
    if (scr >= 0 && scr < MaxScreens) {
        QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
        connect( sysChannel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(sysMessage(QString,QByteArray)) );
        getPixmaps();
    }
    localInfo()->localInstance.insert(this);
}

/*!
    Destroys a QExportedBackground.
*/
QExportedBackground::~QExportedBackground()
{
    if ( d )
        delete d;
    d = 0;
    localInfo()->localInstance.remove(this);
}

/*!
    \deprecated
    Returns the wallpaper.
    
    As of Qtopia 4.3 the wallpaper is the same as the background. Use background() instead.
*/
QPixmap QExportedBackground::wallpaper() const
{
    return background();
}

/*!
    Returns the exported background.
*/
const QPixmap &QExportedBackground::background() const
{
    return d->bg;
}

/*!
    Returns whether an exported background is available.
*/
bool QExportedBackground::isAvailable() const
{
#ifndef USE_PIXMAP_QWS_BITS
    return (!d->state.isNull() && !d->bg.isNull());
#else
    return (!d->state.isNull() && *d->state.qwsBits() && !d->bg.isNull());
#endif
}

void QExportedBackground::sysMessage(const QString &msg, const QByteArray&)
{
    if(msg == "backgroundChanged()") {
        getPixmaps();
        emit changed();
        emit changed(background());
        emit wallpaperChanged();
    }
}

/*!
    \internal
*/
void QExportedBackground::polishWindows(int screen)
{
    QApplication::setPalette(QApplication::palette());

    foreach (QWidget *w, QApplication::topLevelWidgets()) {
        if (QApplication::desktop()->screenNumber(w) == screen) {
            QApplication::style()->polish(w);
            foreach (QObject *o, w->children()) {
                QWidget *sw = qobject_cast<QWidget*>(o);
                if (sw) {
                    QApplication::style()->polish(sw);
                }
            }
        }
    }
}

void QExportedBackground::getPixmaps()
{
    d->bg = QPixmap();
    d->state = QPixmap();
    QString stateKey = QString("_$QTOPIA_BGSTATE_%1").arg(d->screen);
    QString bgKey = QString("_$QTOPIA_BG_%1").arg(d->screen);
    QGlobalPixmapCache::find(stateKey, d->state);
    QGlobalPixmapCache::find(bgKey, d->bg);
}

// Server side

int qtopia_background_brush_rotation(int screen); // from qphonestyle.cpp

/*!
    \internal
*/
QSize QExportedBackground::exportedBackgroundSize(int screen)
{
    QSize size = QApplication::desktop()->screenGeometry(screen).size();
    QList<QScreen *> screens = qt_screen->subScreens();
    if (screens.isEmpty())
        screens.append(qt_screen);
    int rot = qtopia_background_brush_rotation(screen);
    if (screen < screens.size() && (rot == 90 || rot == 270)) {
        // The screen is transformed so that width and height are swapped.
        // Use the untransformed size when creating the exported background.
        size = QSize(size.height(), size.width());
    }
    return size;
}

/*!
    \internal
*/
void QExportedBackground::initExportedBackground(int width, int height, int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if (expBg.bgState)
        return;
    expBg.exportedBackgroundAvailable = false;

    QString stateKey = QString("_$QTOPIA_BGSTATE_%1").arg(screen);
    QString bgKey = QString("_$QTOPIA_BG_%1").arg(screen);

    expBg.bgState = new QPixmap();
    expBg.bgPm = new QPixmap();
    QGlobalPixmapCache::find(stateKey, *expBg.bgState);
    if (expBg.bgState->isNull()) {
        *expBg.bgState = QPixmap(1,1);
        if (!QGlobalPixmapCache::insert(stateKey, *expBg.bgState)) {
            qWarning() << "Could not store exported background in global cache";
            return;
        }
#ifdef USE_PIXMAP_QWS_BITS
        *((uchar*)expBg.bgState->qwsBits()) = 0; // Not set
#endif
    }
    QGlobalPixmapCache::find(bgKey, *expBg.bgPm);
    if (expBg.bgPm->isNull()) {
        QImage::Format fmt = QApplication::desktop()->depth() <= 16 ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied;
        QImage img(width, height, fmt);
        *expBg.bgPm = QPixmap::fromImage(img);
        if(!QGlobalPixmapCache::insert(bgKey, *expBg.bgPm)) {
            qWarning() << "Could not store exported background in global cache";
            return;
        }
    }

    expBg.exportedBackgroundAvailable = true;
    QtopiaIpcEnvelope e("QPE/System", "backgroundChanged()");
}

/*!
    \internal
*/
void QExportedBackground::clearExportedBackground(int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if(!expBg.exportedBackgroundAvailable)
        return;

#ifdef USE_PIXMAP_QWS_BITS
    *((uchar*)expBg.bgState->qwsBits()) = 0; // Not set

#endif
    foreach(QExportedBackground *bg, localInfo()->localInstance)
        bg->getPixmaps();
}

/*!
    \internal
*/
void QExportedBackground::setExportedBackgroundTint(int tint)
{
    localInfo()->tintAmount = tint;
}

/*!
    \internal
*/
void QExportedBackground::setExportedBackground(const QPixmap &image, int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if(!expBg.exportedBackgroundAvailable)
        return;

    if(image.isNull()) {
        clearExportedBackground();
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

#ifndef USE_PIXMAP_QWS_BITS
    QImage expBgimage = expBg.bgPm->toImage();
    QImage imageimage = image.toImage();
    colorize(expBgimage, imageimage, bgCol);
    expBg.bgPm->fromImage(expBgimage);
#else
    colorize(*expBg.bgPm, image, bgCol);
#endif

#ifdef USE_PIXMAP_QWS_BITS
    *((uchar*)expBg.bgState->qwsBits()) = 1; // Set
#endif
    QtopiaIpcEnvelope e("QPE/System", "backgroundChanged()");
   
    foreach(QExportedBackground *bg, localInfo()->localInstance)
        bg->getPixmaps();
}

#ifndef USE_PIXMAP_QWS_BITS
static void colorize(QImage &dest, const QImage &src,
                                   const QColor &colour)
#else
static void colorize(QPixmap &dest, const QPixmap &src,
                                   const QColor &colour)
#endif
{
    int sr, sg, sb;
    colour.getRgb(&sr, &sg, &sb);
    int level = colour.alpha();
    int div = 255;
    int mult = 255-level;
    QSize dataSize = QSize(src.width(),src.height());
    if (src.depth() == 16 && dest.depth() == 16) {
        sr = (sr << 8) & 0xF800;
        sg = (sg << 3) & 0x07e0;
        sb = sb >> 3;
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
#ifndef USE_PIXMAP_QWS_BITS
        int count = src.bytesPerLine()/2 * dataSize.height();
        ushort *sp = (ushort *)src.bits();
        ushort *dp = (ushort *)dest.bits();
#else
        int count = src.qwsBytesPerLine()/2 * dataSize.height();
        ushort *sp = (ushort *)src.qwsBits();
        ushort *dp = (ushort *)dest.qwsBits();
#endif
        for (int x = 0; x < count; x++, dp++, sp++) {
            quint32 spix = *sp;
            quint32 r = ((spix & 0x0000F800)*mult + const_sr)/div;
            quint32 g = ((spix & 0x000007e0)*mult + const_sg)/div;
            quint32 b = ((spix & 0x0000001f)*mult + const_sb)/div;
            *dp = (r&0xF800) | (g&0x07e0) | (b&0x001f);
        }
    } else if (src.depth() == 32 && dest.depth() == 32) {
        int map[3*256];
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
        for (int i = 0; i < 256; i++)
        {
            map[i] = ((const_sr+i*mult)/div);
            map[i+256] = ((const_sg+i*mult)/div);
            map[i+512] = ((const_sb+i*mult)/div);
        }
#ifndef USE_PIXMAP_QWS_BITS
        QRgb *srgb = (QRgb*)src.bits();
        QRgb *drgb = (QRgb*)dest.bits();
        int count = src.bytesPerLine()/sizeof(QRgb) * src.height();
#else
        QRgb *srgb = (QRgb*)src.qwsBits();
        QRgb *drgb = (QRgb*)dest.qwsBits();
        int count = src.qwsBytesPerLine()/sizeof(QRgb) * src.height();
#endif
        for (int i = 0; i < count; i++, srgb++, drgb++) {
            int r = (*srgb >> 16) & 0xff;
            int g = (*srgb >> 8) & 0xff;
            int b = *srgb & 0xff;
            r = map[r];
            g = map[g+256];
            b = map[b+512];
            *drgb = qRgb(r, g, b);
        }
    } else if (src.depth() == 32 && dest.depth() == 16) {
        int map[3*256];
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
        for (int i = 0; i < 256; i++)
        {
            map[i] = ((const_sr+i*mult)/div);
            map[i+256] = ((const_sg+i*mult)/div);
            map[i+512] = ((const_sb+i*mult)/div);
        }
#ifndef USE_PIXMAP_QWS_BITS
        QRgb *srgb = (QRgb*)src.bits();
        ushort *dp = (ushort *)dest.bits();
        int count = src.bytesPerLine()/sizeof(QRgb) * src.height();
#else
        QRgb *srgb = (QRgb*)src.qwsBits();
        ushort *dp = (ushort *)dest.qwsBits();
        int count = src.qwsBytesPerLine()/sizeof(QRgb) * src.height();
#endif
        for (int i = 0; i < count; i++, srgb++, dp++) {
            int r = (*srgb >> 16) & 0xff;
            int g = (*srgb >> 8) & 0xff;
            int b = *srgb & 0xff;
            r = map[r];
            g = map[g+256];
            b = map[b+512];
            *dp = (((r>>3)<<11)&0xF800) | (((g>>2)<<5)&0x07e0) | ((b>>3)&0x001f);
        }
    }
}

