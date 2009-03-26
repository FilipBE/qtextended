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

#ifndef QTHEMEIMAGEITEM_H
#define QTHEMEIMAGEITEM_H

#include <qtopiaglobal.h>
#include <qthemeitem.h>

#ifdef THEME_EDITOR
#include "qthemeimageitemeditor.h"
#include <QXmlStreamWriter>
#endif
class QThemeImageItemPrivate;
class QThemeItemAttribute;

class QTOPIATHEMING_EXPORT QThemeImageItem : public QThemeItem
{
#ifdef THEME_EDITOR
    friend class QThemeImageItemEditor;
    friend class QThemeItem;
#endif
public:
    QThemeImageItem(QThemeItem *parent = 0);
    ~QThemeImageItem();

    enum { Type = ThemeItemType + 6 };
    virtual int type() const;

    void setImage(const QPixmap&, const QString &state = QString());

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void reloadBuffer();
    virtual void saveAttributes(QXmlStreamWriter &writer);
#endif

protected:
#ifdef THEME_EDITOR
    QThemeImageItemPrivate *imgD(){return d;}
#endif

    QPixmap image(const QString &state = QString());
    void setImage(const QString& path, const QString &state = QString());

    int alpha(const QString &state = QString());
    void setAlpha(int alpha, const QString &state = QString());

    QColor color(const QString &state = QString());
    void setColor(const QString& color, const QString &state = QString());

    QStringList states();
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    virtual void loadAttributes(QXmlStreamReader &reader);
    virtual void constructionComplete();
    void replaceColor(QImage &image, const QColor &before, const QColor &after);
    void resizeImages(qreal width, qreal height);
    QString findImageFile(const QString &filename);
    static void colorizeImage(QImage &img, const QColor &col, int alpha, bool blendColor);
    QThemeItemAttribute *source();

protected:
    QThemeImageItemPrivate *d;
};

#endif
