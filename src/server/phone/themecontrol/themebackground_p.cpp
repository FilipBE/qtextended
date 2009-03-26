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

#include "themebackground_p.h"
#include "themecontrol.h"

#include <QPainter>
#include <QSvgRenderer>
#include <qtopiachannel.h>
#include <QSettings>
#include <qtopialog.h>
#include <themedview.h>
#include <QDesktopWidget>
#include <qtopiaapplication.h>
#include <QFile>
#include <custom.h>
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
#include <QExportedBackground>
#endif

// Theme background is responsible for extracting and painting
// the background specified by the theme.

ThemeBackground::ThemeBackground(QObject *)
    : ThemedItemPlugin(), themedView(0)
{
}

ThemeBackground::ThemeBackground(ThemedView *themedView)
    : ThemedItemPlugin(), themedView(themedView)
{
    updateBackground();
}

ThemeBackground::~ThemeBackground()
{
}

void ThemeBackground::resize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

QTOPIA_EXPORT int qtopia_background_brush_rotation(int screen); // From qphonestyle.cpp

void ThemeBackground::paint(QPainter *painter, const QRect &)
{
    if (!themedView)
        return;
    int screen = QApplication::desktop()->screenNumber(themedView);
    QRect sr = QtopiaApplication::desktop()->screenGeometry(screen);
    int rotation = qtopia_background_brush_rotation(screen);
    if (rotation != 0) {
        QBrush brush(bg);
        QTransform transform = brush.transform();
        transform.rotate(rotation);
        brush.setTransform(transform);
        painter->fillRect(sr, brush);
    } else {
        painter->drawPixmap(sr, bg);
    }
}

// Added to enable the background to be persisted.
void ThemeBackground::updateBackground()
{
    if (!themedView)
        return;
    QSettings cfg("Trolltech", "qpe");
    cfg.beginGroup("Appearance");
    QString image = cfg.value("BackgroundImage").toString();
    QString path = ":image/" + themedView->base().toLower() + image;
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    int screen = QApplication::desktop()->screenNumber(themedView);
    bg = QPixmap(path).scaled(QExportedBackground::exportedBackgroundSize(screen));
#else
    bg = QPixmap(path);
#endif
}

//===========================================================================

// HomeScreenImagePlugin provides user selectable homescreen images.

HomeScreenImagePlugin::HomeScreenImagePlugin(ThemedView *themedView)
    : ThemedItemPlugin(), width(0), height(0), dpMode(Center), ref(0)
    , themedView(themedView)
{
    QSettings config("Trolltech","qpe");
    config.beginGroup("HomeScreen");

    int screen = QApplication::desktop()->screenNumber(themedView);
    switch (screen) {
    case 0:
        imgName = config.value("HomeScreenPicture").toString();
        dpMode = (DisplayMode)config.value("HomeScreenPictureMode").toInt();
        break;
    case 1:
        imgName = config.value("SecondaryHomeScreenPicture").toString();
        dpMode = (DisplayMode)config.value("SecondaryHomeScreenPictureMode").toInt();
        break;
    }

    if (!imgName.isEmpty())
    {
        imgContent.setPermission (QDrmRights::Display);
        imgContent.setLicenseOptions (QDrmContent::Handover | QDrmContent::Reactivate);

        connect (&imgContent, SIGNAL (rightsExpired(QDrmContent)),
                 this, SLOT (rightsExpired(QDrmContent)));

        if (!imgContent.requestLicense(QContent(imgName, false)))
            return;

        imgContent.renderStarted();
        qLog(UI) << "Loading home screen picture:" << imgName;
    }
}

void HomeScreenImagePlugin::renderSvg(int w, int h, Qt::AspectRatioMode mode)
{
    QSvgRenderer r(imgName);
    QSize svgSize = r.defaultSize();
    svgSize.scale(w, h, mode);
    bg = QPixmap(svgSize);
    bg.fill(QColor(0,0,0,0));
    QPainter p(&bg);
    r.render(&p);
}

void HomeScreenImagePlugin::resize(int w, int h)
{
    width = w;
    height = h;

    // dpmode
    // ScaleAndCrop - fill viewport keeping aspect ratio by expanding
    // Stretch - stretch image to fill viewport ignoring aspect ratio
    // Center/Tile - no resize
    // Scale - fit all image to viewport
    if (!imgName.isEmpty()) {
        if (dpMode == Center || dpMode == Tile) {
            bg.load(imgName);

            if (imgName.endsWith(".svg"))
                renderSvg(bg.width(), bg.height(), Qt::KeepAspectRatio);
        } else {
            // gussing viewport size
            int screen = QApplication::desktop()->screenNumber(themedView);
            QRect availableRect = QtopiaApplication::desktop()->availableGeometry(screen);
            height = qMin(availableRect.height(), height);
            width = qMin(availableRect.width(), width);

            Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
            if (dpMode == ScaleAndCrop) mode = Qt::KeepAspectRatioByExpanding;
            else if (dpMode == Stretch) mode = Qt::IgnoreAspectRatio;
            else if (dpMode == Scale) mode = Qt::KeepAspectRatio;

            if (imgName.endsWith(".svg")) {
                renderSvg(width, height, mode);
            } else {
                QPixmap p;
                p.load(imgName);
                bg = p.scaled(QSize(width, height), mode);
            }
        }
    }
    qLog(UI) << "Home screen picture display mode:"
        << ((dpMode == ScaleAndCrop) ? "Scale and Crop" : (dpMode == Stretch) ? "Stretch"
                : (dpMode == Center) ? "Center" : (dpMode == Tile) ? "Tile" : "Scale");
    qLog(UI) << "Available screen size:" << width << "x" << height
        << "Homescreen image size:" << bg.size().width() << "x" << bg.size().height();
}

void HomeScreenImagePlugin::paint(QPainter *p, const QRect &r)
{
    Q_UNUSED(r);
    if (!bg.isNull()) {
        int screen = QApplication::desktop()->screenNumber(themedView);
        QRect cr = QtopiaApplication::desktop()->availableGeometry(screen);

        // ensure image is drawn as if screen origin is at (0,0)
        // (secondary displays may not have (0,0) as screen origin)
        QRect sr = QtopiaApplication::desktop()->screenGeometry(screen);
        cr.moveTopLeft(QPoint(cr.left() - sr.left(), cr.top() - sr.top()));

        if (dpMode == Tile) {
            p->drawTiledPixmap( cr.left(), cr.top(), cr.width(), cr.height(), bg );
        } else {
            QPoint off((cr.width()-bg.width())/2 + cr.x(), (cr.height()-bg.height())/2 + cr.y());
            cr.translate(-off.x(), -off.y());
            cr &= QRect(0, 0, bg.width(), bg.height());
            p->drawPixmap(cr.topLeft()+off, bg, cr);
        }
    }
}

void HomeScreenImagePlugin::rightsExpired( const QDrmContent &content )
{
    Q_UNUSED( content );

    QSettings config("Trolltech","qpe");
    config.beginGroup("HomeScreen");

    int screen = QApplication::desktop()->screenNumber(themedView);
    switch (screen) {
    case 0:
        config.setValue("HomeScreenPicture", QString());
        break;
    case 1:
        config.setValue("SecondaryHomeScreenPicture", QString());
        break;
    }

    config.sync(); // need to flush the config info first
    QtopiaChannel::send("QPE/System", "applyStyle()");
    QtopiaChannel::send("QPE/System", "applyStyleSplash()");
}

