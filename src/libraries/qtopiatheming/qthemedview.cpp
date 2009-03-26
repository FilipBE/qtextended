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

#include <QThemedView>
#include <QThemedScene>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QLinearGradient>
#include <QEvent>
#include <QResizeEvent>

#if defined(THEME_EDITOR)
#include <QValueSpace>
#include <QUndoStack>
#endif

class QThemedViewPrivate
{

public:
    QThemedViewPrivate() ;
    ~QThemedViewPrivate();

    QThemedScene *scene;
    QString name;
    QString fileName;
    QString themeName;
#ifdef THEME_EDITOR
    QUndoStack *undoStack;
#endif
};

QThemedViewPrivate::QThemedViewPrivate()
  : scene(0)
{
#ifdef THEME_EDITOR
    undoStack = new QUndoStack();
#endif
}

QThemedViewPrivate::~QThemedViewPrivate()
{
#ifdef THEME_EDITOR
    delete undoStack;
#endif
}

/***************************************************************************/

/*!
  \class QThemedView
    \inpublicgroup QtBaseModule

  \since 4.4

  \brief The QThemedView widget displays themed views in Qtopia.

  A Qt Extended theme is made of multiple themed views, each described by an XML
  document called the themed view XML.

  \ingroup qtopiatheming

  \sa QThemeItem
*/

/*!
    \fn void QThemedView::itemReleased(QThemeItem *item)
    Emitted whenever the interactive \a item item is released.
*/

/*!
    \fn void QThemedView::itemPressed(QThemeItem *item)
    Emitted whenever the interactive \a item item is pressed.
*/

/*!
    \fn void QThemedView::itemClicked(QThemeItem *item)
    Emitted whenever the interactive \a item item is cliked.
*/

/*!
    \fn QThemeItem* QThemedView::findItem(const QString &name) const
    Finds the item in the theme item tree that has the name \a name.

    Returns a pointer to the item if found, otherwise returns 0.

    Use qgraphicsitem_cast() to cast the item to the desired type.

    Example:

    \code
        QThemeWidgetItem *dialerLineEdit = qgraphicsitem_cast<QThemeWidgetItem*>(findItem("dialernumber"));
        if (dialerLineEdit) {
            dialerLineEdit->setWidget(new QLineEdit);
        }
    \endcode
*/

/*!
    \fn QList<QThemeWidgetItem*> QThemedView::widgets() const
    Returns a list of all widgets on the view.
*/

/*!
  Constructs a QThemedView object passing the \a parent widget
  and window \a flags to the QGraphicsView constructor.
*/
QThemedView::QThemedView(QWidget *parent, Qt::WindowFlags flags)
        : QGraphicsView(parent), d(new QThemedViewPrivate)
{
    setWindowFlags(flags);
    d->scene = new QThemedScene;
    setScene(d->scene);
    setFrameStyle(QFrame::NoFrame);
#if defined(THEME_EDITOR)
    setBackgroundBrush(palette().color(QPalette::Dark));
#endif
}

/*!
  Destroys the QThemedView.
*/
QThemedView::~QThemedView()
{
    delete d->scene;
    delete d;
}

/*!
  Loads the view \a fileName. Returns true if it succeeds, false otherwise.
*/
bool QThemedView::load(const QString &fileName)
{
#ifdef THEME_EDITOR
    d->scene->clearHandles(0);
    d->undoStack->setClean();
#endif
    d->fileName = fileName;
    d->name = fileName.split("/").last();
    d->scene->clear();
    return d->scene->load(fileName);
}

/*!
  \internal
*/
bool QThemedView::event(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        if (d->scene)
            d->scene->setPalette(palette());
        return true;
    }
    return QGraphicsView::event(event);
}

/*!
  \internal
*/
void QThemedView::resizeEvent(QResizeEvent *event)
{
    if (!isVisible())
        return;

    if (isVisible() && d->scene) {
        QRectF rect = d->scene->sceneRect();
        if (event->size() == rect.size())
            return;
        QGraphicsView::resizeEvent(event);
        rect.setSize(event->size());
#ifndef THEME_EDITOR
        d->scene->setSceneRect(0, 0, rect.width(), rect.height());
#endif
        d->scene->layout();
    }
}

/*!
  Returns the fileName of the view (for example \i title.xml).
*/
QString QThemedView::fileName() const
{
    return d->name;
}

/*!
  Returns the name of the theme.
*/
QString QThemedView::themePrefix() const
{
    return d->themeName;
}

/*!
    Sets the prefix of the theme to \a prefix. The theme prefix should be set before loading the view.
    
    Example:

    \code
        themedView->setThemePrefix("mytheme");
        themedView->load("/etc/themes/mytheme/title.xml");
    \endcode

    The theme prefix is used by the view to find the images. For example if the theme prefix is set to \i {mytheme}, the view will look for images in \i $QTOPIA_PATH/pics/themes/mytheme/.

    \sa load()
*/
void QThemedView::setThemePrefix(const QString &prefix)
{
    d->themeName = prefix;
}

/*!
  \internal
*/
void QThemedView::itemMouseReleased(QThemeItem *item)
{
    emit itemReleased(item);
}

/*!
  \internal
*/
void QThemedView::itemMousePressed(QThemeItem *item)
{
    emit itemClicked(item);
    emit itemPressed(item);
}

#ifdef THEME_EDITOR
/*!
  Saves the current state of the theme into the file it was loaded from
*/
void QThemedView::save()
{
    if (d->scene)
        d->scene->save(d->fileName);
}

/*!
  \internal
*/
QUndoStack *QThemedView::undoStack() const
{
    return d->undoStack;
}

/*!
  \internal
*/
QThemedScene *QThemedView::themedScene() const
{
    return d->scene;
}

void QThemedView::createDefault()
{
    if(!d->fileName.isEmpty()){
        d->scene->createDefault();
        d->undoStack->clear();
    }
}

#endif
