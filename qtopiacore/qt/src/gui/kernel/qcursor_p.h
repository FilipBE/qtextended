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

#ifndef QCURSOR_P_H
#define QCURSOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qatomic.h"
#include "QtCore/qglobal.h"
#include "QtCore/qnamespace.h"
#include "QtGui/qpixmap.h"

# if defined (Q_WS_MAC)
#  include "private/qt_mac_p.h"
# elif defined(Q_WS_X11)
#  include "private/qt_x11_p.h"
# elif defined(Q_WS_WIN)
#  include "QtCore/qt_windows.h"
#endif

QT_BEGIN_NAMESPACE

#if defined (Q_WS_MAC)
class QMacAnimateCursor;
#endif

class QBitmap;
struct QCursorData {
    QCursorData(Qt::CursorShape s = Qt::ArrowCursor);
    ~QCursorData();

    static void initialize();
    static void cleanup();

    QAtomicInt ref;
    Qt::CursorShape cshape;
    QBitmap  *bm, *bmm;
    QPixmap pixmap;
    short     hx, hy;
#if defined (Q_WS_MAC)
    int mId;
#elif defined(Q_WS_QWS)
    int id;
#endif
#if defined (Q_WS_WIN)
    HCURSOR hcurs;
#elif defined (Q_WS_X11)
    XColor fg, bg;
    Cursor hcurs;
    Pixmap pm, pmm;
#elif defined (Q_WS_MAC)
    enum { TYPE_None, TYPE_ImageCursor, TYPE_ThemeCursor } type;
    union {
        struct {
            uint my_cursor:1;
            void *nscursor;
        } cp;
        struct {
            QMacAnimateCursor *anim;
            ThemeCursor curs;
        } tc;
    } curs;
    void initCursorFromBitmap();
    void initCursorFromPixmap();
#endif
    static bool initialized;
    void update();
    static QCursorData *setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY);
};

QT_END_NAMESPACE

#endif // QCURSOR_P_H
