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

#include <qtaskview.h>

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QAction>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QTextDocument>
#include <QTextFrame>
#include <QAbstractTextDocumentLayout>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

/*!
  \class QTaskDelegate
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTaskDelegate class provides drawing of QTaskModel items (\l{QTask}{QTasks}).

  By using QTaskDelegate, applications using QTasks can achieve a consistent
  look and feel.

  The tasks are presented by rendering their \l {QTask::description()}{description()} in bold
  text.  Generally, this class will be used with a QTaskListView or other similar list view.

  The following image shows three QTask objects, rendered using a QTaskDelegate and a QTaskListView:
  \image qtaskview.png "QTaskListView and QTaskDelegate"

  \sa QTask, QTaskListView, QTaskModel, {Pim Library}
*/

/*!
  Constructs a QTaskDelegate with parent \a parent.
*/
QTaskDelegate::QTaskDelegate(QObject *parent) : QPimDelegate(parent)
{

}

/*!
  Destroys a QTaskDelegate.
*/
QTaskDelegate::~QTaskDelegate()
{

}

/*!
  \internal
  Format a date in a "useful" format.
*/
QString QTaskDelegate::formatDate(const QDate& date) const
{
    QDate today = QDate::currentDate();
    if (today == date)
        return tr("Today");
    else if (today.year() == date.year())
        return QTimeString::localMD(date);
    else
        return QTimeString::localYMD(date);
}

/*!
  \reimp
  */
int QTaskDelegate::subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Always one line of subtexts
    Q_UNUSED(option);
    Q_UNUSED(index);
    return 1;
}

// Draw headers non bold
/*!
  \reimp
*/
QFont QTaskDelegate::secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    return QTaskDelegate::secondaryFont(option, index);
}

/*!
  \reimp
*/
QList<StringPair> QTaskDelegate::subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subTexts;
    QString progress = index.model()->data(index.sibling(index.row(), QTaskModel::PercentCompleted),Qt::DisplayRole).toString();
    QTask::Status s = (QTask::Status)index.model()->data(index.sibling(index.row(), QTaskModel::Status),Qt::DisplayRole).toInt();

    static const QLatin1String space(" ");

    // We only have one line
    QString valueStr;
    QString headerStr;

    switch(s) {
        case QTask::Completed: // Just display the completed date
            {
                QDate finishedDate = index.model()->data(index.sibling(index.row(), QTaskModel::CompletedDate),Qt::DisplayRole).toDate();
                // finishedDate should be valid since it is 100% complete..
                headerStr = tr("Completed:");
                valueStr = formatDate(finishedDate);
            }
            break;

        case QTask::NotStarted: // Display due date, if any, otherwise not started
            {
                QDate dueDate = index.model()->data(index.sibling(index.row(), QTaskModel::DueDate),Qt::DisplayRole).toDate();
                if (dueDate.isValid()) {
                    headerStr = tr("Due:");
                    valueStr = tr("%1 (%2)", "4th July (Not started)").arg(formatDate( dueDate ), tr("Not started"));
                } else {
                    headerStr = tr("Status:");
                    valueStr = tr("Not started");
                }
            }
            break;

        case QTask::InProgress: // Display due date, if any, with progress
            {
                QDate dueDate = index.model()->data(index.sibling(index.row(), QTaskModel::DueDate),Qt::DisplayRole).toDate();
                if (dueDate.isValid()) {
                    headerStr = tr("Due:");
                    valueStr = tr("%1 (%2\%)", "4th July (10%)").arg(formatDate(dueDate), progress);
                } else {
                    // No due date, display "Progress: 10%"
                    headerStr = tr("Progress:");
                    valueStr =  tr("%1\%", "50%").arg(progress);
                }
            }
            break;

        case QTask::Waiting:
        case QTask::Deferred: // Display due date, if any, with progress and the task state
            {
                QDate dueDate = index.model()->data(index.sibling(index.row(), QTaskModel::DueDate),Qt::DisplayRole).toDate();
                QString stateStr = (s == QTask::Waiting) ? tr("Waiting") : tr("Deferred");
                if (dueDate.isValid()) {
                    headerStr = tr("Due:");
                    valueStr = tr("%1 (%2\%, %3)", "4th July (10%, Deferred)").arg(formatDate(dueDate), progress, stateStr);
                } else {
                    // No due date, display "Progress: 10% (Deferred)"
                    headerStr = tr("Progress:");
                    valueStr =  tr("%1\% (%2)", "50% (Deferred)").arg(progress, stateStr);
                }
            }
            break;
    }
    subTexts.append(qMakePair(headerStr + space, valueStr));

    return subTexts;
}

/*!
  \reimp
*/
void QTaskDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
                                  QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const
{
    Q_UNUSED(option);
    Q_UNUSED(trailingFloats);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize) / 2;

    QIcon i = qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole));

    QRect drawRect = option.rect;
    // 8px padding, 4 on either side
    if (rtl) {
        drawRect.setLeft(drawRect.right() - decorationSize - 8);
    } else {
        drawRect.setRight(decorationSize + 8);
    }
    QPoint drawOffset = QPoint(drawRect.left() + ((drawRect.width() - decorationSize)/2), drawRect.top() + ((drawRect.height() - decorationSize) / 2));

    p->drawPixmap(drawOffset, i.pixmap(decorationSize));

    leadingFloats.append(drawRect);

    // Check for recurrence (draw trailing icon)
    int repeatrule = index.model()->data(index.sibling(index.row(), QTaskModel::RepeatRule), Qt::DisplayRole).toInt();
    int reminder = index.model()->data(index.sibling(index.row(), QTaskModel::Alarm), Qt::DisplayRole).toInt();

    QList<QIcon*> trailicons;

    if (repeatrule != 0) {
        static QIcon repicon(":icon/repeat");
        trailicons.append(&repicon);
    }

    if (reminder == QAppointment::Audible) {
        static QIcon audicon(":icon/datebook/audible");
        trailicons.append(&audicon);
    }

    if (reminder == QAppointment::Visible) {
        static QIcon visicon(":icon/datebook/silent");
        trailicons.append(&visicon);
    }

    if (trailicons.count() != 0) {
        drawRect = option.rect;
        // 4px at the ends, 4px between
        int iconswidth = (trailicons.count() * (decorationSize + 4)) + 4;
        if (rtl) {
            drawRect.setRight(iconswidth);
        } else {
            drawRect.setLeft(drawRect.right() - iconswidth);
        }

        QPoint drawOffset = QPoint(rtl ? drawRect.left() + 4 : drawRect.right() - decorationSize - 4, drawRect.top() + ((drawRect.height() - decorationSize) / 2));
        foreach(QIcon *i, trailicons) {
            p->drawPixmap(drawOffset, i->pixmap(decorationSize));
            drawOffset.rx() += rtl ? decorationSize + 4 : -(decorationSize + 4);
        }

        trailingFloats.append(drawRect);
    }

}

/*!
  \reimp
*/
QSize QTaskDelegate::decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& s) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize) / 2;
    return QSize(decorationSize + s.width(), qMax(decorationSize + 2, s.height()));
}

/*!
  \class QTaskListView
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTaskListView class provides a list view widget with some convenience functions
  for use with QTaskModel.

  The convenience functions provided by QTaskListView include functions for interpreting
  the view's model, delegate and current item as the corresponding QTaskModel, QTaskDelegate and
  QTask objects.  In addition, QTaskListView enforces using a QTaskModel (or a derivative)
  as the model.

  Upon construction, QTaskListView automatically sets itself to use a QTaskDelegate for drawing,
  sets \c Batched layout mode (\l setLayoutMode()), and sets the resize mode to \c Adjust
  (\l setResizeMode()).

  The following image shows three QTask objects, rendered using a QTaskDelegate and a QTaskListView:
  \image qtaskview.png "QTaskListView and QTaskDelegate"

  \sa QTask, QTaskDelegate, QTaskModel, {Pim Library}
*/

/*!
    \fn QTask QTaskListView::currentTask() const

    Return the QTask for the currently selected index, or
    an empty QTask if there is no selected index.
*/

/*!
  \fn QTaskModel *QTaskListView::taskModel() const

  Returns the QTaskModel set for the view.
*/

/*!
  \fn QTaskDelegate *QTaskListView::taskDelegate() const

  Returns the QTaskDelegate set for the view.  During
  construction, QTaskListView  will automatically create
  a QTaskDelegate to use as the delegate, but this can be
  overridden with a different delegate derived from
  QTaskDelegate if necessary.
*/

/*!
  Constructs a QTaskListView with parent \a parent.

  This also sets the layout mode to \c Batched for performance,
  the resize mode to \c Adjust, and creates a QTaskDelegate
  to use as the delegate.
*/
QTaskListView::QTaskListView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new QTaskDelegate(this));
    setResizeMode(Adjust);
    setLayoutMode(Batched);

}

/*!
  Destroys the QTaskListView.
*/
QTaskListView::~QTaskListView()
{
}

/*!
  \overload

  Sets the model for the view to \a model.

  Will only accept the model if it inherits or is a QTaskModel.
  If the \a model does not inherit a QTaskModel, the existing
  model will be retained.
*/
void QTaskListView::setModel( QAbstractItemModel *model )
{
    QTaskModel *tm = qobject_cast<QTaskModel *>(model);
    if (!tm)
        return;
    QListView::setModel(model);
}

/*!
  Returns a list of QTasks selected in the view.  Fetching the complete QTask
  object can be expensive, so if a large number of tasks might be selected
  selectedTaskIds() could be used instead.

  \sa selectedTaskIds()
*/
QList<QTask> QTaskListView::selectedTasks() const
{
    QList<QTask> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->task(i));
    }
    return res;
}

/*!
  Returns the list of ids for QTasks selected from the view.

  \sa selectedTasks(), QTask::uid()
*/
QList<QUniqueId> QTaskListView::selectedTaskIds() const
{
    QList<QUniqueId> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->id(i));
    }
    return res;
}

/***************************
  * QTaskSelector
  ***********************/
class QTaskSelectorPrivate
{
public:
    QTaskListView *view;
    bool mNewTaskSelected;
    bool mTaskSelected;
};

/*!
  \class QTaskSelector
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTaskSelector class provides a way of selecting a single task from a QTaskModel.

  In addition, the user can optionally be allowed to indicate they want to create a new task,
  if none of the existing tasks are suitable.

  The following image shows a QTaskSelector with the option to create a new task highlighted.
  \image qtaskselector.png "QTaskSelector, with the new task option highlighted"

  \sa {Pim Library}
*/

/*!
  Constructs a QTaskSelector with parent \a parent.  If \a allowNew is true will also provide
  an option to indicate a new task should be created.
*/
QTaskSelector::QTaskSelector(bool allowNew, QWidget *parent)
    : QDialog(parent)
{
    d = new QTaskSelectorPrivate();
    d->mNewTaskSelected = false;
    d->mTaskSelected = false;
    setWindowTitle( tr("Select Tasks") );
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);

    d->view = new QTaskListView( this );
    d->view->setItemDelegate(new QTaskDelegate(d->view));
    d->view->setSelectionMode( QListView::SingleSelection );
    connect( d->view, SIGNAL(clicked(QModelIndex)), this, SLOT(setSelected(QModelIndex)) );
    connect( d->view, SIGNAL(activated(QModelIndex)), this, SLOT(setSelected(QModelIndex)) );

    l->addWidget( d->view );

    if( allowNew ) {
        QMenu *menu = QSoftMenuBar::menuFor( this );
        menu->addAction( QIcon(":icon/new"), tr("New"), this, SLOT(setNewSelected()) );
    }

    QtopiaApplication::setMenuLike( this, true );
}

/*!
    Destroys a QTaskSelector
    */
QTaskSelector::~QTaskSelector()
{
    delete d;
}
/*!
  \internal
  Accepts the dialog and indicates a that a new task should be created.
*/
void QTaskSelector::setNewSelected()
{
    d->mNewTaskSelected = true;
    accept();
}

/*!
  \internal
  Accepts the dialog and indicates a that a task at \a idx in the model was selected.
*/
void QTaskSelector::setSelected(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        d->view->setCurrentIndex(idx);
        d->mTaskSelected = true;
    }
    accept();
}

/*!
  Sets the model providing the choice of tasks to \a model.
*/
void QTaskSelector::setModel(QTaskModel *model)
{
    QAbstractItemModel *m = d->view->model();
    d->view->setModel(model);
    if (m != model) {
        if (m)
            disconnect(m, SIGNAL(modelReset()), this, SLOT(taskModelReset()));
        if (model)
            connect(model, SIGNAL(modelReset()), this, SLOT(taskModelReset()));
    }
}

/*!
  \internal
   Model resets don't generate current index changed message, so
   force the first task to be selected.
*/
void QTaskSelector::taskModelReset()
{
    /* we know our selection is invalid.. */
    QModelIndex newSel = d->view->model()->index(0,0);
    if (newSel.isValid()) {
        d->view->setCurrentIndex(newSel);
        d->view->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::ClearAndSelect);
    }
}

/*!
  Returns true if the dialog was accepted with the option to
  create a new task selected.
  Otherwise returns false.

  \sa taskSelected()
*/
bool QTaskSelector::newTaskSelected() const
{
    if (result() == Rejected)
        return false;
    return d->mNewTaskSelected;
}

/*!
  Returns true if the dialog was accepted with an existing task selected.
  Otherwise returns false.

  \sa newTaskSelected()
*/
bool QTaskSelector::taskSelected() const
{
    return d->mTaskSelected;
}

/*!
  Returns the task that was selected.  If no task was selected returns a null task.

  \sa taskSelected(), newTaskSelected()
*/
QTask QTaskSelector::selectedTask() const
{
    QTaskModel *m = qobject_cast<QTaskModel *>(d->view->model());

    if (result() == Rejected || d->mNewTaskSelected || !m || !d->view->currentIndex().isValid())
        return QTask();

    return m->task(d->view->currentIndex());
}

