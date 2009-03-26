/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef OUBLIETTE_H
#define OUBLIETTE_H

#include <QtGui/QWidget>
#include <oublietteplan.h>
#include "cursor.h"

QT_BEGIN_NAMESPACE

class QPaintEvent;
class QKeyEvent;
class QListWidgetItem;
struct ItemEffect;

class Oubliette : public QWidget
{
    Q_OBJECT
public:
    Oubliette();
    ~Oubliette();
    const Cursor &character() const { return m_character; }
    QPoint visualCursorPos() const;

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void timerEvent(QTimerEvent *);
    void showEvent(QShowEvent *);

private slots:
    void showInventoryItem(QListWidgetItem *lwi);
    void showInstructions();
    void showVictory();

private:
    void showInventory();
    void animateItem(const Item *item, const QPoint &pos);
    bool tryMove(const QPoint &newPos);
    void updateExplored();
    void paintOubliette(QPainter *p, const QRect &rect);
    void fillTile(QPainter *p, int x, int y, Tile tile);
    inline void fillTile(QPainter *p, const QPoint &point, Tile tile)
    { fillTile(p, point.x(), point.y(), tile); }

signals:
    void characterMoved(const QPoint &pt);

private:
    OubliettePlan m_oubliettePlan;
    Cursor m_character;
    QPoint m_oldCursorPosition;
    int m_currentLevel;
    QList<ItemEffect *> m_effects;
    int m_timerID;
};

QT_END_NAMESPACE

#endif
