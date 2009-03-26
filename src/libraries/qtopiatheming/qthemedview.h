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

#ifndef QTHEMEDVIEW_H
#define QTHEMEDVIEW_H

#include <QGraphicsView>
#include <QList>
#include <themedview.h>
#include <qthemeitem.h>
#include <QThemeWidgetItem>

#include <qtopiaglobal.h>
#ifdef THEME_EDITOR
class QUndoStack;
#endif

class QThemeWidgetItem;
class QThemedViewPrivate;
class QEvent;

class QTOPIATHEMING_EXPORT QThemedView : public QGraphicsView
{
    Q_OBJECT

public:
    QThemedView(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QThemedView();

    bool load(const QString &fileName);
    QThemeItem *findItem(const QString &name) const
    {
        foreach (QGraphicsItem *i, items()) {
            QThemeItem *item = QThemeItem::themeItem(i);
            if (item && item->name() == name) {
                return item;
            }
        }
        return 0;
    }

    QString fileName() const;
    QString themePrefix() const;
    void setThemePrefix(const QString &prefix);
    void itemMouseReleased(QThemeItem *item);
    void itemMousePressed(QThemeItem *item);
    QList<QThemeWidgetItem*> widgets() const {
        QList<QThemeWidgetItem*> list;
        foreach (QGraphicsItem *i, items()) {
            QThemeWidgetItem *widget = qgraphicsitem_cast<QThemeWidgetItem *>(i);
            if (widget)
                list << widget;
        }
        return list;
    }

#ifdef THEME_EDITOR
    void save();
    QUndoStack* undoStack() const;
    QThemedScene *themedScene() const;

    /*  createDefault keeps the filename, but replaces the current content of
        the view with a default, blank, valid view.*/
    void createDefault();
#endif

protected:
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void itemReleased(QThemeItem *item);
    void itemPressed(QThemeItem *item);
    void itemClicked(QThemeItem *item);

private:
    QThemedViewPrivate  *d;
};

#endif
