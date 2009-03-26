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

#ifndef E2_BAR_H
#define E2_BAR_H

#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QPair>
#include "e2_frames.h"

class E2Menu;
class E2Button : public QWidget
{
Q_OBJECT
public:
    E2Button(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~E2Button();

    void setText(const QString &);
    void setPixmap(const QPixmap &);
    void setMenu(E2Menu *menu, bool remainDepressed = false);
    void setEnabled(bool);

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private slots:
    void menuClosing();

private:
    void popupMenu();

    bool m_enabled;
    QString m_text;
    QPixmap m_pixmap;
    QPixmap m_fillBrush;
    QPixmap m_fillBrushPressed;
    E2Menu *m_menu;
    bool m_menuRemainDepressed;
    bool m_isPressed;
    bool m_menuUp;
};

class E2Bar : public QWidget
{
Q_OBJECT
public:
    E2Bar(QWidget *parent = 0, Qt::WFlags flags = 0);

    void addButton(E2Button *, int width);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void relayout();

    int m_layWidth;
    QPixmap m_fillBrush;
    QList<QPair<E2Button *, int> > m_buttons;
    bool m_buttonsStale;
};

class E2MenuPrivate;
class E2Menu : public E2PopupFrame
{
Q_OBJECT
public:
    E2Menu(QWidget *parent = 0, Qt::WFlags flags = 0);

    void replaceItem(int, const QString &);
    void addItem(const QString &);
    void addSeparator();

signals:
    void itemClicked(int);
    void closing();

private:
    E2MenuPrivate *d;
};

#endif
