/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QX11INFO_X11_H
#define QX11INFO_X11_H

#include <QtCore/qnamespace.h>

typedef struct _XDisplay Display;

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

struct QX11InfoData;
class QPaintDevice;
class QApplicationPrivate;
class QX11InfoPrivate;

class Q_GUI_EXPORT QX11Info
{
public:
    QX11Info();
    ~QX11Info();
    QX11Info(const QX11Info &other);
    QX11Info &operator=(const QX11Info &other);

    static Display *display();
    static const char *appClass();
    int screen() const;
    int depth() const;
    int cells() const;
    Qt::HANDLE colormap() const;
    bool defaultColormap() const;
    void *visual() const;
    bool defaultVisual() const;

    static int appScreen();
    static int appDepth(int screen = -1);
    static int appCells(int screen = -1);
    static Qt::HANDLE appColormap(int screen = -1);
    static void *appVisual(int screen = -1);
    static Qt::HANDLE appRootWindow(int screen = -1);
    static bool appDefaultColormap(int screen = -1);
    static bool appDefaultVisual(int screen = -1);
    static int appDpiX(int screen = -1);
    static int appDpiY(int screen = -1);
    static void setAppDpiX(int screen, int dpi);
    static void setAppDpiY(int screen, int dpi);
    static unsigned long appTime();
    static unsigned long appUserTime();
    static void setAppTime(unsigned long time);
    static void setAppUserTime(unsigned long time);
    static bool isCompositingManagerRunning();

protected:
    void copyX11Data(const QPaintDevice *);
    void cloneX11Data(const QPaintDevice *);
    void setX11Data(const QX11InfoData *);
    QX11InfoData* getX11Data(bool def = false) const;

    QX11InfoData *x11data;

    friend class QX11PaintEngine;
    friend class QPixmap;
    friend class QX11PixmapData;
    friend class QWidget;
    friend class QWidgetPrivate;
    friend class QGLWidget;
    friend void qt_init(QApplicationPrivate *priv, int, Display *display, Qt::HANDLE visual,
                        Qt::HANDLE colormap);
    friend void qt_cleanup();
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QX11INFO_X11_H
