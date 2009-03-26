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

#ifndef QDIRECTPAINTER_QWS_H
#define QDIRECTPAINTER_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qregion.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainterPrivate;
class QWSEmbedEvent;

class Q_GUI_EXPORT QDirectPainter : public QObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirectPainter)
public:

    enum SurfaceFlag { NonReserved = 0,
                       Reserved = 1,
                       ReservedSynchronous = 3 };

    explicit QDirectPainter(QObject *parentObject = 0, SurfaceFlag flag = NonReserved);
    ~QDirectPainter();

    void setRegion(const QRegion&);
    QRegion requestedRegion() const;
    QRegion allocatedRegion() const;

    void setGeometry(const QRect&);
    QRect geometry() const;

    WId winId() const;
    virtual void regionChanged(const QRegion &exposedRegion);

    void startPainting(bool lockDisplay = true);
    void endPainting();
    void endPainting(const QRegion &region);
    void flush(const QRegion &region);

    void raise();
    void lower();


    static QRegion reserveRegion(const QRegion&);
    static QRegion reservedRegion();
    static QRegion region() { return reservedRegion(); }

    static uchar* frameBuffer();
    static int screenDepth();
    static int screenWidth();
    static int screenHeight();
    static int linestep();

    static void lock();
    static void unlock();
private:
    friend  void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc, int type);
    friend void qt_directpainter_embedevent(QDirectPainter*, const QWSEmbedEvent*);
};

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDIRECTPAINTER_QWS_H
