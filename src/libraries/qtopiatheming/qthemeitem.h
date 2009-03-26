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

#ifndef QTHEMEITEM_H
#define QTHEMEITEM_H

#include <qtopiaglobal.h>
#include <QGraphicsItem>
#include <QColor>
#include <QPalette>
#include <QDebug>

class QXmlStreamReader;
class QXmlStreamAttributes;
class QExpressionEvaluator;
class QThemedView;
class QThemedScene;
class QThemeItemPrivate;
class QThemeGroupItem;
class QThemeLayoutItem;
struct ThemeMessageData;

#ifdef THEME_EDITOR
class QXmlStreamWriter;
#include "qthemeitemeditor.h"
#endif

const struct {
    const char *name;
    QPalette::ColorRole role;
} colorTable[] = {
    { "Window", QPalette::Window },
    { "Background", QPalette::Window },
    { "WindowText", QPalette::WindowText },
    { "Foreground", QPalette::WindowText },
    { "Button", QPalette::Button },
    { "Light", QPalette::Light },
    { "Midlight", QPalette::Midlight },
    { "Dark", QPalette::Dark },
    { "Mid", QPalette::Mid },
    { "Text", QPalette::Text },
    { "BrightText", QPalette::BrightText },
    { "ButtonText", QPalette::ButtonText },
    { "Base", QPalette::Base },
    { "Shadow", QPalette::Shadow },
    { "Highlight", QPalette::Highlight },
    { "HighlightedText", QPalette::HighlightedText },
    { "AlternateBase", QPalette::AlternateBase },
    { "Link", QPalette::Link },
    { "LinkVisited", QPalette::LinkVisited },
    { 0, QPalette::NColorRoles }
};

class QTOPIATHEMING_EXPORT QThemeItem : public QGraphicsItem
{
    enum Mode {
        Rect = 0,
        Coords
    };

    enum Unit {
        Pixel = 0,
        Percent,
        Point
    };

public:
    QThemeItem(QThemeItem *parent = 0);
    virtual ~QThemeItem();

    static QThemeItem *themeItem(QGraphicsItem *item);
    QString name() const;
    void load(QXmlStreamReader &reader);
    virtual void layout();

    QRectF boundingRect() const;

    enum {  ThemeItemType = UserType + 3224,
            ThemeItemUserType = ThemeItemType + 32,
            ThemeItemMaxUserType = ThemeItemType + 64,
            Type = ThemeItemType};
    virtual int type() const;
    quint64 types() const;
    void registerType(int);

    bool isActive() const;
    virtual void setActive(bool active);

    QString state() const;
    void setState(const QString &state);

    QThemedView *themedView() const;

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void save(QXmlStreamWriter &writer);
    virtual void saveChildren(QXmlStreamWriter &writer);
    virtual void saveAttributes(QXmlStreamWriter &writer);
#endif
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    virtual void loadChildren(QXmlStreamReader &reader);
    virtual void loadAttributes(QXmlStreamReader &reader);
    virtual void resize(qreal width, qreal height);
    void parseRect(const QStringRef &string);
    int parseAlignment(const QString &val);
    virtual void pressed(QGraphicsSceneMouseEvent *event);
    virtual void released(QGraphicsSceneMouseEvent *event);

    QMap<QString, QString> parseSubAttributes(const QString &subatts) const;
    ThemeMessageData parseMessage(const QString &message, bool *ok);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QThemeItem* itemFromType(const QString &type);
    qreal resolveUnit(qreal value, qreal bound, Unit unit) const;
    QThemedScene *themedScene() const;
    QThemeLayoutItem* parentLayout() const;
    QThemeGroupItem* parentGroup() const;

    static bool isExpression(const QString &s);
    static QString strippedExpression(const QString &s);

    virtual void expressionChanged(QExpressionEvaluator *);
    virtual void constructionComplete();

    QExpressionEvaluator *createExpression(const QString &data);
    QVariant getExpressionResult(QExpressionEvaluator *expression, const QVariant::Type &type);
    QColor colorFromString(const QString s) const;
    void setValueSpacePath(const QString &path);
    QString fullValueSpacePath() const;
    QString valueSpacePath() const;

#ifdef THEME_EDITOR
    bool invisibleThemeItem(QThemeItem* item);
    virtual void reset();
    void relayout();
    void aboutToChange(int id = -1);
    bool finishedChange(int id = -1);
    qreal convertUnit(qreal value, qreal pixelBound, Unit from, Unit to) const;
    void normalize();//Sets to use rect mode
    QString textOfUnit(QThemeItem::Unit unit);
    QString textOfType(const QThemeItem* item);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
#endif

private:
    bool filter(const QXmlStreamAttributes &atts);

private:
    QThemeItemPrivate *d;
    friend class QThemeItemPrivate;
    friend class QThemedScene;
    friend class QThemeLayoutItem;
    friend class QThemePageItem;
    friend class QThemeImageItem;
    friend class QThemeTextItem;
    friend class QThemeGroupItem;
    friend class QThemeWidgetItem;
    friend class QThemeListModelEntry;
    friend class QThemeTemplateInstanceItem;
#ifdef THEME_EDITOR
    friend class QThemeItemEditor;
    friend class ThemeSelectionBox;
    friend class QThemeItemUndo;
#endif

};

template<class T, bool possible>
struct qthemeitem_cast_helper2
{
    static inline T* cast(QThemeItem *item)
    {
        if (item && (item->types() & (1 << (int(T::Type) - QThemeItem::ThemeItemType - 1))))
            return static_cast<T*>(item);
        return 0;
    }
    static inline T* cast(const QThemeItem *item)
    {
        if (item && (item->types() & (1 << (int(T::Type) - QThemeItem::ThemeItemType - 1))))
            return static_cast<T*>(item);
        return 0;
    }
};
template<class T>
struct qthemeitem_cast_helper2<T, false>
{
    static inline T* cast(QThemeItem *item)
    {
        if(int(T::Type) == QThemeItem::ThemeItemType)
            return static_cast<T*>(item);
        return 0;
    }
    static inline T* cast(const QThemeItem *item)
    {
        if(int(T::Type) == QThemeItem::ThemeItemType)
            return static_cast<T*>(item);
        return 0;
    }
};

template<class T>
struct qthemeitem_cast_helper
{
};
template<class T>
struct qthemeitem_cast_helper<T*>
{
    static inline T* cast(QThemeItem *item)
    {
        // The second helper ensures we don't try to generate bad code
        // (when T::Type == QThemeItem::ThemeItemType)
        return qthemeitem_cast_helper2<T, (int(T::Type) > QThemeItem::ThemeItemType &&
                                           int(T::Type) <= QThemeItem::ThemeItemMaxUserType)>::cast(item);
    }
    static inline T* cast(const QThemeItem *item)
    {
        // The second helper ensures we don't try to generate bad code
        // (when T::Type == QThemeItem::ThemeItemType)
        return qthemeitem_cast_helper2<T*, (int(T::Type) > QThemeItem::ThemeItemType &&
                                            int(T::Type) <= QThemeItem::ThemeItemMaxUserType)>::cast(item);
    }
};

template <class T>
inline T qthemeitem_cast(QThemeItem *item)
{
    // The first helper ensures we only call this when T is a pointer
    return qthemeitem_cast_helper<T>::cast(item);
}

template <class T>
inline T qthemeitem_cast(const QThemeItem *item)
{
    // The first helper ensures we only call this when T is a pointer
    return qthemeitem_cast_helper<T>::cast(item);
}

template <class T> inline T qthemeitem_cast(QGraphicsItem *item)
{
    return item?((item->type() >= QThemeItem::ThemeItemType
            && item->type() <= QThemeItem::ThemeItemMaxUserType)
        ? qthemeitem_cast<T>(static_cast<QThemeItem*>(item)) : 0):0;
}

template <class T> inline T qthemeitem_cast(const QGraphicsItem *item)
{
    return item?((item->type() >= QThemeItem::ThemeItemType
            && item->type() <= QThemeItem::ThemeItemMaxUserType)
        ? qthemeitem_cast<T>(static_cast<QThemeItem*>(item)) : 0):0;
}

#endif
