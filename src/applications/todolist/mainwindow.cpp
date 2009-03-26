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

#include "mainwindow.h"
#include "todoentryimpl.h"
#include "todotable.h"
#include "todocategoryselector.h"
#include "listpositionbar.h"
#include "reminderpicker.h"

#include <qcontent.h>
#include <qtopiaapplication.h>
#include <qsettings.h>

#include <qtopiasendvia.h>
#include <qtopianamespace.h>
#include <qtask.h>
#include <qtaskview.h>
#include <QCategoryFilter>
#include <qtopia/qsoftmenubar.h>

#include <QDL>
#include <QDLBrowserClient>
#include <QDSActionRequest>
#include <QDSData>

#include <QAction>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QStackedWidget>
#include <QTimer>
#include <QAppointmentModel>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


static QString statusToTrString(QTask::Status s)
{
    switch( s ) {
        case QTask::NotStarted: return qApp->translate("QtopiaPim", "Not yet started"); break;
        case QTask::InProgress: return qApp->translate("QtopiaPim", "In progress"); break;
        case QTask::Waiting: return qApp->translate("QtopiaPim", "Waiting"); break;
        case QTask::Deferred: return qApp->translate("QtopiaPim", "Deferred"); break;
        default: return qApp->translate("QtopiaPim", "Completed"); break;
    }
}

TodoView::TodoView( QWidget *parent)
:   QDLBrowserClient( parent, "qdlnotes" ), mIconsLoaded(false)
{
    setFrameStyle(NoFrame);
}

void TodoView::init( const QTask &task )
{
    mTask = task;
    loadLinks( task.customField( QDL::CLIENT_DATA_KEY ) );

    if (!mIconsLoaded) {
        /* precache some icons, scaled nicely - just using these as img src=... gives poor results */
        QIcon audible(":icon/datebook/audible");
        QIcon repeat(":icon/repeat");
        QIcon silent(":icon/datebook/silent");
        QIcon completed(":icon/ok");
        QIcon priority(":icon/priority");
        int iconMetric = style()->pixelMetric(QStyle::PM_SmallIconSize);

        QTextDocument* doc = document();
        doc->addResource(QTextDocument::ImageResource, QUrl("audibleicon"), audible.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("repeaticon"), repeat.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("silenticon"), silent.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("completedicon"), completed.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("priorityicon"), priority.pixmap(iconMetric));
        mIconsLoaded = true;
    }

    QString txt = createTaskText(task);
    setHtml(txt);
    verifyLinks();

    // setEditFocus so that TodoView::keyPressEvent() will be called when
    // viewing a sequence of linked tasks
    setEditFocus( true );
}

void TodoView::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
        case Qt::Key_Back:
            emit done();
            return;
        case Qt::Key_Left:
            if (layoutDirection() == Qt::LeftToRight)
                emit previous();
            else
                emit next();
            e->accept();
            return;
        case Qt::Key_Right:
            if (layoutDirection() == Qt::LeftToRight)
                emit next();
            else
                emit previous();
            e->accept();
            return;
        default:
            QDLBrowserClient::keyPressEvent( e );
            break;
    }
}

QString TodoView::createTaskText(const QTask &task)
{
    QAppointment ev;
    QString text;
    QDate today = QDate::currentDate();

    bool rtl = qApp->layoutDirection() == Qt::RightToLeft;
    QLatin1String floatLead(rtl ? "float: right;" : "float: left;");
    QLatin1String floatTrail(rtl ? "float: left;" : "float: right;");

    QUniqueId apptId = task.dependentChildrenOfType("duedate").value(0);

    QString leadIcons;
    QString trailIcons;

    if (!apptId.isNull()) {
        QAppointmentModel am;
        ev = am.appointment(apptId);
    }

    // Some nice icons first
    if (task.isCompleted())
        leadIcons += QLatin1String("<img src=\"completedicon\">");
    else if (task.priority() <= 2)
        leadIcons += QLatin1String("<img src=\"priorityicon\">");


    if ( ev.hasRepeat() )
        trailIcons += QLatin1String("<img src=\"repeaticon\">");

    if ( ev.hasAlarm() ) {
        if (ev.alarm() == QAppointment::Audible)
            trailIcons += QLatin1String("<img src=\"audibleicon\">");
        else
            trailIcons += QLatin1String("<img src=\"silenticon\">");
    }

    if (!leadIcons.isEmpty())
        text += "<table style='" + floatLead + "'><tr><td>" + leadIcons + "</td></tr></table>";
    if (!trailIcons.isEmpty())
        text += "<table style='" + floatTrail + "'><tr><td>" + trailIcons + "</td></tr></table>";

    text += "<b>";

    if ( !task.description().isEmpty() )
        text += Qt::escape(task.description());
    else
        text += tr("No description", "no description entered for task");

    text += "</b>";

    if (task.hasDueDate())
        text += "<br><br><b>"+tr("Due:")+" </b>" + formatDate(task.dueDate(), today);

    if ( ev.hasAlarm() ) {
        text += "<br><b>" + tr("Reminder:") + " </b>" + ReminderPicker::formatReminder(true, ev.alarm(), ev.alarmDelay());
    }
    if ( !task.startedDate().isNull() && task.status() != QTask::NotStarted)
        text += "<br><b>" + qApp->translate("QtopiaPim", "Started:") + "</b> " +
            QTimeString::localYMD( task.startedDate(), QTimeString::Long );
    if ( !task.completedDate().isNull() && task.isCompleted() )
        text += "<br><b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " +
            QTimeString::localYMD( task.completedDate(), QTimeString::Long );

    QString statusString = statusToTrString( task.status() );
    text += "<br><b>" + qApp->translate("QtopiaPim", "Status:") + "</b> " + statusString;
    text +="<br><b>" + qApp->translate("QtopiaPim", "Priority:") + "</b> " + QString::number( task.priority() );

    if ( (task.status() != QTask::NotStarted && !task.isCompleted() ) )
        text += "<br><b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " + QString::number(task.percentCompleted()) + " " +
            qApp->translate("QtopiaPim", "percent", "Word or symbol after numbers for percentage");

    if ( ev.hasRepeat() ) {
        QString word;
        if ( ev.repeatRule() == QAppointment::Daily )
            if ( ev.frequency() > 1 )
                word = tr("every %1 days", "eg. every 2 days").arg(ev.frequency());
            else
                word = tr("every day");
        else if ( ev.repeatRule() == QAppointment::Weekly ) {
            int repDays = 0;
            int dayOfWeek;
            QString repStr;

            for (dayOfWeek = 1; dayOfWeek <= 7; dayOfWeek++) {
                if (ev.repeatOnWeekDay(dayOfWeek)) {
                    repDays++;
                }
            }

            // XXX Fix I18N for 4.4
            if ( ev.frequency() > 1 )
                word = tr("every %2 weeks on %1", "eg. every 2 weeks on: Monday, Wednesday");
            else
                word = tr("every week on %1", "e.g. every week on: Monday, Thursday");

            if (repDays > 1) {
                Qt::DayOfWeek startDay = Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday;
                dayOfWeek = startDay;
                do {
                    if (ev.repeatOnWeekDay(dayOfWeek)) {
                        if (!repStr.isEmpty())
                            repStr += tr(",", "list separator - e.g. Monday_, _Tuesday") + " ";
                        repStr += QTimeString::nameOfWeekDay(dayOfWeek, QTimeString::Long);
                    }
                    if (dayOfWeek == 7)
                        dayOfWeek = 1;
                    else
                        dayOfWeek++;
                } while (dayOfWeek != startDay);
            } else {
                repStr = QTimeString::nameOfWeekDay(ev.startInCurrentTZ().date().dayOfWeek(), QTimeString::Long);
            }

            word = word.arg(repStr);

            if (ev.frequency() > 1)
                word = word.arg(ev.frequency());
        }
        else if ( ev.repeatRule() == QAppointment::MonthlyDate ||
                ev.repeatRule() == QAppointment::MonthlyDay ||
                ev.repeatRule() == QAppointment::MonthlyEndDay )
            /// XXX this could also get extra information
            if ( ev.frequency() > 1 )
                word = tr("every %1 months", "eg. every 2 months").arg(ev.frequency());
            else
                word = tr("every month");
        else if ( ev.repeatRule() == QAppointment::Yearly )
            if ( ev.frequency() > 1 )
                word = tr("every %1 years", "eg. every 2 years").arg(ev.frequency());
            else
                word = tr("every year");

        text += "<br><b>" + tr("Repeat:") + " </b>";

        QString endword;
        if ( ev.repeatForever() )
            endword = tr("forever");
        else
            endword = tr("ending on %1", "1=date").arg(formatDate(ev.repeatUntil(), today));

        text += tr("From %1, %2, %3", "1=date, 2=every x days/weeks/etc, 3=ending on date/forever")
            .arg(formatDate(ev.startInCurrentTZ().date(), today))
            .arg(word)
            .arg(endword);
    }

    if ( !task.notes().isEmpty() ) {
        QString txt = task.notes();
        text += "<br><b>" + tr("Notes:") + " </b>" + txt; // txt is already formatted html
    }

    return text;
}

QString TodoView::formatDate(const QDate& date, const QDate& today)
{
    if (date == today) {
        return tr("today");
    } else {
        /* We want either "Mon, 26 Feb 2008" or "Mon, 26 Feb" if the year is the current year */
        if (date.year() == today.year())
            return tr("%1, %2", "[Mon], [26 Feb]").arg(QTimeString::localDayOfWeek(date), QTimeString::localMD(date, QTimeString::Short));
        else
            return tr("%1, %2", "[Mon], [26 Feb 2007]").arg(QTimeString::localDayOfWeek(date), QTimeString::localYMD(date, QTimeString::Medium));
    }
}

//===========================================================================

TodoWindow::TodoWindow( QWidget *parent, Qt::WFlags f) :
    QMainWindow( parent, f ),
    prevTasks(), contextMenuActionsDirty(true), showCompleted(true)
{
    QtopiaApplication::loadTranslations("libqtopiapim");

    setWindowTitle( tr("Tasks") );

    tView = 0;

    centralView = new QStackedWidget;
    listView = new QWidget;
    listView->setFocusPolicy(Qt::NoFocus);
    centralView->addWidget(listView);

    model = new QTaskModel(this);
    table = new TodoTable;
    table->setModel(model);
    closeAfterDetailView = false;
   // table->setContentsMargins(0,0,0,0);
    table->installEventFilter(this);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setWhatsThis( tr("List of tasks matching the completion and category filters.") );

    setCentralWidget( centralView );

    createUI();

    connect( table, SIGNAL(taskActivated(QTask)),
            this, SLOT(showDetailView(QTask)) );
    connect( table, SIGNAL(currentItemChanged(QModelIndex)),
             this, SLOT(markMenuDirty()));
    connect( model, SIGNAL(modelReset()), this, SLOT(taskModelReset()));
    connect(qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );
    connect(qApp, SIGNAL(reload()), this, SLOT(reload()));
    connect(qApp, SIGNAL(flush()), this, SLOT(flush()));

    new TasksService(this);

    taskModelReset();
    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

TodoWindow::~TodoWindow()
{
}

void TodoWindow::createUI()
{
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(0);
    grid->setMargin(0);
    grid->addWidget( table, 0, 0, 1, 2);

    categoryLbl = new QLabel;
    categoryLbl->hide();
    grid->addWidget(categoryLbl, 1, 0, 1, 2);

    categoryDlg = 0;

    QSettings config("Trolltech","todo");
    config.beginGroup( "View" );
    QCategoryFilter f;
    f.readConfig(config, "Category");
    if (config.contains( "ShowComplete" ))
        showCompleted = config.value( "ShowComplete" ).toBool();
    model->setFilterCompleted( !showCompleted );
    catSelected(f);
    listView->setLayout(grid);
}

void TodoWindow::delayedInit()
{
    newAction = new QAction( QIcon( ":icon/new" ), tr( "New task" ), this );
    connect( newAction, SIGNAL(triggered()),
             this, SLOT(createNewEntry()) );
    newAction->setWhatsThis( tr("Create a new task.") );

    editAction = new QAction( QIcon( ":icon/edit" ), tr( "Edit task" ), this );
    connect( editAction, SIGNAL(triggered()),
             this, SLOT(editCurrentEntry()) );
    editAction->setWhatsThis( tr("Edit the highlighted task.") );

    deleteAction = new QAction( QIcon( ":icon/trash" ), tr( "Delete task" ), this );
    connect( deleteAction, SIGNAL(triggered()),
             this, SLOT(deleteCurrentEntry()) );
    deleteAction->setWhatsThis( tr("Delete the highlighted task.") );

    markDoneAction = new QAction( QIcon( ":icon/ok" ), tr( "Mark task complete" ), this );
    markDoneAction->setWhatsThis( tr("Mark the current task as completed.") );
    connect( markDoneAction, SIGNAL(triggered()),
             this, SLOT(markTaskDone()) );

    markNotDoneAction = new QAction( QIcon( ":icon/phone/reject" ), tr( "Mark task incomplete" ), this );
    markNotDoneAction->setWhatsThis( tr("Mark the current task as not completed.") );
    connect( markNotDoneAction, SIGNAL(triggered()),
             this, SLOT(markTaskNotDone()) );

    if ( QtopiaSendVia::isDataSupported("text/x-vcalendar")) {
        beamfile = Qtopia::tempDir() + "obex";
        QDir d;
        d.mkdir(beamfile);
        beamfile += "/todo.vcs";
        beamAction = new QAction( QIcon( ":icon/beam" ), tr( "Send" ), this );
        connect( beamAction, SIGNAL(triggered()),
                 this, SLOT(beamCurrentEntry()) );
        beamAction->setWhatsThis( tr("Send the highlighted task to another device.") );
    } else {
        beamAction = 0;
    }

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(newAction);
    contextMenu->addAction(markDoneAction);
    contextMenu->addAction(markNotDoneAction);
    contextMenu->addAction(editAction);
    contextMenu->addAction(deleteAction);

    if (beamAction)
        contextMenu->addAction(beamAction);

    actionShowCompleted = new QAction(tr("Show completed tasks"), this);
    actionHideCompleted = new QAction(tr("Hide completed tasks"), this);


    connect(actionShowCompleted, SIGNAL(triggered()), this, SLOT(showCompletedTasks()));
    connect(actionHideCompleted, SIGNAL(triggered()), this, SLOT(hideCompletedTasks()));
    contextMenu->addAction(actionShowCompleted);
    contextMenu->addAction(actionHideCompleted);
    if (showCompleted)
        actionShowCompleted->setVisible(false);
    else
        actionHideCompleted->setVisible(false);

    actionCategory = new QAction(QIcon(":icon/viewcategory"), tr("View Category..."), this );
    connect( actionCategory, SIGNAL(triggered()), this, SLOT(selectCategory()));
    contextMenu->addAction(actionCategory);

    updateContextMenu();
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
}

void TodoWindow::markMenuDirty()
{
    contextMenuActionsDirty = true;
}

void TodoWindow::updateContextMenu()
{
    if (!contextMenuActionsDirty)
        return;

    QTask todo;

    if ( centralView->currentIndex() == 0 ) {
        newAction->setVisible(true);
        actionShowCompleted->setVisible(!showCompleted);
        actionHideCompleted->setVisible(showCompleted);
        actionCategory->setVisible(true);
        editAction->setVisible(false);
        deleteAction->setVisible(false);
        if (beamAction)
            beamAction->setVisible(false);
        todo = table->currentTask();
    } else if ( centralView->currentIndex() == 1 ) {
        newAction->setVisible(false);
        actionShowCompleted->setVisible(false);
        actionHideCompleted->setVisible(false);
        actionCategory->setVisible(false);
        editAction->setVisible(true);
        deleteAction->setVisible(true);
        if (beamAction)
            beamAction->setVisible(true);
        todo = tView->task();
        if (todo.uid().isNull())
            todo = table->currentTask();
    }

    if (todo != QTask()) {
        if (todo.status() == QTask::Completed) {
            markDoneAction->setVisible(false);
            markNotDoneAction->setVisible(true);
        } else {
            markDoneAction->setVisible(true);
            markNotDoneAction->setVisible(false);
        }
    } else {
        markDoneAction->setVisible(false);
        markNotDoneAction->setVisible(false);
    }

    contextMenuActionsDirty = false;
}

void TodoWindow::appMessage(const QString &msg, const QByteArray &data)
{
    /*@ \service Tasks */
    bool needShow = false;
    if ( msg == "receiveData(QString,QString)" ) {
        QDataStream stream(data);
        QString f,t;
        stream >> f >> t;
        if ( t.toLower() == "text/x-vcalendar" )
            if ( receiveFile(f) )
                needShow = true;
        QFile::remove(f);
    } else if (msg == "updateRecurringTasks(QDateTime,int)") {
        // Poke the model
        model->updateRecurringTasks();
    }

    if ( needShow ) {
//TODO: Something needs to happen here in the absence of the setKeepRunning() behaviour.
//      showMaximized();
//      raise();
//      setActiveWindow();
    }
}

void TodoWindow::showListView()
{
    if ( centralView->currentIndex() != 0 ) {
        centralView->setCurrentIndex(0);
        table->setEditFocus(true);
        setWindowTitle( tr("Tasks") );
        contextMenuActionsDirty = true;
    }
}

void TodoWindow::showDetailView(const QTask &t)
{
    if( isHidden() ) // only close after view if hidden on first activation
    {
        closeAfterDetailView = true;
    }

    todoView()->init( t );

    QModelIndex newSel = model->index(t.uid());

    // Only allow the navigation when we come from the list view
    if (prevTasks.count() > 0) {
        listPositionBar->setPosition(0,0);
    } else {
        listPositionBar->setPosition(newSel.row() + 1, model->rowCount());
    }

    if (centralView->currentIndex() != 1) {
        centralView->setCurrentIndex(1);
        // ?? todoView()->setFocus();  //To avoid events being passed to QTable
        setWindowTitle( tr("Task Details") );

        /* make this the current list item too */
        if ( newSel.isValid() ) {
            table->setCurrentIndex(newSel);
            table->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }

        contextMenuActionsDirty = true;
    }
}

TodoView* TodoWindow::todoView()
{
    if ( !tView ) {
        QWidget *w = new QWidget();
        QVBoxLayout *vl = new QVBoxLayout();
        vl->setSpacing(0);
        vl->setMargin(0);
        tView = new TodoView(this);
        connect( tView, SIGNAL(done()), this, SLOT(doneDetailView()) );
        connect( tView, SIGNAL(previous()), this, SLOT(viewPrevious()) );
        connect( tView, SIGNAL(next()), this, SLOT(viewNext()) );
        QSoftMenuBar::setLabel(tView, Qt::Key_Select, QSoftMenuBar::NoLabel);

        listPositionBar = new ListPositionBar();
        connect( listPositionBar, SIGNAL(nextPosition()), this, SLOT(viewNext()));
        connect( listPositionBar, SIGNAL(previousPosition()), this, SLOT(viewPrevious()));
        listPositionBar->setMessage(tr("Task %1 of %2"));

        vl->addWidget(listPositionBar);
        vl->addWidget(tView);
        w->setLayout(vl);
        centralView->addWidget(w);
    }

    return tView;
}

void TodoWindow::doneDetailView()
{
    if (closeAfterDetailView) {
        closeAfterDetailView = false;
        close();
    }
    else {
        if ( prevTasks.count() == 0 ) {
            showListView();
        } else {
            // We occasionally push null ids here (if we get a
            // qdl/qcop message and we're not already on the
            // details view)
            QUniqueId id;
            bool validId = false;
            while(prevTasks.count() > 0) {
                id = prevTasks.pop();
                if (!id.isNull()) {
                    showDetailView(model->task(id));
                    validId = true;
                    break;
                }
            }

            if (!validId)
                showListView();
        }
    }
}
// Helper function

void TodoWindow::updateDependentAppointment(const QTask& src, const QAppointment& appt)
{
    // Update the appointment, if it exists
    QUniqueId apptId = src.dependentChildrenOfType("duedate").value(0); // no tr
    if (!apptId.isNull()) {
        QAppointmentModel am;
        QAppointmentContext *context = qobject_cast<QAppointmentContext*>(am.context(apptId));
        if (context != NULL) {
            // Copy some of the fields
            QAppointment generated = am.appointment(apptId);
            generated.setRepeatRule(appt.repeatRule());
            generated.setAlarm(appt.alarmDelay(), appt.alarm());
            generated.setWeekFlags(appt.weekFlags());
            if (appt.repeatForever())
                generated.setRepeatForever();
            else
                generated.setRepeatUntil(appt.repeatUntil());
            generated.setFrequency(appt.frequency());
            context->updateAppointment(generated);
            model->updateRecurringTasks();
        }
    }
}

void TodoWindow::createNewEntry(bool useCurrentCategory)
{
    QList<QString> categories;
    if (useCurrentCategory)
        categories = model->categoryFilter().requiredCategories();

    TaskDialog* edit = new TaskDialog( categories, parentWidget() );
    edit->setObjectName("edit-screen");
    edit->setModal(true);
    edit->setWindowTitle( tr( "New Task" ) );

    // Use a signal so we can prep the details dialog before execDialog returns
    // and prevent flicker
    connect(edit, SIGNAL(taskEditAccepted(QTask,QAppointment)), this, SLOT(saveNewTask(QTask,QAppointment)));

    QtopiaApplication::execDialog( edit );

    /* the dialog emits during accept(), so we don't need to do anything */
    delete edit;
}

void TodoWindow::saveNewTask(const QTask& newTask, const QAppointment& taskAppointment)
{
    QTask todo(newTask);
    if ( todo.description().isEmpty() && !todo.notes().isEmpty()) {
        todo.setDescription( tr("Note") );
    }
    if ( !todo.description().isEmpty() ) {
        todo.setUid(model->addTask( todo ));
        updateDependentAppointment(todo, taskAppointment);
        QModelIndex newSel = model->index(todo.uid());
        if ( newSel.isValid() ) {
            table->setCurrentIndex(newSel);
            table->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        // Make sure we paint over the dialog
        raise();
    }
}

void TodoWindow::deleteCurrentEntry()
{
    QTask task;
    if(centralView->currentIndex() == 1) {
        task = tView->task();
    } else if(table->currentIndex().isValid()) {
        task = table->currentTask();
    } else {
        return;
    }
    int rowToSelect = model->index(task.uid()).row();
    QString strName = Qt::escape(task.description().left(30));

    if ( Qtopia::confirmDelete( this, tr( "Tasks" ),
        strName.simplified() ) ) {
        removeTaskQDLLink( task );
        model->removeTask( task );

        // Select the next task
        QModelIndex newIdx = model->index(rowToSelect, 0);
        if (!newIdx.isValid())
            newIdx = model->index(model->rowCount() - 1, 0);
        if (newIdx.isValid()) {
            table->setCurrentIndex(newIdx);
            table->selectionModel()->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);
        }
        showListView();
    }
}

void TodoWindow::editCurrentEntry()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    TaskDialog* edit = new TaskDialog( todo, parentWidget() );
    edit->setObjectName("edit-screen");
    edit->setModal( true );
    edit->setWindowTitle( tr( "Edit Task" ) );

    int ret = QtopiaApplication::execDialog( edit );

    if ( ret == QDialog::Accepted ) {
        todo = edit->todoEntry();
        model->updateTask( todo );
        todo = model->task(todo.uid());
        updateDependentAppointment(todo, edit->todoAppointment()); // no tr
        showDetailView(todo);
    }

    delete edit;
}

void TodoWindow::showCompletedTasks()
{
    showCompleted = true;
    model->setFilterCompleted( false );
    actionHideCompleted->setVisible( centralView->currentIndex() == 0 );
    actionShowCompleted->setVisible( false );
}

void TodoWindow::hideCompletedTasks()
{
    showCompleted = false;
    model->setFilterCompleted( true );
    actionHideCompleted->setVisible( false );
    actionShowCompleted->setVisible( centralView->currentIndex() == 0 );
}

void TodoWindow::viewPrevious()
{
    // We only allow navigating tasks when the task detail stack is empty
    // (e.g. we've entered the details from the list view, not from qcop/qdl)
    if (prevTasks.count() == 0) {
        QModelIndex idx = table->currentIndex();
        if (idx.isValid()) {
            idx = idx.sibling(idx.row() - 1, 0);
            if (idx.isValid()) {
                showDetailView(model->task(idx));
                table->setCurrentIndex(idx);
                table->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }
}

void TodoWindow::viewNext()
{
    if (prevTasks.count() == 0) {
        QModelIndex idx = table->currentIndex();
        if (idx.isValid()) {
            idx = idx.sibling(idx.row() + 1, idx.column());
            if (idx.isValid()) {
                showDetailView(model->task(idx));
                table->setCurrentIndex(idx);
                table->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }
}

void TodoWindow::taskModelReset()
{
    /* Basically, we know that the current index will never be valid, so base our decisions
       on the number of tasks in the model */
    if ( model->count() <= 0 ) {
        QSoftMenuBar::setLabel( table, Qt::Key_Select, "new", tr("New") );
    } else {
        QSoftMenuBar::setLabel( table, Qt::Key_Select, QSoftMenuBar::View);
    }
}

/*
   When marking a task as done, try to select a different task
   that is also not done (e.g. the one after this task, or before
   it if necessary).  If both the next and previous tasks are
   also done, we keep searching backwards...

   This facilitates the rote marking of all tasks as complete from
   any position in the list of incomplete tasks.


   next
   previous
   same
*/

void TodoWindow::markTaskDone()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    if (todo != QTask()) {
        todo.setStatus(QTask::Completed);

        QModelIndex origidx = model->index(todo.uid());
        QModelIndex idx = model->index(origidx.row() + 1, origidx.column());
        if (!idx.isValid() || model->task(idx).status() == QTask::Completed)
            idx = model->index(origidx.row() - 1, origidx.column());
        if (!idx.isValid() || model->task(idx).status() == QTask::Completed)
            idx = origidx;

        table->setCurrentIndex(idx);

        model->updateTask(todo);
        todo = model->task(todo.uid());
        if (centralView->currentIndex() == 1)
            showDetailView(todo);
        contextMenuActionsDirty = true;
    }
}

void TodoWindow::markTaskNotDone()
{
    QTask todo;
    if (centralView->currentIndex() == 1)
        todo = tView->task();
    if (todo.uid().isNull())
        todo = table->currentTask();

    if (todo != QTask() && todo.status() == QTask::Completed) {
        todo.setStatus(QTask::InProgress);

        QModelIndex origidx = model->index(todo.uid());
        QModelIndex idx = model->index(origidx.row() + 1, origidx.column());
        if (!idx.isValid() || model->task(idx).status() != QTask::Completed)
            idx = model->index(origidx.row() - 1, origidx.column());
        if (!idx.isValid() || model->task(idx).status() != QTask::Completed)
            idx = origidx;

        table->setCurrentIndex(idx);

        model->updateTask(todo);
        todo = model->task(todo.uid());
        if (centralView->currentIndex() == 1)
            showDetailView(todo);
        contextMenuActionsDirty = true;
    }
}

void TodoWindow::reload()
{
    model->refresh();
}

void TodoWindow::flush()
{
    model->flush();
}

void TodoWindow::catSelected( const QCategoryFilter &c )
{
    model->setCategoryFilter( c );
    if (c.acceptAll()) {
        categoryLbl->hide();
    } else {
        categoryLbl->setText(tr("Category: %1").arg(c.label(TodoCategoryScope)));
        categoryLbl->show();
    }
}

void TodoWindow::closeEvent( QCloseEvent *e )
{
    e->accept();
    QSettings config("Trolltech","todo");
    config.beginGroup( "View" );
    config.setValue( "ShowComplete", !model->filterCompleted() );
    model->categoryFilter().writeConfig(config, "Category");
}

void TodoWindow::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Back || e->key() == Qt::Key_No ) {
        e->accept();
        if ( centralView->currentIndex() == 1 )
            showListView();
        else
            close();
    } else
        QMainWindow::keyPressEvent(e);
}

bool TodoWindow::eventFilter( QObject *o, QEvent *e )
{
    if(o == table)
    {
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            if ( ke->key() == Qt::Key_Select) {
                if(table->currentIndex().isValid())
                    return false;
                else {
                    createNewEntry();
                    return true;
                }
            }
        }
    }
    return false;
}

void TodoWindow::setDocument( const QString &filename )
{
    QContent doc(filename);
    if ( doc.isValid() )
        receiveFile(doc.fileName());
    else
        receiveFile(filename);
}

bool TodoWindow::receiveFile( const QString &filename )
{
    QList<QTask> tl = QTask::readVCalendar( filename );

    if (tl.count() < 1) {
        // should spit out more appropriate message.
        QMessageBox::information(this, tr("New Tasks"),
                    tr("<p>Received empty task list.  No tasks added"),
                    QMessageBox::Ok);
        return false;
    }
    QString msg = tr("<P>%1 new tasks.<p>Do you want to add them to your Tasks?").
        arg(tl.count());

    if ( QMessageBox::information(this, tr("New Tasks"),
            msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
        for( QList<QTask>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
            model->addTask( *it );
        }
        return true;
    }
    return false;
}


void TodoWindow::beamCurrentEntry()
{
    if ( !table->currentIndex().isValid() ) {
        qWarning("todo::beamCurrentEntry called with nothing to beam");
        return;
    }

    QString description;

    ::unlink( beamfile.toLocal8Bit() ); // delete if exists
    QTask c = table->currentTask();
    if ( table->selectionMode() == TodoTable::ExtendedSelection ) {
        QList<QTask> l = table->selectedTasks();
        QTask::writeVCalendar( beamfile, l );

        if ( l.count() > 1 )
            description = tr("the %1 selected tasks").arg( l.count() );
        else
            description = c.description();
    } else {
        QTask::writeVCalendar( beamfile, c );
        description = c.description();
    }

    QFile f(beamfile);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data = f.readAll();
        QtopiaSendVia::sendData(this, data, "text/x-vcalendar");
    }
}

void TodoWindow::selectAll()
{
    table->selectAll();
}

void TodoWindow::selectCategory()
{
    if (!categoryDlg) {
        categoryDlg = new QCategoryDialog(TodoCategoryScope, QCategoryDialog::Filter | QCategoryDialog::SingleSelection, this);
        categoryDlg->setObjectName("Todo List");
    }
    categoryDlg->selectFilter(model->categoryFilter());

    if (QtopiaApplication::execDialog(categoryDlg) == QDialog::Accepted)
        catSelected(categoryDlg->selectedFilter());
}

void TodoWindow::qdlActivateLink( const QDSActionRequest& request )
{
    // Grab the link from the request and check that is one of ours
    QDLLink link( request.requestData() );
    if ( link.service() != "Tasks" ) {
        QDSActionRequest( request ).respond( tr( "Link doesn't belong to Tasks" ) );
        return;
    }
    const QByteArray dataRef = link.data();
    QDataStream refStream( dataRef );
    QUniqueId uid;
    refStream >> uid;
    if ( model->exists( uid ) ) {
        bool showDetails = true;
        QTask t = model->task(uid);

        // Check if we're already showing a task, if so push it onto the previous tasks
        // stack.  Otherwise, push an empty id, so we know that we didn't go to the
        // details through the list view
        if (centralView->currentIndex() == 1) {
            QTask current = todoView()->task();
            if (t != current) {
                prevTasks.push( todoView()->task().uid() );
            } else {
                showDetails = false;
            }
        } else {
            if(prevTasks.count() == 0)
                prevTasks.push(QUniqueId());
        }
        if (showDetails) {
            showDetailView(model->task(uid));
            showMaximized();
            raise();
        }
        QDSActionRequest( request ).respond();
    }
    else {
        QMessageBox::warning(
            this,
            tr("Tasks"),
            "<qt>" + tr("The selected task no longer exists.") + "</qt" );
        QDSActionRequest( request ).respond( "Task doesn't exist" );
    }
}

void TodoWindow::qdlRequestLinks( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    if ( model->count() == 0 ) {
        QMessageBox::warning(
            this,
            tr( "Tasks" ),
            "<qt>" + tr( "No tasks available." ) + "</qt>",
            QMessageBox::Ok );

        processingRequest.respond( "No tasks available." );

        return;
    }

    QTaskSelector *s = new QTaskSelector( false, ( isVisible() ? this : 0 ) );
    s->setModal( true );
    s->setModel(model);
    s->showMaximized();

    if( (s->exec() == QDialog::Accepted) && (s->taskSelected()) )
    {
        QTask task = s->selectedTask();
        QList<QDSData> links;
        links.push_back( taskQDLLink( task ) );

        QByteArray array;
        {
            QDataStream ds( &array, QIODevice::WriteOnly );
            ds << links;
        }

        processingRequest.respond( QDSData( array, QDLLink::listMimeType() ) );
    } else {
        processingRequest.respond( tr( "Task selection cancelled" ) );
    }
    delete s;
}

QDSData TodoWindow::taskQDLLink( QTask& task )
{
    // Check if we need to create the QDLLink
    QString keyString = task.customField( QDL::SOURCE_DATA_KEY );
    if ( keyString.isEmpty() ||
         !QDSData( QUniqueId( keyString ) ).isValid() ) {
        QByteArray dataRef;
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << task.uid();

        QDLLink link( "Tasks",
                      dataRef,
                      task.description(),
                      "pics/todolist/TodoList" );

        QDSData linkData = link.toQDSData();
        QUniqueId key = linkData.store();
        task.setCustomField( QDL::SOURCE_DATA_KEY, key.toString() );
        model->updateTask( task );

        return linkData;
    }

    // Get the link from the QDSDataStore
    return QDSData( QUniqueId( keyString ) );
}

void TodoWindow::removeTasksQDLLink( QList<QUniqueId>& taskIds )
{
    foreach( QUniqueId taskId, taskIds ) {
        QTask task = model->task( taskId );
        removeTaskQDLLink( task );
    }
}

void TodoWindow::removeTaskQDLLink( QTask& task )
{
    if ( task == QTask() )
        return;

    // Release any client QDLLinks
    QString links = task.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the task is a QDLLink source, if so break it
    QString key = task.customField( QDL::SOURCE_DATA_KEY );
    if ( !key.isEmpty() ) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();

        // Finally remove the stored key
        task.removeCustomField( QDL::SOURCE_DATA_KEY );
        model->updateTask( task );
    }
}

/*!
    \service TasksService Tasks
    \inpublicgroup QtPimModule
    \brief The TasksService class provides the Tasks service.

    The \i Tasks service enables applications to access features of
    the Tasks application.
*/

/*!
    \internal
*/
TasksService::~TasksService()
{
}

/*!
    Open a dialog to allow the user to create a new task.

    This slot corresponds to the QCop service message
    \c{Tasks::newTask()}.
*/
void TasksService::newTask()
{
    todo->createNewEntry(false);
}

/*!
    Add a new \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::addTask(QTask)}.
*/
void TasksService::addTask( const QTask& task )
{
    todo->model->addTask(task);
}

/*!
    Update an existing \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::updateTask(QTask)}.
*/
void TasksService::updateTask( const QTask& task )
{
    todo->model->updateTask(task);
}

/*!
    Remove an existing \a task.

    This slot corresponds to the QCop service message
    \c{Tasks::removeTask(QTask)}.
*/
void TasksService::removeTask( const QTask& task )
{
    QTask taskCopy( task );
    todo->removeTaskQDLLink( taskCopy );
    todo->model->removeTask(task);
}

/*!
    Show the task identified by \a uid.

    This slot corresponds to the QCop service message
    \c{Tasks::showTask(QUniqueId)}.
*/
void TasksService::showTask( const QUniqueId& uid )
{
    QTask t = todo->model->task(uid);
    if (t != QTask()) {
        if (todo->centralView->currentIndex() == 1) {
            // Only show it if it's different
            QTask current = todo->todoView()->task();
            if (t != current)
                todo->prevTasks.push( current.uid() );
            else
                return;
        } else {
            if(todo->prevTasks.count() == 0)
                todo->prevTasks.push(QUniqueId());
        }
        todo->showDetailView(t);
        todo->showMaximized();
        todo->activateWindow();
        todo->raise();
    }
}

/*!
    Activate the QDL link contained within \a request.

    The slot corresponds to a QDS service with a request data type of
    QDLLink::mimeType() and no response data.

    The slot corresponds to the QCop service message
    \c{Tasks::activateLink(QDSActionRequest)}.
*/
void TasksService::activateLink( const QDSActionRequest& request )
{
    todo->qdlActivateLink( request );
}

/*!
    Request for one or more QDL links using the hint contained within
    \a request.

    The slot corresponds to a QDS service with a request data type of
    "text/x-qstring" and response data type of QDLLink::listMimeType().

    The slot corresponds to the QCop service message
    \c{Tasks::requestLinks(QDSActionRequest)}.

*/
void TasksService::requestLinks( const QDSActionRequest& request )
{
    todo->qdlRequestLinks( request );
}

/*!
    Process any recurring task due date changes by calling
    QTaskModel::updateRecurringTasks().

    This slot corresponds to the QCop service message
    \c{Tasks::updateRecurringTasks(QDateTime,int)}.

    This message is usually generated at the appropriate times
    by the PIM library.  The \a datetime parameter describes
    when the updates are expected to be required, and the \a data
    parameter is currently unused.
*/
void TasksService::updateRecurringTasks(const QDateTime& datetime,int data)
{
    Q_UNUSED(datetime);
    Q_UNUSED(data);
    todo->model->updateRecurringTasks();
}
