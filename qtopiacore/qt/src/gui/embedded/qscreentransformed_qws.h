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

#ifndef QSCREENTRANSFORMED_QWS_H
#define QSCREENTRANSFORMED_QWS_H

#include <QtGui/qscreenproxy_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_TRANSFORMED

class QTransformedScreenPrivate;

class Q_AUTOTEST_EXPORT QTransformedScreen : public QProxyScreen
{
public:
    explicit QTransformedScreen(int display_id);
    ~QTransformedScreen();

    enum Transformation { None, Rot90, Rot180, Rot270 };

    void setTransformation(Transformation t);
    Transformation transformation() const;
    int transformOrientation() const;

    QSize mapToDevice(const QSize &s) const;
    QSize mapFromDevice(const QSize &s) const;

    QPoint mapToDevice(const QPoint &, const QSize &) const;
    QPoint mapFromDevice(const QPoint &, const QSize &) const;

    QRect mapToDevice(const QRect &, const QSize &) const;
    QRect mapFromDevice(const QRect &, const QSize &) const;

    QRegion mapToDevice(const QRegion &, const QSize &) const;
    QRegion mapFromDevice(const QRegion &, const QSize &) const;

    bool connect(const QString &displaySpec);

    bool isTransformed() const { return transformation() != None; }

    void exposeRegion(QRegion region, int changing);
    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);
    void setDirty(const QRect&);

    QRegion region() const;

private:
    friend class QTransformedScreenPrivate;
    QTransformedScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_TRANSFORMED

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSCREENTRANSFORMED_QWS_H
