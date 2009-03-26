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

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"
#include <AGL/agl.h>

#include <qimage.h>
#include <private/qgl_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#endif

static int nearest_gl_texture_size(int v)
{
    int n = 0, last = 0;
    for (int s = 0; s < 32; ++s) {
        if (((v>>s) & 1) == 1) {
            ++n;
            last = s;
        }
    }
    if (n > 1)
        return 1 << (last+1);
    return 1 << last;
}

bool QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    GLint attribs[40], i=0;
    attribs[i++] = AGL_RGBA;
    attribs[i++] = AGL_BUFFER_SIZE;
    attribs[i++] = 32;
    attribs[i++] = AGL_LEVEL;
    attribs[i++] = f.plane();
    if (f.redBufferSize() != -1) {
        attribs[i++] = AGL_RED_SIZE;
        attribs[i++] = f.redBufferSize();
    }
    if (f.greenBufferSize() != -1) {
        attribs[i++] = AGL_GREEN_SIZE;
        attribs[i++] = f.greenBufferSize();
    }
    if (f.blueBufferSize() != -1) {
        attribs[i++] = AGL_BLUE_SIZE;
        attribs[i++] = f.blueBufferSize();
    }
    if (f.stereo())
        attribs[i++] = AGL_STEREO;
    if (f.alpha()) {
        attribs[i++] = AGL_ALPHA_SIZE;
        attribs[i++] = f.alphaBufferSize() == -1 ? 8 : f.alphaBufferSize();
    }
    if (f.stencil()) {
        attribs[i++] = AGL_STENCIL_SIZE;
        attribs[i++] = f.stencilBufferSize() == -1 ? 8 : f.stencilBufferSize();
    }
    if (f.depth()) {
        attribs[i++] = AGL_DEPTH_SIZE;
        attribs[i++] = f.depthBufferSize() == -1 ? 32 : f.depthBufferSize();
    }
    if (f.accum()) {
        attribs[i++] = AGL_ACCUM_RED_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_BLUE_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_GREEN_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_ALPHA_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
    }

    if (f.sampleBuffers()) {
        attribs[i++] = AGL_SAMPLE_BUFFERS_ARB;
        attribs[i++] = 1;
        attribs[i++] = AGL_SAMPLES_ARB;
        attribs[i++] = f.samples() == -1 ? 4 : f.samples();
    }
    attribs[i] = AGL_NONE;

    AGLPixelFormat format = aglChoosePixelFormat(0, 0, attribs);
    if (!format) {
	qWarning("QGLPixelBuffer: Unable to find a pixel format (AGL error %d).",
		 (int) aglGetError());
        return false;
    }

    GLint res;
    aglDescribePixelFormat(format, AGL_LEVEL, &res);
    this->format.setPlane(res);
    aglDescribePixelFormat(format, AGL_DOUBLEBUFFER, &res);
    this->format.setDoubleBuffer(res);
    aglDescribePixelFormat(format, AGL_DEPTH_SIZE, &res);
    this->format.setDepth(res);
    if (this->format.depth())
	this->format.setDepthBufferSize(res);
    aglDescribePixelFormat(format, AGL_RGBA, &res);
    this->format.setRgba(res);
    aglDescribePixelFormat(format, AGL_RED_SIZE, &res);
    this->format.setRedBufferSize(res);
    aglDescribePixelFormat(format, AGL_GREEN_SIZE, &res);
    this->format.setGreenBufferSize(res);
    aglDescribePixelFormat(format, AGL_BLUE_SIZE, &res);
    this->format.setBlueBufferSize(res);
    aglDescribePixelFormat(format, AGL_ALPHA_SIZE, &res);
    this->format.setAlpha(res);
    if (this->format.alpha())
	this->format.setAlphaBufferSize(res);
    aglDescribePixelFormat(format, AGL_ACCUM_RED_SIZE, &res);
    this->format.setAccum(res);
    if (this->format.accum())
	this->format.setAccumBufferSize(res);
    aglDescribePixelFormat(format, AGL_STENCIL_SIZE, &res);
    this->format.setStencil(res);
    if (this->format.stencil())
	this->format.setStencilBufferSize(res);
    aglDescribePixelFormat(format, AGL_STEREO, &res);
    this->format.setStereo(res);
    aglDescribePixelFormat(format, AGL_SAMPLE_BUFFERS_ARB, &res);
    this->format.setSampleBuffers(res);
    if (this->format.sampleBuffers()) {
        aglDescribePixelFormat(format, AGL_SAMPLES_ARB, &res);
        this->format.setSamples(res);
    }

    AGLContext share = 0;
    if (shareWidget)
	share = share_ctx = static_cast<AGLContext>(shareWidget->d_func()->glcx->d_func()->cx);
    ctx = aglCreateContext(format, share);
    if (!ctx) {
	qWarning("QGLPixelBuffer: Unable to create a context (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    GLenum target = GL_TEXTURE_2D;

    if ((QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
        && (size.width() != nearest_gl_texture_size(size.width())
            || size.height() != nearest_gl_texture_size(size.height())))
    {
        target = GL_TEXTURE_RECTANGLE_EXT;
    }

    if (!aglCreatePBuffer(size.width(), size.height(), target, GL_RGBA, 0, &pbuf)) {
	qWarning("QGLPixelBuffer: Unable to create a pbuffer (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    if (!aglSetPBuffer(ctx, pbuf, 0, 0, 0)) {
	qWarning("QGLPixelBuffer: Unable to set pbuffer (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    aglDestroyPixelFormat(format);
    return true;
#else
    Q_UNUSED(size);
    Q_UNUSED(f);
    Q_UNUSED(shareWidget);
    return false;
#endif
}

bool QGLPixelBufferPrivate::cleanup()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    aglDestroyPBuffer(pbuf);
    return true;
#endif
    return false;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);
    if (d->invalid || !d->share_ctx)
	return false;
    aglSetCurrentContext(d->share_ctx);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    return true;
#else
    Q_UNUSED(texture_id);
    return false;
#endif
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
bool QGLPixelBuffer::bindToDynamicTexture(QMacCompatGLuint texture_id)
{
    return bindToDynamicTexture(GLuint(texture_id));
}
#endif

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

GLuint QGLPixelBuffer::generateDynamicTexture() const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(const QGLPixelBuffer);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    glBindTexture(GL_TEXTURE_2D, texture); // updates texture target
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
#else
    return 0;
#endif
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    return true;
#else
    return false;
#endif
}

QT_END_NAMESPACE
