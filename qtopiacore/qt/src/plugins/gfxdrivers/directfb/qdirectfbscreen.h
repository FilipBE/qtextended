/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QDIRECTFBSCREEN_H
#define QDIRECTFBSCREEN_H

#include <QtGui/qscreen_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#include <directfb.h>

#define Q_DIRECTFB_VERSION ((DIRECTFB_MAJOR_VERSION << 16) | (DIRECTFB_MINOR_VERION << 8) | DIRECTFB_MICRO_VERSION)

class QDirectFBScreenPrivate;

class Q_GUI_EXPORT QDirectFBScreen : public QScreen
{
public:
    QDirectFBScreen(int display_id);
    ~QDirectFBScreen();

    bool connect(const QString &displaySpec);
    void disconnect();
    bool initDevice();
    void shutdownDevice();

    void exposeRegion(QRegion r, int changing);
    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void scroll(const QRegion &region, const QPoint &offset);
    void solidFill(const QColor &color, const QRegion &region);

    void setMode(int width, int height, int depth);
    void blank(bool on);

    QWSWindowSurface* createSurface(QWidget *widget) const;
    QWSWindowSurface* createSurface(const QString &key) const;

    static inline QDirectFBScreen* instance() {
        Q_ASSERT(QScreen::instance()->classId() == QScreen::DirectFBClass);
        return static_cast<QDirectFBScreen*>(QScreen::instance());
    }

    IDirectFB* dfb();
    IDirectFBSurface* dfbSurface();
#ifndef QT_NO_DIRECTFB_LAYER
    IDirectFBDisplayLayer* dfbDisplayLayer();
#endif

    static int depth(DFBSurfacePixelFormat format);

    static DFBSurfacePixelFormat getSurfacePixelFormat(const QImage &image);
    static DFBSurfaceDescription getSurfaceDescription(const QImage &image);
    static DFBSurfaceDescription getSurfaceDescription(const uint *buffer,
                                                       int length);
    static QImage::Format getImageFormat(DFBSurfacePixelFormat  format);

#ifndef QT_NO_DIRECTFB_PALETTE
    static void setSurfaceColorTable(IDirectFBSurface *surface,
                                     const QImage &image);
#endif

private:
    void compose(const QRegion &r);
    void blit(IDirectFBSurface *src, const QPoint &topLeft,
              const QRegion &region);

    QDirectFBScreenPrivate *d_ptr;
};

QT_END_HEADER

#endif // QDIRECTFBSCREEN_H
