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

#ifndef QMEDIAMENU_H
#define QMEDIAMENU_H

#include <QtGui>

#include <QMediaList>


class QAbstractMediaMenuItemPrivate;

class QTOPIAMEDIA_EXPORT QAbstractMediaMenuItem : public QObject
{
    Q_OBJECT
public:
    QAbstractMediaMenuItem(QMediaList::Roles role, QMediaList* data);
    QAbstractMediaMenuItem(QMediaList::Roles role,QString filter, QMediaList* data);
    QAbstractMediaMenuItem(QIcon* icon, QString text, QMediaList* data);
    QAbstractMediaMenuItem();

    virtual bool  execute() = 0;
    virtual QSize size()    = 0;
    virtual void  paint(QPainter* painter, const QStyleOptionViewItem& option) = 0;

    void add(QAbstractMediaMenuItem* item);
    void remove(QAbstractMediaMenuItem* item);

    void setData(QMediaList* l);
    void setLevel(int l);

    int                  level();
    QIcon*               icon();
    QString              text();
    const QString&       filter();
    QMediaList*          data();
    QMediaList::Roles    displayRole();

    QAbstractMediaMenuItem*         prev;
    QList<QAbstractMediaMenuItem*>  menu;

signals:
    void dataChanged();

private:
    QAbstractMediaMenuItemPrivate*  d;
};


class QMediaMenuItemPrivate;

class QTOPIAMEDIA_EXPORT QMediaMenuItem : public QAbstractMediaMenuItem
{
    Q_OBJECT
public:
    QMediaMenuItem(QMediaList::Roles role, QMediaList* data);
    QMediaMenuItem(QMediaList::Roles role,QString filter, QMediaList* data);
    QMediaMenuItem(QIcon* icon, QString text, QMediaList* data);
    QMediaMenuItem();

    bool  execute();
    QSize size();
    void  paint(QPainter* painter, const QStyleOptionViewItem& option);

    void filterBy(QString str);

private:
    QMediaMenuItemPrivate* dd;
};


class QMediaMenuPrivate;
class QMediaMenuDelegatePrivate;

class QTOPIAMEDIA_EXPORT QMediaMenu : public QListView
{
    Q_OBJECT
public:
    QMediaMenu(QWidget* parent = 0);
    ~QMediaMenu();

    void add(QMediaMenuItem* item);
    void remove(QMediaMenuItem* item);

    QMediaMenuItem* current();

    void resetMenu();

signals:
    void playlist(const QMediaPlaylist& plist);

public slots:
    void next();
    void prev();

    void selection(const QModelIndex& index);

    void refreshData();
private:
    QMediaMenuPrivate         *d;
    QMediaMenuDelegatePrivate *delegate;
};

#endif
