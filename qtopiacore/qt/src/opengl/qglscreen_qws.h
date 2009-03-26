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

#ifndef QSCREENEGL_P_H
#define QSCREENEGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QScreenEGL class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/QScreen>
#include <QtOpenGL/qgl.h>
#include <GLES/egl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(OpenGL)

class QGLScreenPrivate;

class Q_OPENGL_EXPORT QGLScreenSurfaceFunctions
{
public:
    virtual bool createNativeWindow(QWidget *widget, NativeWindowType *native);
    virtual bool createNativePixmap(QPixmap *pixmap, NativePixmapType *native);
    virtual bool createNativeImage(QImage *image, NativePixmapType *native);
};

class Q_OPENGL_EXPORT QGLScreen : public QScreen
{
    Q_DECLARE_PRIVATE(QGLScreen)
public:
    QGLScreen(int displayId);
    virtual ~QGLScreen();

    enum Option
    {
        NoOptions       = 0,
        NativeWindows   = 1,
        NativePixmaps   = 2,
        NativeImages    = 4,
        Overlays        = 8
    };
    Q_DECLARE_FLAGS(Options, Option)

    QGLScreen::Options options() const;

    virtual bool chooseContext(QGLContext *context, const QGLContext *shareContext);
    virtual bool hasOpenGL() = 0;

    QGLScreenSurfaceFunctions *surfaceFunctions() const;

protected:
    void setOptions(QGLScreen::Options value);
    void setSurfaceFunctions(QGLScreenSurfaceFunctions *functions);

private:
    QGLScreenPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLScreen::Options)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSCREENEGL_P_H
