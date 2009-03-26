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

#include "todoentryimpl.h"
#include "recurrencedetails.h"
#include "reminderpicker.h"
#include "qtopiatabwidget.h"

#include <qtask.h>
#include <QDL>
#include <QDLEditClient>
#include <QTextEdit>
#include <QTabWidget>
#include <qlayout.h>
#include <QScrollArea>
#include "qformlayout.h"
#include <QLineEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include "todocategoryselector.h"
#include <QDebug>
#include <QPushButton>
#include <QtopiaApplication>
#include <QAppointmentModel>
#include <QStackedLayout>
#include <QLabel>

TaskDialog::TaskDialog( const QTask& task, QWidget *parent,
        Qt::WFlags fl )
    : QDialog( parent, fl), todo( task ),
    inputNotes(0), inputNotesQC(0),
    dueCheck(0), startedCheck(0), completedCheck(0),
    dueEdit(0), startedEdit(0), completedEdit(0),
    inputDescription(0), comboPriority(0), comboStatus(0),
    spinComplete(0), comboCategory(0), recurDetails(0), reminderPicker(0),
    recurStack(0), recurControls(0)
{
    QUniqueId id = todo.dependentChildrenOfType(QString("duedate")).value(0);

    newTask = false;

    // grab the reminder defaults from Datebook
    bool setAlarm = true;

    {
        QSettings config("Trolltech","DateBook");
        config.beginGroup("Main");
        defaultReminderTime = QTime(config.value("startviewtime", 8).toInt(), 0);
        setAlarm = config.value("alarmpreset").toBool();
    }

    if (!id.isNull()) {
        QAppointmentModel am;
        todoAppt = am.appointment(id);

        // Preload this, since alarm minutes for NoAlarm aren't stored in the db
        if (todoAppt.alarm() == QAppointment::NoAlarm)
            todoAppt.setAlarm(-(defaultReminderTime.hour() * 60 + defaultReminderTime.minute()), QAppointment::NoAlarm);
    } else {
        todoAppt.setAlarm(-(defaultReminderTime.hour() * 60 + defaultReminderTime.minute()), setAlarm ? QAppointment::Audible : QAppointment::NoAlarm);
        todoAppt.setAllDay();
    }

    init();
}

/*
    Constructs a TaskDialog to create a new task.  The dialog
    will be a child of 'parent', with the widget flags set to 'f'.
    The initial task categories are set from 'categories'.

    The default reminder settings will be obtained from the
    datebook settings.

    The dialog will by default be modeless, unless you set 'modal' to
    true to construct a modal dialog.
*/
TaskDialog::TaskDialog(QList<QString> categories, QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl),
    inputNotes(0), inputNotesQC(0),
    dueCheck(0), startedCheck(0), completedCheck(0),
    dueEdit(0), startedEdit(0), completedEdit(0),
    inputDescription(0), comboPriority(0), comboStatus(0),
    spinComplete(0), comboCategory(0), recurDetails(0), reminderPicker(0),
    recurStack(0), recurControls(0)
{
    todo.setCategories(categories);

    newTask = true;
    // Since this is a new task, grab the reminder defaults from Datebook
    {
        QSettings config("Trolltech","DateBook");
        config.beginGroup("Main");
        defaultReminderTime = QTime(config.value("startviewtime", 8).toInt(), 0);
        todoAppt.setAlarm(-(config.value("startviewtime", 8).toInt() * 60), config.value("alarmpreset").toBool() ? QAppointment::Audible : QAppointment::NoAlarm);
    }
    todoAppt.setAllDay();

    /* may have to set up the 'empty' task. */
    init();
}

void TaskDialog::init()
{
    setWindowTitle( tr( "New Task" ) );

    QVBoxLayout *vl = new QVBoxLayout;
    QtopiaTabWidget *tw = new QtopiaTabWidget;

    vl->setSpacing( 3 );
    vl->setMargin( 0 );

    vl->addWidget(tw);

    // connected here, in case delayed loading is disabled.
    connect(tw, SIGNAL(prepareTab(int,QScrollArea*)), this, SLOT(initTab(int,QScrollArea*)));

    tw->addTab(QIcon(":icon/TodoList"), tr("Task"));
    tw->addTab(QIcon(":icon/day"), tr("Progress"));
    tw->addTab(QIcon(":icon/repeat"), tr("Recurrence"));
    tw->addTab(QIcon(":icon/addressbook/notes"), tr("Notes"));

    setLayout(vl);
}

void TaskDialog::initTab(int index, QScrollArea *parent)
{
    switch(index)
    {
        case 0:
            initTaskTab(parent);
            break;
        case 1:
            initProgressTab(parent);
            break;
        case 2:
            initRecurrenceTab(parent);
            break;
        case 3:
            initNotesTab(parent);
            break;
    }
}

void TaskDialog::initTaskTab(QScrollArea *scrollArea)
{
    /* construct */
    QWidget* taskDetail = new QWidget(scrollArea);
    QFormLayout *fl = new QFormLayout;

    inputDescription = new QLineEdit;

    comboPriority = new QComboBox;
    comboPriority->addItem(tr("1 - Very High"));
    comboPriority->addItem(tr("2 - High"));
    comboPriority->addItem(tr("3 - Normal"));
    comboPriority->addItem(tr("4 - Low"));
    comboPriority->addItem(tr("5 - Very Low"));

    dueCheck = new QGroupBox;
    dueCheck->setCheckable(true);
    dueCheck->setTitle(tr("Due:"));
    QFormLayout *duelayout = new QFormLayout;
    dueEdit = new QDateEdit;
    duelayout->addRow(tr("Due:"), dueEdit);
    reminderPicker = new ReminderPicker(this, duelayout, todoAppt);
    dueCheck->setLayout(duelayout);

    fl->addRow(tr("Desc."), inputDescription);
    fl->addRow(tr("Priority"), comboPriority);
    fl->addRow(dueCheck);

    taskDetail->setLayout(fl);

    /* initialize */
    inputDescription->setText(todo.description());
    comboPriority->setCurrentIndex( todo.priority() - 1 );
    if (todo.dueDate().isValid()) {
        dueCheck->setChecked(true);
        dueEdit->setDate(todo.dueDate());
    } else {
        dueCheck->setChecked(false);
        dueEdit->setDate(QDate::currentDate());
    }

    reminderPicker->updateUI(todo.dueDate().isValid());

    /* connect */
    connect( dueEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(dueDateChanged(QDate)) );
    connect( dueCheck, SIGNAL(toggled(bool)),
            this, SLOT(dueDateChecked(bool)) );

    scrollArea->setWidget(taskDetail);
    taskDetail->setFocusPolicy(Qt::NoFocus);
}

void TaskDialog::initProgressTab(QScrollArea *scrollArea)
{
    /* construct */
    QWidget *progressDetail = new QWidget(scrollArea);
    QFormLayout *fl = new QFormLayout;

    comboStatus = new QComboBox;
    comboStatus->addItem(tr("Not Started"));
    comboStatus->addItem(tr("In Progress"));
    comboStatus->addItem(tr("Completed"));
    comboStatus->addItem(tr("Waiting"));
    comboStatus->addItem(tr("Deferred"));

    spinComplete = new QSpinBox;
    spinComplete->setMinimum(0);
    spinComplete->setMaximum(100);
    spinComplete->setSuffix(tr("%"));

    startedCheck = new QGroupBox;
    startedCheck->setCheckable(true);
    startedCheck->setTitle(tr("Started:"));

    completedCheck = new QGroupBox;
    completedCheck->setCheckable(true);
    completedCheck->setTitle(tr("Completed:"));

    startedEdit = new QDateEdit;
    completedEdit = new QDateEdit;

    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(startedEdit);
    startedCheck->setLayout(vl);

    vl = new QVBoxLayout();
    vl->addWidget(completedEdit);
    completedCheck->setLayout(vl);

    fl->addRow(tr("Status"), comboStatus);
    fl->addRow(tr("Progress"), spinComplete);
    fl->addRow(startedCheck);
    fl->addRow(completedCheck);

    progressDetail->setLayout(fl);

    /* initialize */
    QDate current = QDate::currentDate();
    comboStatus->setCurrentIndex( todo.status() );

    spinComplete->setValue(todo.percentCompleted());

    if ( !todo.startedDate().isNull() ) {
        startedCheck->setChecked(true);
        startedEdit->setDate(todo.startedDate());
    } else {
        startedCheck->setChecked(false);
        startedEdit->setDate(current);
    }

    if ( todo.isCompleted() ) {
        completedCheck->setChecked(true);
        completedEdit->setDate(todo.completedDate());
    } else {
        completedCheck->setChecked(false);
        completedEdit->setDate(current);
    }

    /* connect */
    connect( startedEdit, SIGNAL(dateChanged(QDate)),
             this, SLOT(startDateChanged(QDate)) );
    connect( completedEdit, SIGNAL(dateChanged(QDate)),
             this, SLOT(endDateChanged(QDate)) );
    connect( comboStatus, SIGNAL(activated(int)),
            this, SLOT(statusChanged(int)) );
    connect( startedCheck, SIGNAL(toggled(bool)),
            this, SLOT(startDateChecked(bool)) );
    connect( completedCheck, SIGNAL(toggled(bool)),
            this, SLOT(endDateChecked(bool)) );
    connect( spinComplete, SIGNAL(valueChanged(int)),
            this, SLOT(percentChanged(int)) );

    scrollArea->setWidget(progressDetail);
    progressDetail->setFocusPolicy(Qt::NoFocus);
}

void TaskDialog::initRecurrenceTab(QScrollArea *scrollArea)
{
    /* construct */
    QWidget *recurrenceDetail = new QWidget(scrollArea);
    recurStack = new QStackedLayout;

    recurControls = new QWidget();
    // todoAppt passed in non-const
    recurDetails = new RecurrenceDetails(todoAppt);
    QFormLayout *fl = new QFormLayout();
    recurDetails->initGui(fl);
    recurControls->setLayout(fl);

    QLabel *noRecurLabel = new QLabel(tr("Add a due date to this task to allow recurrence."));
    noRecurLabel->setWordWrap(true);
    noRecurLabel->setFocusPolicy(Qt::StrongFocus);

    recurStack->addWidget(recurControls);
    recurStack->addWidget(noRecurLabel);

    recurrenceDetail->setLayout(recurStack);

    /* init */
    recurStack->setCurrentIndex(todo.hasDueDate() ? 0 : 1);
    recurDetails->updateUI();

    /* connect - no connections */

    scrollArea->setWidget(recurrenceDetail);
    recurrenceDetail->setFocusPolicy(Qt::NoFocus);
}

void TaskDialog::initNotesTab(QScrollArea *scrollArea)
{
    /* construct */
    QWidget* noteDetail = new QWidget(scrollArea);
    QFormLayout *fl = new QFormLayout;

    inputNotes = new QTextEdit( );
    inputNotes->setLineWrapMode( QTextEdit::WidgetWidth );
    QFontMetrics fmTxtNote( inputNotes->font() );
    inputNotes->setFixedHeight( fmTxtNote.height() * 5 );

    inputNotesQC = new QDLEditClient( inputNotes, "qdlnotes" );
    inputNotesQC->setupStandardContextMenu();

    comboCategory = new TodoCategorySelector;

    fl->addRow(tr("Notes"), inputNotes);
    fl->addRow(tr("Category"), comboCategory);

    noteDetail->setLayout(fl);

    /* init */
    comboCategory->selectCategories(todo.categories());
    inputNotes->setHtml( todo.notes() );
    QDL::loadLinks( todo.customField( QDL::CLIENT_DATA_KEY ),
                    QDL::clients( inputNotes ) );
    inputNotesQC->verifyLinks();

    /* connect - no connections */
    scrollArea->setWidget(noteDetail);
    noteDetail->setFocusPolicy(Qt::NoFocus);
}

void TaskDialog::startDateChecked(bool checked)
{
    if (checked) {
        todo.setStartedDate(startedEdit->date());
    } else {
        todo.setStartedDate(QDate());
    }
    updateFromTask();
}

void TaskDialog::endDateChecked(bool checked)
{
    if (checked)
        todo.setCompletedDate(completedEdit->date());
    else
        todo.setCompletedDate(QDate());
    updateFromTask();
}

void TaskDialog::dueDateChecked(bool checked)
{
    if (checked)
        todo.setDueDate(dueEdit->date());
    else
        todo.setDueDate(QDate());
    updateFromTask();
}

void TaskDialog::percentChanged(int percent)
{
    int oldpercent = todo.percentCompleted();
    todo.setPercentCompleted(percent);

    // Try to avoid doing a lot of computation here.
    // only update when we make a transition that is meaningful
    // (e.g. 0 to non zero, or 100 to non 100)
    // so that people who like to spin non stop can do so..
    // The date controls don't call updateFromTask when they change..
    if (!spinComplete->hasEditFocus() || oldpercent == 0 || oldpercent == 100 || percent == 0 || percent == 100) {
        updateFromTask();
    }
}

void TaskDialog::statusChanged(int status)
{
    todo.setStatus(status);
    updateFromTask();
}

void TaskDialog::updateFromTask()
{
    int p = todo.percentCompleted();
    int s = todo.status();
    QDate sDate = todo.startedDate();
    QDate cDate = todo.completedDate();

    if (spinComplete) {
        // implies the others in this if statement as on the
        // same tab
        spinComplete->blockSignals(true);
        startedCheck->blockSignals(true);
        completedCheck->blockSignals(true);
        startedEdit->blockSignals(true);
        completedEdit->blockSignals(true);
        comboStatus->blockSignals(true);

        spinComplete->setValue(p);
        comboStatus->setCurrentIndex(s);
        startedCheck->setChecked(sDate.isValid());
        completedCheck->setChecked(cDate.isValid());

        if (sDate.isValid())
            startedEdit->setDate(sDate);
        if (cDate.isValid())
            completedEdit->setDate(cDate);

        spinComplete->blockSignals(false);
        startedCheck->blockSignals(false);
        completedCheck->blockSignals(false);
        startedEdit->blockSignals(false);
        completedEdit->blockSignals(false);
        comboStatus->blockSignals(false);
    }

    if (recurStack)
        recurStack->setCurrentIndex(todo.dueDate().isValid() ? 0 : 1);
    if (recurDetails)
        recurDetails->updateUI();
    if(reminderPicker)
        reminderPicker->updateUI(todo.dueDate().isValid());
}

/*
    Destroys the object and frees any allocated resources
*/
TaskDialog::~TaskDialog()
{
}

void TaskDialog::startDateChanged( const QDate& date )
{
    todo.setStartedDate(date);
    if ( date > completedEdit->date() )
        completedEdit->setDate( date );
}

void TaskDialog::endDateChanged( const QDate& date )
{
    todo.setCompletedDate(date);
    if ( date < startedEdit->date() )
        startedEdit->setDate( date );
}

void TaskDialog::dueDateChanged( const QDate& date )
{
    todo.setDueDate(date);
    if (recurDetails)
        recurDetails->updateAppointment();
    todoAppt.setStart(QDateTime(todo.dueDate()));
    todoAppt.setEnd(QDateTime(todo.dueDate()));
    todoAppt.setAllDay(true);

    updateFromTask();
}

const QTask &TaskDialog::todoEntry() const
{
    /* task tab */
    if (inputDescription) {
        todo.setDescription( inputDescription->text() );
        todo.setPriority( (QTask::Priority) (comboPriority->currentIndex() + 1) );
        if (dueCheck->isChecked())
            todo.setDueDate( dueEdit->date() );
        else
            todo.clearDueDate();
    }

    /* progress tab */
    if (spinComplete) {
        todo.setStatus(comboStatus->currentIndex());
        todo.setPercentCompleted(spinComplete->value());

        if (startedCheck->isChecked())
            todo.setStartedDate( startedEdit->date() );
        else
            todo.setStartedDate( QDate() );

        if (completedCheck->isChecked())
            todo.setCompletedDate( completedEdit->date() );
        else
            todo.setCompletedDate( QDate() );
    }

    /* recurrence tab */

    /* notes tab */
    if (inputNotes) {
        todo.setCategories( comboCategory->selectedCategories() );

        /* changing to plain text until we have a more efficient import export format */
        if (inputNotes->toPlainText().simplified().isEmpty())
            todo.setNotes(QString());
        else
            todo.setNotes( inputNotes->toHtml() );
        // XXX should load links here
        QString links;
        QDL::saveLinks( links, QDL::clients( inputNotes ) );
        todo.setCustomField( QDL::CLIENT_DATA_KEY, links );
    }

    return todo;
}

const QAppointment &TaskDialog::todoAppointment() const
{
    if (recurDetails)
        recurDetails->updateAppointment();
    return todoAppt;
}

void TaskDialog::accept()
{
    emit taskEditAccepted(todoEntry(), todoAppointment());
    QDialog::accept();
}


