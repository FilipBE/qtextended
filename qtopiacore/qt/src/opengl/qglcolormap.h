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

#ifndef QGLCOLORMAP_H
#define QGLCOLORMAP_H

#include <QtGui/qcolor.h>
#include <QtCore/qvector.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(OpenGL)

class Q_OPENGL_EXPORT QGLColormap
{
public:
    QGLColormap();
    QGLColormap(const QGLColormap &);
    ~QGLColormap();

    QGLColormap &operator=(const QGLColormap &);

    bool   isEmpty() const;
    int    size() const;
    void   detach();

    void   setEntries(int count, const QRgb * colors, int base = 0);
    void   setEntry(int idx, QRgb color);
    void   setEntry(int idx, const QColor & color);
    QRgb   entryRgb(int idx) const;
    QColor entryColor(int idx) const;
    int    find(QRgb color) const;
    int    findNearest(QRgb color) const;

protected:
    Qt::HANDLE handle() { return d ? d->cmapHandle : 0; }
    void setHandle(Qt::HANDLE ahandle) { d->cmapHandle = ahandle; }

private:
    struct QGLColormapData {
        QBasicAtomicInt ref;
        QVector<QRgb> *cells;
        Qt::HANDLE cmapHandle;
    };

    QGLColormapData *d;
    static struct QGLColormapData shared_null;
    static void cleanup(QGLColormapData *x);
    void detach_helper();

    friend class QGLWidget;
    friend class QGLWidgetPrivate;
};

inline void QGLColormap::detach()
{
    if (d->ref != 1)
        detach_helper();
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QGLCOLORMAP_H
