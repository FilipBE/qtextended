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

#include <QThemedScene>
#include <QThemeItem>
#include <QThemePageItem>
#include <QThemeLayoutItem>
#include <QThemedView>
#include <QThemeItemFactory>
#include <QDebug>
#include <QXmlStreamReader>
#include <QFile>
#include <QMap>
#include <QExpressionEvaluator>
#include <QThemeItemFactory>
#ifdef THEME_EDITOR
#include "themeselectionbox.h"
#include "qthemeitem_p.h"
#include <QXmlStreamWriter>
#include <QGraphicsSceneMouseEvent>
#include <QPointer>
#endif

class QThemedScenePrivate
{

public:
    QThemedScenePrivate();
    QXmlStreamReader    reader;
    QThemePageItem      *page;
    QMap<QExpressionEvaluator*, QThemeItem*> expressionToThemeItemMap;
#ifdef THEME_EDITOR
    ~QThemedScenePrivate();
    QXmlStreamWriter    writer;
    QThemeItem *highlighted;
    QThemeItem *deadParent;
    ThemeSelectionBox *handles;
    bool editorLock;
    bool showInvisible;
    bool visualAids;
    int idNum;
#endif
};
QThemedScenePrivate::QThemedScenePrivate() : page(0)
{
#ifdef THEME_EDITOR
    editorLock = false;
    showInvisible = false;
    visualAids = true;
    highlighted = 0;
    handles = 0;
    idNum = 0x10; //0x00 is the page, and it doesn't get set the normal way
    deadParent = 0;
#endif
}
#ifdef THEME_EDITOR
QThemedScenePrivate::~QThemedScenePrivate()
{
    /*if(deadParent) //Is it collected already?
        delete deadParent;*/
}
#endif

/***************************************************************************/

/*!
  \internal
  \class QThemedScene
    \inpublicgroup QtBaseModule
  \since 4.4

  \brief The QThemedScene class is a container for QThemeItems in Qtopia.

  The class serves as a container for QThemeItems. It is used together with QThemedView for
  visualizing theme items. QThemedScene uses The Graphics View Framework.

  \ingroup qtopiatheming

  \sa QThemedView, QThemeItem
*/

/*!
  Constructs a QThemedScene object passing the \a parent object the QGraphicsScene constructor.
*/
QThemedScene::QThemedScene(QObject *parent)
        : QGraphicsScene(parent), d(new QThemedScenePrivate)
{
#ifdef THEME_EDITOR
    connect(this, SIGNAL(selectionChanged()), this, SLOT(clearHandles()));
#endif
}

/*!
  Destroys the QThemedScene;
*/
QThemedScene::~QThemedScene()
{
    delete d;
}

/*!
  Loads the scene from the XML file \a filename.
*/
bool QThemedScene::load(const QString &filename)
{
    QFile file(filename);

    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        d->reader.setDevice(&file);

        while (!d->reader.atEnd()) {
            d->reader.readNext();
            if (d->reader.isStartElement()) {
                if (d->reader.name() == "page") {
                    QThemeItem *item = QThemeItemFactory::create("page", 0);
                    d->page = qgraphicsitem_cast<QThemePageItem*>(item);
                    if (d->page) {
                        addItem(d->page);
                        d->page->load(d->reader);
                    }
                } else {
                    d->reader.raiseError(QLatin1String("Page item expected"));
                }
            }
        }
        if (d->reader.hasError()) {
            if (d->reader.error() != QXmlStreamReader::UnexpectedElementError) {
                qWarning() << "Error while reading XML theme:" << d->reader.errorString();
#ifdef THEME_EDITOR
                createDefault();
#endif
                return false;
            }
        }
#ifdef THEME_EDITOR
        d->page->layoutPage();
#endif
        return true;
    }
#ifdef THEME_EDITOR
    createDefault();
#endif
    return false;
}

#ifdef THEME_EDITOR
void QThemedScene::save(const QString &filename)
{
    QFile file(filename);
    if (!d->page){
        qWarning("Trying to save non-existent QThemedScene");
        return;
    }
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        d->writer.setDevice(&file);
        d->writer.setAutoFormatting(true);
        d->writer.writeStartDocument();
        d->page->save(d->writer);
        d->writer.writeEndDocument();
    }
}
#endif
/*!
  Returns the view.
*/
QThemedView *QThemedScene::themedView() const
{
    QGraphicsView *v = views().last();
    return static_cast<QThemedView*>(v);
}

/*!
  Lays out the \a item. If \a item is null, lays out the entire scene.
*/
void QThemedScene::layout(QThemeItem *item)
{
    if (!item)
        item = d->page;

    if (!item)
        return;

    QThemeLayoutItem *l = qgraphicsitem_cast<QThemeLayoutItem*>(item->parentItem());

    if (!item->parentItem() || !l) {
        item->layout();
    }

    foreach(QGraphicsItem *item, item->childItems()) {
        QThemeItem *themeItem = QThemeItem::themeItem(item);
        if (themeItem)
            layout(themeItem);
    }
}

/*!
  Registers the expression \a expression for the given theme \a item.
  The ThemedView will unregister the expression when \a expression is deleted, and call
  the expressionChanged() call-back on \a item whenever the QExpressionEvaluator::termsChanged()
  signal is emitted.
*/
void QThemedScene::registerExpression(QThemeItem *item, QExpressionEvaluator *expression)
{
    if (d->expressionToThemeItemMap.contains(expression)) {
        qWarning("ThemedView::registerExpression - Already registered expression for this item");
        return;
    }

    d->expressionToThemeItemMap.insert(expression, item);

    connect(expression, SIGNAL(termsChanged()), this, SLOT(notifyExpressionChanged()));
    connect(expression, SIGNAL(destroyed(QObject*)), this, SLOT(expressionDestroyed(QObject*)));
}

/*!
  \internal
*/
void QThemedScene::notifyExpressionChanged()
{
    QExpressionEvaluator* expr = (QExpressionEvaluator *)sender();
    Q_ASSERT(d->expressionToThemeItemMap.contains(expr));
    d->expressionToThemeItemMap[expr]->expressionChanged(expr);
}

/*!
  \internal
*/
void QThemedScene::expressionDestroyed(QObject* obj)
{
    QExpressionEvaluator* expr = (QExpressionEvaluator *)obj;
    Q_ASSERT(d->expressionToThemeItemMap.contains(expr));
    d->expressionToThemeItemMap.remove(expr);
}

#ifdef THEME_EDITOR
void QThemedScene::setShowInvisible(bool set)
{
    d->showInvisible = set;
}

bool QThemedScene::showInvisible()
{
    return d->showInvisible;
}

void QThemedScene::makeHandles(QThemeItem* item)
{
    if (d->handles) {//Need to remove from scene first?
        if (d->handles->itemPtr() == item) {
            d->handles->refresh();
        } else {
            QRectF updateRect = d->handles->sceneBoundingRect();
            removeItem(d->handles);
            delete d->handles;
            d->handles = 0;
            invalidate(updateRect);
        }
    }
    if (!item->isVisible())
        return;
    if (!d->handles) {
        d->handles = new ThemeSelectionBox(item);
        addItem(d->handles);
        d->handles->setZValue(1 << 30);//Needs to be on top
        d->handles->setPos(item->scenePos());
    }
}

/*!
\internal
This must be called before the item is destroyed. A ThemeSelectionBox without a
valid QThemeItem will cause a segmentation fault.
*/
void QThemedScene::clearHandles(QThemeItem* item)
{
    Q_UNUSED(item);
    if (d->handles) {//Need to remove from scene first?
        QRectF updateRect = d->handles->sceneBoundingRect();
        removeItem(d->handles);
        delete d->handles;
        d->handles = 0;
        update(updateRect);
    }
}

void QThemedScene::highlight(QGraphicsItem* item)
{
    QThemeItem *newItem = QThemeItem::themeItem(item);
    if (d->highlighted == newItem)
        return;
    QThemeItem *oldItem = d->highlighted;
    d->highlighted = newItem;
    if (oldItem)
        oldItem->update();
    if (newItem)
        newItem->update();
    emit itemHighlighted(newItem);
}

bool QThemedScene::highlighted(QThemeItem* item)
{
    return item == d->highlighted;
}

void QThemedScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit wantSelectMenu(event);
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}

int QThemedScene::nextId()
{
    int ret = d->idNum;
    d->idNum += 0x10;
    if (d->idNum <= 0x10)//overflow (d->idNum initialized to 0x10, after first add should be 0x20)
        qWarning("Non-Fatal Error: QThemeItem ID Overflow. Close and reload themes to avoid problems.");
    return ret;
}

void QThemedScene::setEditorLock(bool set)
{
    d->editorLock = set;
}

bool QThemedScene::editorLock()
{
    return d->editorLock;
}

void QThemedScene::setVisualAids(bool set)
{
    d->visualAids = set;
}

bool QThemedScene::visualAids()
{
    return d->visualAids;
}
/*
   The 'deadParent' item is an item in the scene that is not a descendant of
   page and so is not saved. It it set to be the parent of 'deleted' nodes so
   that they can be removed from the tree but not lost.
   Deleted nodes are not actually deleted from memory, because their pointer is
   used in various undo actions, including the one that 'undeletes' them.
   Undelete adds an undo action to the queue such that undoing it deletes the item.
*/
QThemeItem *QThemedScene::deadParent()
{
    if(!d->deadParent){
        d->deadParent = new QThemeItem();
        d->deadParent->setVisible(false);
        addItem(d->deadParent);
    }
    return d->deadParent;
}

void QThemedScene::deleteItem(QThemeItem *item)
{
    if(item==d->page)
        return;//Cannot delete Page item
    item->aboutToChange();
    item->setParentItem(deadParent());
    item->setActive(false);
    delete item->d->activeExpression;
    item->d->activeExpression = 0;
    item->finishedChange();
    item->setVisible(false);
}

void QThemedScene::undeleteItem(QThemeItem *item)
{
    if(item==d->page)
        return;//Cannot undelete Page item
    QExpressionEvaluator *activeExpr=item->d->activeExpression;
    bool activeBool = item->isActive();
    QGraphicsItem *parent = item->parentItem();
    item->setParentItem(deadParent());
    item->setActive(false);
    item->d->activeExpression = 0;
    item->aboutToChange();
    item->setParentItem(parent);
    item->setActive(activeBool);
    item->d->activeExpression = activeExpr;
    item->finishedChange();
}

void QThemedScene::createDefault()
{
    //All items should be children of the page
    if(d->page)
        delete d->page;
    d->page = new QThemePageItem();
    addItem(d->page);
    d->page->layoutPage();
}
#endif
