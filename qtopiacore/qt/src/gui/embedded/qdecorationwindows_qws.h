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

#ifndef QDECORATIONWINDOWS_QWS_H
#define QDECORATIONWINDOWS_QWS_H

#include <QtGui/qdecorationdefault_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_WINDOWS) || defined(QT_PLUGIN)

class Q_GUI_EXPORT QDecorationWindows : public QDecorationDefault
{
public:
    QDecorationWindows();
    virtual ~QDecorationWindows();

    QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
               DecorationState state = Normal);

protected:
    void paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                     DecorationState state, const QPalette &pal);
    const char **xpmForRegion(int reg);
};

#endif // QT_NO_QWS_DECORATION_WINDOWS

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECORATIONWINDOWS_QWS_H
