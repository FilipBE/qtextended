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

#ifndef QTHEMELISTITEM_H
#define QTHEMELISTITEM_H

#include <QThemeWidgetItem>
#include <QAbstractListModel>
#include <QListView>
#include <QThemedView>

class QThemeListItemPrivate;
class QThemeListModel;

class QTOPIATHEMING_EXPORT QThemeListItem : public QThemeWidgetItem
{

public:
    QThemeListItem(QThemeItem *parent = 0);
    ~QThemeListItem();

    enum { Type = ThemeItemType + 11 };
    int type() const;

    QListView* listView() const;
    virtual void setWidget(QWidget *widget);

    void setModel(QThemeListModel *model);
    QThemeListModel *model() const;

private:
    QThemeListItemPrivate *d;
};

/***************************************************************************/

class QThemeListModel;
struct QThemeListModelEntryPrivate;
class QThemeTemplateInstanceItem;
class QTOPIATHEMING_EXPORT QThemeListModelEntry
{
public:
    explicit QThemeListModelEntry(QThemeListModel *model);
    virtual ~QThemeListModelEntry();

    QString uid();

    virtual QString type() const = 0;

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key);

    QThemeListModel *model() const;
    QThemeTemplateInstanceItem *templateInstance();

private:
    void getTemplateInstance();
    QString valuespacePath();
    QThemeListModelEntryPrivate *d;
};
Q_DECLARE_METATYPE(QThemeListModelEntry*)

/***************************************************************************/

struct QThemeListModelPrivate;
class QTOPIATHEMING_EXPORT QThemeListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QThemeListModel(QObject *parent, QThemeListItem *li, QThemedView *view);
    virtual ~QThemeListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QThemeListItem *listItem() const;
    QThemedView *themedView() const;

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex entryIndex(const QThemeListModelEntry *entry) const;

    QThemeListModelEntry *themeListModelEntry(const QModelIndex &index) const;

    void addEntry(QThemeListModelEntry *item);
    void removeEntry(const QModelIndex &index);
    void clear();

    void triggerUpdate();

protected:
    QList<QThemeListModelEntry*> items() const;

private:
    QThemeListModelPrivate *d;
};

/***************************************************************************/

struct QThemeListDelegatePrivate;
class QThemeListDelegate : public QItemDelegate
{
public:
    QThemeListDelegate(QListView *listview, QThemedView *view, QObject *parent = 0);
    virtual ~QThemeListDelegate();

    void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int height(QThemeListModelEntry* entry, const QModelIndex& index) const;

private:
    QThemeListDelegatePrivate* d;
};

#endif
