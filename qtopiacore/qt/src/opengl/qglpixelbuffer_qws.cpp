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

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"
#include <GLES/egl.h>

#include <qimage.h>
#include <private/qgl_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#define QGL_PBUFFER_ATTRIBS_SIZE    32

static void qt_pbuffer_format_to_attribs(const QGLFormat& f, EGLint *configAttribs)
{
    int cfg = 0;
    configAttribs[cfg++] = EGL_RED_SIZE;
    configAttribs[cfg++] = f.redBufferSize() == -1 ? 1 : f.redBufferSize();
    configAttribs[cfg++] = EGL_GREEN_SIZE;
    configAttribs[cfg++] = f.greenBufferSize() == -1 ? 1 : f.greenBufferSize();
    configAttribs[cfg++] = EGL_BLUE_SIZE;
    configAttribs[cfg++] = f.blueBufferSize() == -1 ? 1 : f.blueBufferSize();
    if (f.alpha()) {
        configAttribs[cfg++] = EGL_ALPHA_SIZE;
        configAttribs[cfg++] = f.alphaBufferSize() == -1 ? 1 : f.alphaBufferSize();
    }
    if (f.depth()) {
        configAttribs[cfg++] = EGL_DEPTH_SIZE;
        configAttribs[cfg++] = f.depthBufferSize() == -1 ? 1 : f.depthBufferSize();
    }
    configAttribs[cfg++] = EGL_SURFACE_TYPE;
    configAttribs[cfg++] = EGL_PBUFFER_BIT;
    configAttribs[cfg++] = EGL_NONE;
}

bool QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
    // Choose an appropriate configuration on the display.
    dpy = qt_qgl_egl_display();
    if (dpy == EGL_NO_DISPLAY)
        return false;
    EGLint attribs[QGL_PBUFFER_ATTRIBS_SIZE];
    qt_pbuffer_format_to_attribs(f, attribs);
    EGLint matching = 0;
    if (!eglChooseConfig(dpy, attribs, &config, 1, &matching) || matching < 1) {
        qWarning("QGLPixelBufferPrivate::init(): Could not find a suitable EGL configuration");
        return false;
    }

    // Retrieve the actual format properties.
    EGLint value = 0;
    eglGetConfigAttrib(dpy, config, EGL_RED_SIZE, &value);
    format.setRedBufferSize(value);
    eglGetConfigAttrib(dpy, config, EGL_GREEN_SIZE, &value);
    format.setGreenBufferSize(value);
    eglGetConfigAttrib(dpy, config, EGL_BLUE_SIZE, &value);
    format.setBlueBufferSize(value);
    eglGetConfigAttrib(dpy, config, EGL_ALPHA_SIZE, &value);
    format.setAlpha(value != 0);
    if (format.alpha())
        format.setAlphaBufferSize(value);
    eglGetConfigAttrib(dpy, config, EGL_DEPTH_SIZE, &value);
    format.setDepth(value != 0);
    if (format.depth())
        format.setDepthBufferSize(value);
    if (eglGetConfigAttrib(dpy, config, EGL_LEVEL, &value))
        format.setPlane(value);
    eglGetConfigAttrib(dpy, config, EGL_SAMPLE_BUFFERS, &value);
    format.setSampleBuffers(value != 0);
    if (format.sampleBuffers()) {
        eglGetConfigAttrib(dpy, config, EGL_SAMPLES, &value);
        format.setSamples(value);
    }
    eglGetConfigAttrib(dpy, config, EGL_STENCIL_SIZE, &value);
    format.setStencil(value != 0);
    if (format.stencil())
        format.setStencilBufferSize(value);

    // Create the attributes needed for the pbuffer.
    int cfg = 0;
    attribs[cfg++] = EGL_WIDTH;
    attribs[cfg++] = size.width();
    attribs[cfg++] = EGL_HEIGHT;
    attribs[cfg++] = size.height();
    attribs[cfg++] = EGL_NONE;

    // Create the pbuffer surface.
    pbuf = eglCreatePbufferSurface(dpy, config, attribs);
    if (pbuf == EGL_NO_SURFACE) {
        qWarning("QGLPixelBufferPrivate::init(): Unable to create EGL pbuffer surface");
        return false;
    }

    // Create a new context for the configuration.
    EGLContext shareContext = 0;
    if (shareWidget && shareWidget->d_func()->glcx)
        shareContext = shareWidget->d_func()->glcx->d_func()->cx;
    ctx = 0;
    if (shareContext) {
        ctx = eglCreateContext(dpy, config, shareContext, 0);
        if (!ctx)
            qWarning("QGLPixelBufferPrivate::init(): Could not share context");
    }
    if (!ctx) {
        ctx = eglCreateContext(dpy, config, 0, 0);
        if (!ctx) {
            qWarning("QGLPixelBufferPrivate::init(): Unable to create EGL context");
            eglDestroySurface(dpy, pbuf);
            pbuf = 0;
            return false;
        }
    }

    return true;
}

bool QGLPixelBufferPrivate::cleanup()
{
    eglDestroySurface(dpy, pbuf);
    return true;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
    Q_UNUSED(texture_id);
    return false;
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}


GLuint QGLPixelBuffer::generateDynamicTexture() const
{
    return 0;
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
    EGLDisplay dpy = qt_qgl_egl_display();
    if (dpy == EGL_NO_DISPLAY)
        return false;

    // See if we have at least 1 configuration that matches the default format.
    EGLint attribs[QGL_PBUFFER_ATTRIBS_SIZE];
    qt_pbuffer_format_to_attribs(QGLFormat::defaultFormat(), attribs);
    EGLConfig config = 0;
    EGLint matching = 0;
    if (!eglChooseConfig(dpy, attribs, &config, 1, &matching) || matching < 1)
        return false;

    return true;
}

QT_END_NAMESPACE
