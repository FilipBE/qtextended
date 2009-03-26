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

#include <QThemeListItem>
#include <QThemeTemplateItem>
#include <QThemedScene>
#include <QPainter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QListView>
#include <QValueSpace>
#include <QUuid>

struct QThemeListItemPrivate {
    QThemeListItemPrivate() : model(0) {}
    QThemeListModel* model;
};

/*!
  \class QThemeListItem
    \inpublicgroup QtBaseModule
  \since 4.4
  \brief The QThemeListItem class provides a list item that you can add to a QThemedView.

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeListItem.
  The \a parent is passed to the base class constructor.
*/
QThemeListItem::QThemeListItem(QThemeItem *parent)
        : QThemeWidgetItem(parent), d(new QThemeListItemPrivate)
{
    QThemeItem::registerType(Type);
    QListView *lv = new QListView;
    lv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidget(lv);
}

/*!
  Destroys the ThemeListItem object.
*/
QThemeListItem::~QThemeListItem()
{
    delete d;
}

/*!
  Returns the ThemeListModel object associated with this ThemeListItem.
*/
QThemeListModel* QThemeListItem::model() const
{
    return d->model;
}

/*!
  Sets \a model to be the ThemeListModel associated with this ThemeListItem.
  If this item currently has a QListView widget set, this model is installed
  immediately on the view using QListView::setModel().
  If a call is made to QWidget::setWidget() in the future, this model
  will be installed in the same way.
*/
void QThemeListItem::setModel(QThemeListModel *model)
{
    if (d->model != model) {
        d->model = model;
        QListView* view = listView();
        if (view != 0 && view->model() != d->model)
            view->setModel(d->model);
    }
}

/*!
  \reimp
*/
int QThemeListItem::type() const
{
    return Type;
}

/*!
  \reimp
  Sets the QListView \a w as this ThemeListItem's widget.
  \a w must be non-zero and inherit QListView, otherwise this function will abort.
  An internal implementation of QItemDelegate is installed on the view to handle
  communication between itself and a ThemeListModel.
  If a ThemeListModel object has already been set using setModel(), it is installed immediately
  using QListView::setModel(). If a call is made to setModel() in the future, that model
  will be installed on this widget in the same way.
*/
void QThemeListItem::setWidget(QWidget *w)
{
    QThemeWidgetItem::setWidget(w);
    Q_ASSERT(widget() == 0 || widget()->inherits("QListView") == true);
    if (!widget())
        return;
    QListView* v = listView();
    v->setItemDelegate(new QThemeListDelegate(v, themedScene()->themedView(), themedScene()->themedView()));
    if (d->model != 0 && v->model() != d->model)
        v->setModel(d->model);
}

/*!
  Returns the QListView for this item.
  Equivalent to qobject_cast<QListView*>(widget())
*/
QListView* QThemeListItem::listView() const
{
    if (!widget())
        return 0;
    Q_ASSERT(widget()->inherits("QListView") == true);
    return qobject_cast<QListView*>(widget());
}

/***************************************************************************/

class QValueSpaceObject;
struct QThemeListModelEntryPrivate {
    QThemeListModelEntryPrivate() : /*vsObject("/"),*/ vsItem("/") {}
    QString uid;
    QThemeListModel* model;
//    QValueSpaceObject vsObject;
    QValueSpaceItem vsItem;
    QThemeTemplateInstanceItem* templateInstance;
};

/*!
  \internal
  \class QThemeListModelEntry
    \inpublicgroup QtBaseModule

  \brief The QThemeListModelEntry class implements a single theme list entry in a QThemeListModel.

  The QThemeListModelEntry has a uid() and a type(), which are used by templateInstance() to associate
  a single template instance to this entry. The QThemeListModelEntry stores this template instance
  internally and can be retrieved using templateInstance().

  This class must be subclassed and the pure virtual type() function reimplemented to return the appropriate
  value.

  \sa QThemeListModel, QThemeTemplateItem, QThemeTemplateInstanceItem
  \ingroup qtopiatheming
  \since 4.4
*/

/*!
  Constructs a QThemeListModelEntry an associates it with the given \a model.
*/
QThemeListModelEntry::QThemeListModelEntry(QThemeListModel* model)
{
    d = new QThemeListModelEntryPrivate;
    d->uid = QUuid::createUuid().toString();
    d->model = model;
    d->templateInstance = 0;
}

/*!
    Destroys the ThemeListModelEntry.
*/
QThemeListModelEntry::~QThemeListModelEntry()
{
    delete d;
}

/*!
    \fn QString QThemeListModelEntry::type() const = 0

    Returns the type of this entry.
    Subclasses must overrride this pure virtual function and return a value that corresponds
    to the name attribute of a template item defined under the associated list item in the themed view XML.
    templateInstance() calls this function to lookup and associate this entry to a ThemeTemplateItem.
    If the value that this function returns changes, either the subclass or the parent ThemeListModel implementation should
    instruct the view to repaint this entry, probably by calling ThemeListModel::triggerUpdate().
    The internal QItemDelegate implementation will call templateInstance() to create and associate a ThemeTemplateInstanceItem
    object with this entry, based on the new return value.
*/

/*!
  Returns a globally unique identifier for this entry.
  The return value of this function is used by templateInstance()
  in order to associate this entry with a theme template instance.
*/
QString QThemeListModelEntry::uid()
{
    return d->uid;
}

/*!
  Sets the given \a value in the valuespace based on the given \a key.
  The actual key set will be templateInstance()->fullVSPath() + \a key.
*/
void QThemeListModelEntry::setValue(const QString &key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
//    d->vsObject.setAttribute(valuespacePath() + key, value);
}

/*!
  Retrives a value from the valuespace based on the given \a key.
  The actual valuespace key used to get the value will be templateInstance()->fullVSPath() + \a key.
*/
QVariant QThemeListModelEntry::value(const QString &key)
{
    return d->vsItem.value(valuespacePath() + key);
}

/*!
  Returns the parent ThemeListModel instance associated with this entry.
*/
QThemeListModel* QThemeListModelEntry::model() const
{
    return d->model;
}

/*!
  \internal
*/
QString QThemeListModelEntry::valuespacePath()
{
    Q_ASSERT(d->model != 0);
    if (!templateInstance())
        return QString();
    return d->templateInstance->fullValueSpacePath();
    return QString("");
}

/*!
  Creates and returns a ThemeTemplateInstanceItem object for this entry.
  This function searches all template items defined under the list item in the themed view XML file
  looking for an item that has a name attribute matching the return value of type().
  When it finds one it calls ThemeTemplateItem::createInstance() passing the value of uid(), and stores the return value in this entry.
  If a template instance item already exists but its name attribute does not match the value currently returned by type(),
  then it is deleted and a new template instance is created.
  If no template can be found for this entry 0 is returned, otherwise a pointer to the associated ThemeTemplateInstanceItem is returned.
  \sa ThemeTemplateInstanceItem
*/
QThemeTemplateInstanceItem* QThemeListModelEntry::templateInstance()
{
    getTemplateInstance();
    return d->templateInstance;
}

void QThemeListModelEntry::getTemplateInstance()
{
    Q_ASSERT(d->model != 0);
    if (d->templateInstance == 0) {
        QThemeTemplateItem* ti = qgraphicsitem_cast<QThemeTemplateItem*>(d->model->themedView()->findItem(/*li,*/ type()));
        if (!ti) {
            qWarning("ThemeListModelEntry::getTemplateInstance() - Cannot find template item with name '%s'", type().toAscii().data());
            return;
        }
        d->templateInstance = ti->createInstance(uid());
        if (!d->templateInstance) {
            qWarning("ThemeListModelEntry::getTemplateInstance() - Could not create template instance.");
            return;
        }
    } else if (d->templateInstance->name() != type()) {
        Q_ASSERT(d->templateInstance->name() != type());
        delete d->templateInstance;
        d->templateInstance = 0;
        getTemplateInstance(); // call again
    } // else the same
}

/***************************************************************************/

struct QThemeListModelPrivate {
    QList<QThemeListModelEntry*> items;
    QThemeListItem* listItem;
    QThemedView* themedView;
};

/*!
    \internal
    \class QThemeListModel
    \inpublicgroup QtBaseModule

    \brief The QThemeListModel class provides a list model that is used for list functionality in Qt Extended theming.

    List functionality in theming is implemented using Qt's model-view architecture and theme templates.

    To use a ThemeListModel you pass in the associated ThemeListItem and ThemedView objects during construction, and then
    populate it with ThemeListModelEntry items using the addEntry() and removeEntry() functions.
    The ThemeListModel installs itself as the model for the ThemeListItem's QListView object during construction.
    From that point on communication between the QListView and the ThemeListModel occurs through Qt's model-view architecture.

    Theme templates are used to describe the visual look of items in the ThemeListModel.
    As a list's items are added dynamically at runtime, they cannot be specified in the themed view XML.
    However, using theme templates, the visual appearance of a list item is defined instead, which is used to draw items as appropriate.
    Instances of these theme templates can be created at anytime using ThemeTemplateItem::createInstance().

    An internal implementation of QItemDelegate called ThemeListDelegate exists to handle communication between the QListView
    of a ThemeListItem, and associated ThemeListModel/ThemeListModelEntry objects.
    It works completely under the hood, and so this information is given only to help the developer understand how the system works.

    The ThemeListDelegate is installed using QListView::setItemDelegate() on the ThemeListItem's QListView. Qt's model-view architecture
    then asks the delegate to paint items in the QListView as required.
    When requested to repaint a particular QModelIndex in the QListView, the ThemeItemDelegate performs the following functions:
    \list
    \o The index is used to look up the associated ThemeListModelEntry through the associated ThemeListModel. The ThemeItemDelegate asserts
        that the return value of QListView::model() is an object that inherits from ThemeListModel.
    \o If no template instance has been created for the ThemeListModelEntry object, a new one is created using by calling ThemeListModelEntry::templateInstance()
        which calls ThemeTemplateItem::createInstance().
    \o If a template instance exists but the value returned by its itemName() function is not equal to the value returned by the ThemeListModelEntry's
        type() function, then the delegate deletes the existing template instance and calls ThemeTemplateItem::templateInstance() to get a new one.
    \o A pointer to the template instance is stored in the ThemeListModelEntry object.
    \o The template instance for the ThemeListModelEntry object is painted to the QListView at the index.
    \endlist

    See the ThemeTemplateItem and ThemeListModelEntry documentation for more information.

    The valuespace is used to communicate data from the system to items in the themed list.
    Normally a valuespace path can be assigned in the themed view XML to items using the vspath attribute of ThemeItem.
    However, as template definitions representing a theme list items' visual appearance are not actual instances,
    their vspath is dynamically determined and set at runtime when they are created using ThemeTemplateItem::createInstance().
    ThemeTemplateItem::createInstance() takes a uid which is actually set to the vspath of the returned ThemeTemplateInstance.
    For template instances created by ThemeListModelEntry::templateInstance() the uid passed is the return value of ThemeListModelEntry::uid(). This acts as a
    unique association between a theme template instance and the ThemeListModelEntry, as well as a unique valuespace path for both to communicate under.
    A call to ThemeListModelEntry::setValue() set keys under this unique vspath.
    For example, given the following themed view XML definition:
    \code
    <list vspath="/UserInterface/MyList/">
        <template name="myListItem">
            <text name="titleMessage">@TitleMessage</text>
        </template>
    </list>
    \endcode

    you could set the text of the element 'titleMessage' with the following:
    \code
    ThemeListModelEntry* myEntry; // assuming that myEntry->type() returns "myListItem"
    ..
    myEntry->setValue("TitleMessage", "Hello, World"); // expands to /UserInterface/MyList/<myEntry->uid()>/TitleMessage
    \endcode

    \sa {Themed View Elements}, ThemeTemplateItem, ThemeTemplateInstanceItem, ThemeListModelEntry
  \ingroup appearance
*/


/*!
  Constructs a ThemeListModel.
  \a parent is passed to QAbstractListModel, \a listItem is the ThemeListItem associated with this model and \a view
  is the ThemedView associated with this model.
  The model is installed on the ThemeListItem \a listItem using the ThemeListItem::setModel() function.
*/
QThemeListModel::QThemeListModel(QObject* parent, QThemeListItem* listItem, QThemedView *view)
        : QAbstractListModel(parent)
{
    d = new QThemeListModelPrivate;
    d->listItem = listItem;
    d->themedView = view;
    Q_ASSERT(listItem != 0);
    listItem->setModel(this);
}

/*!
  Destroys the model. All items in the model are deleted.
*/
QThemeListModel::~QThemeListModel()
{
    clear();
    delete d;
}

/*!
   Returns a QList of all items in the model.
*/
QList<QThemeListModelEntry*> QThemeListModel::items() const
{
    return d->items;
}

/*!
  Returns the ThemedView instance associated with this model.
*/
QThemedView* QThemeListModel::themedView() const
{
    return d->themedView;
}

/*!
   \reimp
   Returns the number of items below \a parent in this model.
*/
int QThemeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

/*!
  Returns the ThemeListItem instance associated with this model.
*/
QThemeListItem* QThemeListModel::listItem() const
{
    return d->listItem;
}

/*!
  \reimp
*/
QVariant QThemeListModel::data(const QModelIndex &index, int) const
{
    QVariant ret;
    if (index.isValid() && (index.row() < d->items.count()))
        ret = QVariant::fromValue(d->items.at(index.row()));
    return ret;
}


/*!
  Returns the index of the item \a entry in the model.
*/
QModelIndex QThemeListModel::entryIndex(const QThemeListModelEntry *entry) const
{
    int idx = d->items.indexOf(const_cast<QThemeListModelEntry*>(entry));
    return index(idx, 0);
}

/*!
  Returns a pointer to the ThemeListModelEntry at \a index.
*/
QThemeListModelEntry* QThemeListModel::themeListModelEntry(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= d->items.count())
        return 0;
    return d->items.at(index.row());
}

/*!
  Adds \a item to the end of this model.
  ThemeListModel takes ownership of \a item and will delete it when it is removed.
*/
void QThemeListModel::addEntry(QThemeListModelEntry *item)
{
    /* calling addEntry transfers ownership of item to this model ie. removeEntry and clear will delete it*/
    beginInsertRows(QModelIndex(), d->items.count(), d->items.count());
    d->items.append(item);
    endInsertRows();
}

/*!
  Removes the item at \a index from the mdoel.
  The item is deleted.
*/
void QThemeListModel::removeEntry(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    QThemeListModelEntry* entry = d->items.takeAt(index.row());
    Q_ASSERT(entry != 0);
    delete entry;
    endRemoveRows();
}

/*!
  Clears all items from the model.
  The items are deleted.
*/
void QThemeListModel::clear()
{
    if (!rowCount())
        return;
    beginRemoveRows(QModelIndex(), 0, rowCount());
    while (d->items.count() != 0) {
        QThemeListModelEntry* entry = d->items.takeFirst();
        Q_ASSERT(entry != 0);
        delete entry;
    }
    endRemoveRows();
}

/*!
  Triggers an update in the view for all items in the model.
 */
void QThemeListModel::triggerUpdate()
{
    emit dataChanged(index(0), index(rowCount() - 1));
}

/***************************************************************************/

struct QThemeListDelegatePrivate {
    QListView* listView;
    QThemedView* themedView;
};

QThemeListDelegate::QThemeListDelegate(QListView* listview, QThemedView* tv, QObject *parent)
        : QItemDelegate(parent)
{
    d = new QThemeListDelegatePrivate;
    d->listView = listview;
    d->themedView = tv;
}

QThemeListDelegate::~QThemeListDelegate()
{
    delete d;
}

void QThemeListDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QThemeListModel* model = qobject_cast<const QThemeListModel*>(index.model());
    QThemeListModelEntry* entry = model->themeListModelEntry(index);
    if (entry == 0) {
        qWarning("ThemeListDelegate::paint(): invalid index passed ");
        return;
    }
    Q_ASSERT(d->themedView != 0);
    Q_ASSERT(d->listView != 0);

    if (!entry->templateInstance())
        return;

    QSize listSize = sizeHint(option, index);
    p->save();
    p->setClipRect(option.rect);
    p->translate(option.rect.x(), option.rect.y());

//    QThemeItem *bgItem = d->themedView->findItem("background");
//     if (bgItem)
//         d->themedView->paint(p, d->listView->visualRect(index), bgItem);
//
//     QSize itemSize = sizeHint(option, index);
//    d->themedView->paint(p, QRect(0,0,itemSize.width(),itemSize.height()), entry->templateInstance());

    entry->templateInstance()->paint(p, new QStyleOptionGraphicsItem(), 0);
    p->restore();
}

QSize QThemeListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex &index) const
{
    const QThemeListModel* model = qobject_cast<const QThemeListModel*>(index.model());
    QThemeListModelEntry* entry = model->themeListModelEntry(index);
    int w = d->listView->width();
    if (!entry) {
        qWarning("ThemeListDelegate::sizeHint(): invalid index passed");
        return QSize(w, 0);
    }
    return QSize(w, height(entry, index));
}

int QThemeListDelegate::height(QThemeListModelEntry* entry, const QModelIndex&) const
{
    Q_ASSERT(entry != 0);
    if (!entry->templateInstance())
        return 0;
    return qRound(entry->templateInstance()->boundingRect().height());
    return 32;
}
