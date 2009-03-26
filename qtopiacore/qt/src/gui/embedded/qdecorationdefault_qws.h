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

#ifndef QDECORATIONDEFAULT_QWS_H
#define QDECORATIONDEFAULT_QWS_H

#include <QtGui/qdecoration_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)

#define CORNER_GRAB 16
#define BORDER_WIDTH  4
#define BOTTOM_BORDER_WIDTH BORDER_WIDTH

class Q_GUI_EXPORT QDecorationDefault : public QDecoration
{
public:
    QDecorationDefault();
    virtual ~QDecorationDefault();

    virtual QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    virtual bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
                       DecorationState state = Normal);

protected:
    virtual int titleBarHeight(const QWidget *widget);

    virtual void paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                             DecorationState state, const QPalette &pal);
    virtual QPixmap pixmapFor(const QWidget *widget, int decorationRegion, int &xoff, int &yoff);
    virtual const char **xpmForRegion(int region);

    QString windowTitleFor(const QWidget *widget) const;

    int menu_width;
    int help_width;
    int close_width;
    int minimize_width;
    int maximize_width;
    int normalize_width;

private:
    static QPixmap *staticHelpPixmap;
    static QPixmap *staticMenuPixmap;
    static QPixmap *staticClosePixmap;
    static QPixmap *staticMinimizePixmap;
    static QPixmap *staticMaximizePixmap;
    static QPixmap *staticNormalizePixmap;

};


QT_END_NAMESPACE
#endif // QT_NO_QWS_DECORATION_DEFAULT
QT_END_HEADER

#endif // QDECORATIONDEFAULT_QWS_H
