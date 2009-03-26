/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QTHEMEITEM_P_H
#define QTHEMEITEM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QThemeItem>
#include <QList>
#include <QMap>
#ifdef THEME_EDITOR
#include <QPointer>
#endif

struct ThemeMessageData {
    ThemeMessageData() {
    }

    ThemeMessageData(const ThemeMessageData& other) {
        (*this) = other;
    }

    ThemeMessageData& operator=(const ThemeMessageData& other) {
        channel = other.channel;
        msg = other.msg;
        type = other.type;
        variant = other.variant;
        return *this;
    }

    QString channel;
    QString msg;
    QString type;
    QVariant variant;
};

class QThemeItemPrivate
{

public:
    QThemeItemPrivate()
            : mode(QThemeItem::Rect)
            , activeExpression(0)
            , activeAttribute(false)
            , isActive(true)
            , transient(false)
            , dataExpression(false)
            , loaded(false)
            , state("default")
            , types(0)
#ifdef THEME_EDITOR
            , beforeChange()
            , beforeParent(0)
            , changeId(-1)
            , id(0x00) //Valid ID's are a multiple of 0x10, see QThemeItemUndo for further detail
#endif
    {
        for (int i = 0; i < 4; i++)
            unit[i] = QThemeItem::Pixel;
    }

    QRectF boundingRect;
    QRectF sr;
    QThemeItem::Mode mode;
    QThemeItem::Unit unit[4];
    QString name;
    QExpressionEvaluator* activeExpression;
    QString activeAttribute;
    bool isActive;
    bool transient;
    QList<ThemeMessageData> messages;
    bool dataExpression;
    static int count;
    QString vsPath;
    bool loaded;
    bool handleMove;
    QMap<QString, QString> onClickAtts;
    QString state;
    quint64 types;
#ifdef THEME_EDITOR
    QPointF oldPos;
    QPointF mouseDelta;
    QPointF snapDelta;
    QPointer<QThemeItemEditor> editor;
    QString beforeChange;
    QGraphicsItem *beforeParent;
    int changeId;
    int id;
    QList<QRectF> snapRects;
#endif
};

#endif
