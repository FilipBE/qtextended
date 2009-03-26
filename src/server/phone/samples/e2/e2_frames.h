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

#ifndef E2_FRAMES_H
#define E2_FRAMES_H

#include <QWidget>
#include <QItemDelegate>
#include <QListWidget>

void e2Center(QWidget *);

class E2Bar;
class E2PopupFrame : public QWidget
{
Q_OBJECT
public:
    E2PopupFrame(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
};

class E2TitleFrame : public QWidget
{
Q_OBJECT
public:
    enum TitleType { GradientTitle, NormalTitle, NoTitle };
    E2TitleFrame(TitleType title = GradientTitle,
                   QWidget *parent = 0,
                   Qt::WFlags flags = 0);

    void setTitleText(const QString &);
    E2Bar *bar() const;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

private:
    void doLayout();

    TitleType m_titleType;
    QString m_title;
    E2Bar *m_bar;
    QPixmap m_titleFill;
};

class E2ListDelegate : public QItemDelegate
{
Q_OBJECT
public:
    E2ListDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
                       const QModelIndex & index ) const;
};

class E2ListWidget : public QListWidget
{
Q_OBJECT
public:
    E2ListWidget(QWidget *parent = 0);
};

#endif
