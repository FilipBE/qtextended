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

#include "qgl_p.h"

QT_BEGIN_NAMESPACE

bool qt_resolve_framebufferobject_extensions(QGLContext *ctx)
{
    if (glIsRenderbufferEXT != 0)
        return true;

    if (ctx == 0) {
        qWarning("QGLFramebufferObject: Unable to resolve framebuffer object extensions -"
                 " make sure there is a current context when creating the framebuffer object.");
        return false;
    }

    glIsRenderbufferEXT = (_glIsRenderbufferEXT) ctx->getProcAddress(QLatin1String("glIsRenderbufferEXT"));
    glBindRenderbufferEXT = (_glBindRenderbufferEXT) ctx->getProcAddress(QLatin1String("glBindRenderbufferEXT"));
    glDeleteRenderbuffersEXT = (_glDeleteRenderbuffersEXT) ctx->getProcAddress(QLatin1String("glDeleteRenderbuffersEXT"));
    glGenRenderbuffersEXT = (_glGenRenderbuffersEXT) ctx->getProcAddress(QLatin1String("glGenRenderbuffersEXT"));
    glRenderbufferStorageEXT = (_glRenderbufferStorageEXT) ctx->getProcAddress(QLatin1String("glRenderbufferStorageEXT"));
    glGetRenderbufferParameterivEXT =
        (_glGetRenderbufferParameterivEXT) ctx->getProcAddress(QLatin1String("glGetRenderbufferParameterivEXT"));
    glIsFramebufferEXT = (_glIsFramebufferEXT) ctx->getProcAddress(QLatin1String("glIsFramebufferEXT"));
    glBindFramebufferEXT = (_glBindFramebufferEXT) ctx->getProcAddress(QLatin1String("glBindFramebufferEXT"));
    glDeleteFramebuffersEXT = (_glDeleteFramebuffersEXT) ctx->getProcAddress(QLatin1String("glDeleteFramebuffersEXT"));
    glGenFramebuffersEXT = (_glGenFramebuffersEXT) ctx->getProcAddress(QLatin1String("glGenFramebuffersEXT"));
    glCheckFramebufferStatusEXT = (_glCheckFramebufferStatusEXT) ctx->getProcAddress(QLatin1String("glCheckFramebufferStatusEXT"));
    glFramebufferTexture1DEXT = (_glFramebufferTexture1DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture1DEXT"));
    glFramebufferTexture2DEXT = (_glFramebufferTexture2DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture2DEXT"));
    glFramebufferTexture3DEXT = (_glFramebufferTexture3DEXT) ctx->getProcAddress(QLatin1String("glFramebufferTexture3DEXT"));
    glFramebufferRenderbufferEXT = (_glFramebufferRenderbufferEXT) ctx->getProcAddress(QLatin1String("glFramebufferRenderbufferEXT"));
    glGetFramebufferAttachmentParameterivEXT =
        (_glGetFramebufferAttachmentParameterivEXT) ctx->getProcAddress(QLatin1String("glGetFramebufferAttachmentParameterivEXT"));
    glGenerateMipmapEXT = (_glGenerateMipmapEXT) ctx->getProcAddress(QLatin1String("glGenerateMipmapEXT"));
    return glIsRenderbufferEXT;
}

bool qt_resolve_version_1_3_functions(QGLContext *ctx)
{
    if (glMultiTexCoord4f != 0)
        return true;

    QGLContext cx(QGLFormat::defaultFormat());
    glMultiTexCoord4f = (_glMultiTexCoord4f) ctx->getProcAddress(QLatin1String("glMultiTexCoord4f"));
    glActiveTexture = (_glActiveTexture) ctx->getProcAddress(QLatin1String("glActiveTexture"));

    return glMultiTexCoord4f && glActiveTexture;
}

bool qt_resolve_stencil_face_extension(QGLContext *ctx)
{
    if (glActiveStencilFaceEXT != 0)
        return true;

    QGLContext cx(QGLFormat::defaultFormat());
    glActiveStencilFaceEXT = (_glActiveStencilFaceEXT) ctx->getProcAddress(QLatin1String("glActiveStencilFaceEXT"));

    return glActiveStencilFaceEXT;
}

bool qt_resolve_frag_program_extensions(QGLContext *ctx)
{
    if (glProgramStringARB != 0)
        return true;

    // ARB_fragment_program
    glProgramStringARB = (_glProgramStringARB) ctx->getProcAddress(QLatin1String("glProgramStringARB"));
    glBindProgramARB = (_glBindProgramARB) ctx->getProcAddress(QLatin1String("glBindProgramARB"));
    glDeleteProgramsARB = (_glDeleteProgramsARB) ctx->getProcAddress(QLatin1String("glDeleteProgramsARB"));
    glGenProgramsARB = (_glGenProgramsARB) ctx->getProcAddress(QLatin1String("glGenProgramsARB"));
    glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) ctx->getProcAddress(QLatin1String("glProgramLocalParameter4fvARB"));

    return glProgramStringARB
        && glBindProgramARB
        && glDeleteProgramsARB
        && glGenProgramsARB
        && glProgramLocalParameter4fvARB;
}

QT_END_NAMESPACE
