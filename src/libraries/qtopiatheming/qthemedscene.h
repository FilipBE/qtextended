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

#ifndef QTHEMEDSCENE_H
#define QTHEMEDSCENE_H

#include <QGraphicsScene>

#include <qtopiaglobal.h>

class QThemeItem;

class QThemedView;

class QExpressionEvaluator;
class QThemedScenePrivate;

#ifdef THEME_EDITOR
class QGraphicsSceneMouseEvent;
#endif

class QTOPIATHEMING_EXPORT QThemedScene : public QGraphicsScene
{
    Q_OBJECT

public:
    QThemedScene(QObject *parent = 0);
    ~QThemedScene();

    bool load(const QString &filename);
    void layout(QThemeItem *item = 0);

    QThemedView *themedView() const;
    void registerExpression(QThemeItem *item, QExpressionEvaluator *expr);
#ifdef THEME_EDITOR
    void save(const QString &filename);
    int nextId();
    bool editorLock();
    bool showInvisible();
    bool visualAids();
    void deleteItem(QThemeItem *item);
    void undeleteItem(QThemeItem *item);
    void createDefault();//Overwrites the current scene with a blank but valid scene
public slots:
    void setShowInvisible(bool set);
    void setVisualAids(bool set);
    void clearHandles(QThemeItem* item = 0);
    void makeHandles(QThemeItem* item);
    void highlight(QGraphicsItem* item);
    bool highlighted(QThemeItem* item);

    void itemClicked(QThemeItem* item) {
        emit itemSelected(item);
    }
    void changePending(QThemeItem* item) {
        emit itemChangePending(item);
    }
    void changed(QThemeItem *item) {
        emit itemChanged(item);
    }
    void treeChanged() {
        emit treeChange();
    }
    void setEditorLock(bool set);
signals:
    void itemSelected(QThemeItem* item);
    void itemChangePending(QThemeItem* item);
    void itemChanged(QThemeItem* item);
    void itemHighlighted(QThemeItem *item);
    void treeChange();//General Purpose reset model/view
    void wantSelectMenu(QGraphicsSceneMouseEvent *event);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    QThemeItem *deadParent();

#endif

private slots:
    void notifyExpressionChanged();
    void expressionDestroyed(QObject *obj);

private:
    QThemedScenePrivate *d;
};

#endif
