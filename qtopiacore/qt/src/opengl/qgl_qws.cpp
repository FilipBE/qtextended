/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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
** http://www.gnu.org/copyleft/gpl.html.
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

#include "qgl.h"

#if defined(Q_WS_QWS)
#include <GLES/egl.h>
#include <GLES/gl.h>
#include <qdirectpainter_qws.h>

#include <qglscreen_qws.h>
#include <qscreenproxy_qws.h>
#include <private/qglwindowsurface_qws_p.h>

#endif
#include <private/qbackingstore_p.h>
#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdesktopwidget.h>
#include <qdebug.h>
#include <qvarlengtharray.h>


#define Q_USE_QEGL
//#define Q_USE_DIRECTPAINTER

// this one for full QScreen implemented using EGL/GLES:
//#define Q_USE_EGLWINDOWSURFACE
#ifdef Q_USE_QEGL
#include "qegl_qws_p.h"

#ifdef Q_USE_EGLWINDOWSURFACE
#include "private/qglwindowsurface_qws_p.h"
#endif
#endif

QT_BEGIN_NAMESPACE

static QGLScreen *glScreenForDevice(QPaintDevice *device)
{
    QScreen *screen = qt_screen;
    if (screen->classId() == QScreen::MultiClass) {
        int screenNumber;
        if (device && device->devType() == QInternal::Widget)
            screenNumber = qApp->desktop()->screenNumber(static_cast<QWidget *>(device));
        else
            screenNumber = 0;
        screen = screen->subScreens()[screenNumber];
    }
    while (screen->classId() == QScreen::ProxyClass) {
        screen = static_cast<QProxyScreen *>(screen)->screen();
    }
    if (screen->classId() == QScreen::GLClass)
        return static_cast<QGLScreen *>(screen);
    else
        return 0;
}

/*****************************************************************************
  QOpenGL debug facilities
 *****************************************************************************/
//#define DEBUG_OPENGL_REGION_UPDATE

bool QGLFormat::hasOpenGL()
{
    return true;
}


bool QGLFormat::hasOpenGLOverlays()
{
    QGLScreen *glScreen = glScreenForDevice(0);
    if (glScreen)
        return (glScreen->options() & QGLScreen::Overlays);
    else
        return false;
}

#define QT_EGL_CHECK(x)                          \
    if (!(x)) {                                  \
        EGLint err = eglGetError();              \
        printf("egl " #x " failure %x!\n", err); \
    }                                            \

#define QT_EGL_ERR(txt)                         \
    do {                                        \
        EGLint err = eglGetError();             \
        if (err != EGL_SUCCESS)                 \
            printf( txt " failure %x!\n", err); \
    } while (0)

EGLDisplay qt_qgl_egl_display()
{
    static EGLDisplay dpy = EGL_NO_DISPLAY;
    if (dpy == EGL_NO_DISPLAY) {
        dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (dpy == EGL_NO_DISPLAY) {
            qWarning("qt_qgl_egl_display(): Cannot open EGL display");
            return EGL_NO_DISPLAY;
        }
        if (!eglInitialize(dpy, NULL, NULL)) {
            qWarning("qt_qgl_egl_display(): Cannot initialize EGL display");
            return EGL_NO_DISPLAY;
        }
    }
    return dpy;
}

bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    Q_D(QGLContext);

    if (!device())
        return false;

    // Find the QGLScreen for this paint device.
    QGLScreen *glScreen = glScreenForDevice(device());
    if (!glScreen) {
        qWarning("QGLContext::chooseContext(): The screen is not a QGLScreen");
        return false;
    }

    // Determine if we should use QGLScreen::chooseContext() because
    // we cannot otherwise create a native drawable.  This behavior
    // is provided for backwards compatibility with Qt 4.3.
    int devType = device()->devType();
    QGLScreen::Option option;
    if (devType == QInternal::Pixmap)
        option = QGLScreen::NativePixmaps;
    else if (devType == QInternal::Image)
        option = QGLScreen::NativeImages;
    else
        option = QGLScreen::NativeWindows;
    if ((glScreen->options() & option) == 0)
        return glScreen->chooseContext(this, shareContext);

    d->cx = 0;

    // Get the display and initialize it.
    d->dpy = qt_qgl_egl_display();
    if (d->dpy == EGL_NO_DISPLAY)
        return false;

    // Construct the configuration we need for this surface.
    EGLint configAttribs[64];
    int cfg = 0;
    QGLFormat fmt = format();
    EGLint red, green, blue, alpha;
    switch (glScreen->pixelFormat()) {
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
            red = green = blue = 8; alpha = 0; break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            red = green = blue = alpha = 8; break;
        case QImage::Format_RGB16:
            red = 5; green = 6; blue = 5; alpha = 0; break;
        case QImage::Format_ARGB8565_Premultiplied:
            red = 5; green = 6; blue = 5; alpha = 8; break;
        case QImage::Format_RGB666:
            red = green = blue = 6; alpha = 0; break;
        case QImage::Format_ARGB6666_Premultiplied:
            red = green = blue = alpha = 6; break;
        case QImage::Format_RGB555:
            red = green = blue = 5; alpha = 0; break;
        case QImage::Format_ARGB8555_Premultiplied:
            red = green = blue = 5; alpha = 8; break;
        case QImage::Format_RGB444:
            red = green = blue = 4; alpha = 0; break;
        case QImage::Format_ARGB4444_Premultiplied:
            red = green = blue = alpha = 4; break;
        default:
            qWarning("QGLContext::chooseContext(): Unsupported pixel format");
            return false;
    }
    configAttribs[cfg++] = EGL_RED_SIZE;
    configAttribs[cfg++] = red;
    configAttribs[cfg++] = EGL_GREEN_SIZE;
    configAttribs[cfg++] = green;
    configAttribs[cfg++] = EGL_BLUE_SIZE;
    configAttribs[cfg++] = blue;
    configAttribs[cfg++] = EGL_ALPHA_SIZE;
    configAttribs[cfg++] = alpha;
    if (fmt.depth()) {
        configAttribs[cfg++] = EGL_DEPTH_SIZE;
        configAttribs[cfg++] = fmt.depthBufferSize() == -1 ? 1 : fmt.depthBufferSize();
    }
    configAttribs[cfg++] = EGL_LEVEL;
    configAttribs[cfg++] = fmt.plane();
    configAttribs[cfg++] = EGL_SURFACE_TYPE;
    if (devType == QInternal::Pixmap || devType == QInternal::Image)
        configAttribs[cfg++] = EGL_PIXMAP_BIT;
    else
        configAttribs[cfg++] = EGL_WINDOW_BIT;
    configAttribs[cfg] = EGL_NONE;

    EGLint matching = 0;
    if (!eglChooseConfig(d->dpy, configAttribs, &d->config, 1, &matching) || matching < 1) {
        qWarning("QGLContext::chooseContext(): Could not find a suitable EGL configuration");
        return false;
    }

    // Inform the context about the actual format properties.
    EGLint value = 0;
    eglGetConfigAttrib(d->dpy, d->config, EGL_RED_SIZE, &value);
    d->glFormat.setRedBufferSize(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_GREEN_SIZE, &value);
    d->glFormat.setGreenBufferSize(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_BLUE_SIZE, &value);
    d->glFormat.setBlueBufferSize(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_ALPHA_SIZE, &value);
    d->glFormat.setAlpha(value != 0);
    if (d->glFormat.alpha())
        d->glFormat.setAlphaBufferSize(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_DEPTH_SIZE, &value);
    d->glFormat.setDepth(value != 0);
    if (d->glFormat.depth())
        d->glFormat.setDepthBufferSize(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_LEVEL, &value);
    d->glFormat.setPlane(value);
    eglGetConfigAttrib(d->dpy, d->config, EGL_SAMPLE_BUFFERS, &value);
    d->glFormat.setSampleBuffers(value != 0);
    if (d->glFormat.sampleBuffers()) {
        eglGetConfigAttrib(d->dpy, d->config, EGL_SAMPLES, &value);
        d->glFormat.setSamples(value);
    }
    eglGetConfigAttrib(d->dpy, d->config, EGL_STENCIL_SIZE, &value);
    d->glFormat.setStencil(value != 0);
    if (d->glFormat.stencil())
        d->glFormat.setStencilBufferSize(value);

    // Create the native drawable for this paint device.
    NativePixmapType pixmapDrawable = 0;
    NativeWindowType windowDrawable = 0;
    bool ok;
    if (devType == QInternal::Pixmap) {
        ok = glScreen->surfaceFunctions()
                ->createNativePixmap(static_cast<QPixmap *>(device()), &pixmapDrawable);
    } else if (devType == QInternal::Image) {
        ok = glScreen->surfaceFunctions()
                ->createNativeImage(static_cast<QImage *>(device()), &pixmapDrawable);
    } else {
        ok = glScreen->surfaceFunctions()
                ->createNativeWindow(static_cast<QWidget *>(device()), &windowDrawable);
    }
    if (!ok) {
        qWarning("QGLContext::chooseContext(): Cannot create the native EGL drawable");
        return false;
    }

    // Create a new context for the configuration.
    if (shareContext && shareContext->d_func()->cx) {
        d->cx = eglCreateContext(d->dpy, d->config, shareContext->d_func()->cx, 0);
        if (!d->cx) {
            qWarning("QGLContext::chooseContext(): Could not share context");
            shareContext = 0;
        }
    }
    if (!d->cx) {
        d->cx = eglCreateContext(d->dpy, d->config, 0, 0);
        if (!d->cx) {
            qWarning("QGLContext::chooseContext(): Unable to create EGL context");
            return false;
        }
    }
    if (shareContext && shareContext->d_func()->cx) {
        QGLContext *share = const_cast<QGLContext *>(shareContext);
        d->sharing = true;
        share->d_func()->sharing = true;
    }

#if defined(EGL_VERSION_1_1)
    if (fmt.swapInterval() != -1)
        eglSwapInterval(d->dpy, fmt.swapInterval());
#endif

    // Create the EGL surface to draw into.
    if (devType == QInternal::Widget)
        d->surface = eglCreateWindowSurface(d->dpy, d->config, windowDrawable, 0);
    else
        d->surface = eglCreatePixmapSurface(d->dpy, d->config, pixmapDrawable, 0);
    if (!d->surface) {
        qWarning("QGLContext::chooseContext(): Unable to create EGL surface");
        eglDestroyContext(d->dpy, d->cx);
        d->cx = 0;
        return false;
    }

    return true;
}


void QGLContext::reset()
{
    Q_D(QGLContext);
    if (!d->valid)
        return;
    doneCurrent();
    if (d->cx)
        eglDestroyContext(d->dpy, d->cx);
    d->crWin = false;
    d->cx = 0;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
    qgl_share_reg()->removeShare(this);
}

void QGLContext::makeCurrent()
{
    Q_D(QGLContext);
    if(!d->valid) {
        qWarning("QGLContext::makeCurrent(): Cannot make invalid context current");
        return;
    }

    bool ok = eglMakeCurrent(d->dpy, d->surface, d->surface, d->cx);
    if (!ok) {
        EGLint err = eglGetError();
        qWarning("QGLContext::makeCurrent(): Failed %x.", err);
    }
    if (ok) {
        if (!qgl_context_storage.hasLocalData() && QThread::currentThread())
            qgl_context_storage.setLocalData(new QGLThreadContext);
        if (qgl_context_storage.hasLocalData())
            qgl_context_storage.localData()->context = this;
        currentCtx = this;
    }
}

void QGLContext::doneCurrent()
{
    Q_D(QGLContext);
    eglMakeCurrent(d->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    QT_EGL_ERR("QGLContext::doneCurrent");
    if (qgl_context_storage.hasLocalData())
        qgl_context_storage.localData()->context = 0;
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    Q_D(const QGLContext);
    if(!d->valid)
        return;

    eglSwapBuffers(d->dpy, d->surface);
    QT_EGL_ERR("QGLContext::swapBuffers");
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor(0, 0, 0);                // Invalid color
}

uint QGLContext::colorIndex(const QColor &c) const
{
    //### color index doesn't work on egl
    Q_UNUSED(c);
    return 0;
}

void QGLContext::generateFontDisplayLists(const QFont & fnt, int listBase)
{
    Q_UNUSED(fnt);
    Q_UNUSED(listBase);
}

void *QGLContext::getProcAddress(const QString &proc) const
{
    return (void*)eglGetProcAddress(reinterpret_cast<const char *>(proc.toLatin1().data()));
}

#ifdef Q_USE_DIRECTPAINTER
class QGLDirectPainter : public QDirectPainter
{
public:
    QGLDirectPainter(QGLContextPrivate *cd, QGLWidgetPrivate *wd) :glxPriv(cd), glwPriv(wd), image(0), nativePix(0) {}
    ~QGLDirectPainter() {
    }
    void regionChanged(const QRegion &);
    void render();

    QRect geom;
    QGLContextPrivate *glxPriv;
    QGLWidgetPrivate *glwPriv;
    QImage *image;
    NativeWindowType nativePix;
};


void QGLDirectPainter::regionChanged(const QRegion&)
{
#ifdef Q_USE_QEGL
    if (geometry() != geom) {
        geom = geometry();
        uchar *fbp = QDirectPainter::frameBuffer() + geom.top() * QDirectPainter::linestep()
                     + ((QDirectPainter::screenDepth()+7)/8) * geom.left();

        QImage *oldImage = image;
        NativeWindowType oldPix = nativePix;
        image = new QImage(fbp, geom.width(), geom.height(), QDirectPainter::screenDepth(),
                           QDirectPainter::linestep(), 0, 0, QImage::IgnoreEndian);
#if 0 // debug
        static int i = 0;
        i = (i+13) %255;
        for (int y = 0; y < image->height(); ++y)
            for (int x = 0; x < image->width(); ++x)
                image->setPixel(x, y, 0xff4000 + i);
#endif
        QT_EGL_ERR("before eglDestroySurface");
        if (glxPriv->surface != EGL_NO_SURFACE) {
            eglDestroySurface(glxPriv->dpy, glxPriv->surface);
            QT_EGL_ERR("eglDestroySurface");
        }
#if 1
        nativePix = QEGL::createNativePixmap(image);
        glxPriv->surface =    eglCreatePixmapSurface(glxPriv->dpy, glxPriv->config, nativePix, 0);////const EGLint *attrib list);
#else

#endif
        QT_EGL_ERR("createEGLSurface");
        glxPriv->valid =  glxPriv->surface != EGL_NO_SURFACE;
        glwPriv->resizeHandler(geom.size());
        delete oldImage;
        QEGL::destroyNativePixmap(oldPix);
    }
#endif
    if (0) {
    QRegion alloc = allocatedRegion();
    int max = 0;
    QRect allocR;

    for (int i=0; i < alloc.rects().count(); ++i) {
        QRect r = alloc.rects().at(i);
        int a = r.width()*r.height();
        if (a  > max) {
            max = a;
            allocR = r;
        }
    }
    allocR.translate(-geom.topLeft());
    glScissor(allocR.left(), geom.height() - allocR.bottom(), allocR.width(), allocR.height());

    glwPriv->render(allocR);
    }
}
#else

class QWSGLPrivate
{
public:
    QWSGLPrivate() : img(0), oldbsimg(0), nativePix(0), dirty(true) {}
    QImage *img;
    QImage *oldbsimg;
    NativeWindowType nativePix;
    bool dirty;
};

class QGLDirectPainter : public QWSGLPrivate {}; //###

#endif // Q_USE_DIRECTPAINTER

void QGLWidgetPrivate::render(const QRegion &r)
{
    Q_Q(QGLWidget);
    QPaintEvent e(r);
    q->paintEvent(&e); //### slightly hacky...
}

void QGLWidgetPrivate::resizeHandler(const QSize &s)
{
    Q_Q(QGLWidget);

    q->makeCurrent();
    if (!glcx->initialized())
        q->glInit();

#ifndef Q_USE_EGLWINDOWSURFACE
    eglWaitNative(EGL_CORE_NATIVE_ENGINE);
#endif

    QT_EGL_ERR("QGLWidgetPrivate::resizeHandler");

    q->resizeGL(s.width(), s.height());
}

bool QGLWidget::event(QEvent *e)
{
#if 0 // ??? we have to make sure update() works...
    if (e->type() == QEvent::Paint)
        return true; // We don't paint when the GL widget needs to be painted, but when the directpainter does
#endif
#ifdef Q_USE_EGLWINDOWSURFACE
    return QWidget::event(e); // for EGL/GLES windowsurface do nothing in ::event()
#else

#ifndef Q_USE_DIRECTPAINTER
    if (e->type() == QEvent::Paint) {
        Q_D(QGLWidget);
        QWindowSurface *ws = d->currentWindowSurface();
        Q_ASSERT(ws);
        QImage *bsImage = static_cast<QImage*>(ws->paintDevice());
        if (bsImage
            && (bsImage != d->directPainter->oldbsimg || !d->directPainter->img ||d->directPainter->img->size() != size())) {
            QPoint offs = mapToGlobal(QPoint(0,0)) - window()->frameGeometry().topLeft();
            uchar *fbp = bsImage->bits() + offs.y() * bsImage->bytesPerLine()
                         + ((bsImage->depth()+7)/8) * offs.x();
            QImage *oldImage = d->directPainter->img;
            d->directPainter->img = new QImage(fbp, width(), height(), bsImage->depth(),
                                            bsImage->bytesPerLine(), 0, 0, QImage::IgnoreEndian);

            QGLContextPrivate *glxPriv = d->glcx->d_func();

            if (glxPriv->surface != EGL_NO_SURFACE) {
                eglDestroySurface(glxPriv->dpy, glxPriv->surface);
                QT_EGL_ERR("eglDestroySurface");
            }

            NativeWindowType nativePix = QEGL::createNativePixmap(d->directPainter->img);
            glxPriv->surface = eglCreatePixmapSurface(glxPriv->dpy, glxPriv->config, nativePix, 0);
            glxPriv->valid =  glxPriv->surface != EGL_NO_SURFACE;
            delete oldImage;
            QEGL::destroyNativePixmap(d->directPainter->nativePix);
            d->directPainter->nativePix = nativePix;
        }

    }
#endif
#endif
    return QWidget::event(e);
}

void QGLWidget::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
}


void QGLWidget::resizeEvent(QResizeEvent *)
{
    Q_D(QGLWidget);
//     if (!isValid())
//         return;
#ifdef Q_USE_DIRECTPAINTER
    if (!d->directPainter)
        d->directPainter = new QGLDirectPainter(d->glcx->d_func(), d);
    d->directPainter->setGeometry(geometry());
#else
    if (!d->directPainter)
        d->directPainter = new QGLDirectPainter;
#endif
    //handle overlay
}

const QGLContext* QGLWidget::overlayContext() const
{
    return 0;
}

void QGLWidget::makeOverlayCurrent()
{
    //handle overlay
}

void QGLWidget::updateOverlayGL()
{
    //handle overlay
}

void QGLWidget::setContext(QGLContext *context, const QGLContext* shareContext, bool deleteOldContext)
{
    Q_D(QGLWidget);
    if(context == 0) {
        qWarning("QGLWidget::setContext: Cannot set null context");
        return;
    }

    if(d->glcx)
        d->glcx->doneCurrent();
    QGLContext* oldcx = d->glcx;
    d->glcx = context;
    if(!d->glcx->isValid())
        d->glcx->create(shareContext ? shareContext : oldcx);
    if(deleteOldContext)
        delete oldcx;
}

void QGLWidgetPrivate::init(QGLContext *context, const QGLWidget* shareWidget)
{
    Q_Q(QGLWidget);

    directPainter = 0;

#ifdef Q_USE_EGLWINDOWSURFACE
    QGLScreen *glScreen = glScreenForDevice(q);
    if (glScreen) {
        wsurf = static_cast<QWSGLWindowSurface*>(glScreen->createSurface(q));
        q->setWindowSurface(wsurf);
    }
#endif

    initContext(context, shareWidget);

    if(q->isValid() && glcx->format().hasOverlay()) {
        //no overlay
        qWarning("QtOpenGL ES doesn't currently support overlays");
    }
}

bool QGLWidgetPrivate::renderCxPm(QPixmap*)
{
    return false;
}

void QGLWidgetPrivate::cleanupColormaps()
{
}

const QGLColormap & QGLWidget::colormap() const
{
    return d_func()->cmap;
}

void QGLWidget::setColormap(const QGLColormap &)
{
}

void QGLExtensions::init()
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

#if 0 //### to avoid confusing experimental EGL: don't create two GL widgets
    QGLWidget dmy;
    dmy.makeCurrent();
    init_extensions();
#endif
}

QT_END_NAMESPACE
