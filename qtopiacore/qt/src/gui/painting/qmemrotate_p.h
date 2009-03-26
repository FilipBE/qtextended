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

#ifndef QMEMROTATE_P_H
#define QMEMROTATE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qdrawhelper_p.h"

QT_BEGIN_NAMESPACE

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif
#endif

#ifdef Q_WS_QWS
#define Q_GUI_QWS_EXPORT Q_GUI_EXPORT
#else
#define Q_GUI_QWS_EXPORT
#endif

#define QT_DECL_MEMROTATE(srctype, desttype)                            \
    void Q_GUI_QWS_EXPORT qt_memrotate90(const srctype*, int, int, int, desttype*, int); \
    void Q_GUI_QWS_EXPORT qt_memrotate180(const srctype*, int, int, int, desttype*, int); \
    void Q_GUI_QWS_EXPORT qt_memrotate270(const srctype*, int, int, int, desttype*, int)

QT_DECL_MEMROTATE(quint32, quint32);
QT_DECL_MEMROTATE(quint32, quint16);
QT_DECL_MEMROTATE(quint16, quint32);
QT_DECL_MEMROTATE(quint16, quint16);
QT_DECL_MEMROTATE(quint24, quint24);
QT_DECL_MEMROTATE(quint32, quint24);
QT_DECL_MEMROTATE(quint32, quint18);
QT_DECL_MEMROTATE(quint32, quint8);
QT_DECL_MEMROTATE(quint16, quint8);
QT_DECL_MEMROTATE(quint8, quint8);
#ifdef QT_QWS_DEPTH_GENERIC
QT_DECL_MEMROTATE(quint32, qrgb_generic16);
QT_DECL_MEMROTATE(quint16, qrgb_generic16);
#endif

#undef QT_DECL_MEMROTATE

QT_END_NAMESPACE

#endif // QMEMROTATE_P_H
