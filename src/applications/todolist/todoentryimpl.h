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

#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H

#include <qtask.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qwidget.h>

#include <qappointment.h>

class QTextEdit;
class QScrollArea;
class QDLEditClient;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QDateEdit;
class QComboBox;
class QLabel;
class TodoCategorySelector;
class RecurrenceDetails;
class ReminderPicker;
class QStackedLayout;

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    TaskDialog( const QTask &task, QWidget *parent = 0, Qt::WFlags fl = 0 );
    TaskDialog( QList<QString> categories, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~TaskDialog();

    const QTask &todoEntry() const;
    const QAppointment &todoAppointment() const;

    void accept();

signals:
    void categoriesChanged();
    void taskEditAccepted(const QTask&, const QAppointment&);

protected slots:
    void startDateChanged( const QDate& );
    void endDateChanged( const QDate& );
    void dueDateChanged( const QDate& );

    void startDateChecked(bool);
    void endDateChecked(bool);
    void dueDateChecked(bool);

    void percentChanged(int);
    void statusChanged(int);

    void updateFromTask();

    void initTab(int, QScrollArea *parent);

private:
    void init();
    void initTaskTab(QScrollArea *);
    void initProgressTab(QScrollArea *);
    void initRecurrenceTab(QScrollArea *);
    void initNotesTab(QScrollArea *);

    mutable QTask todo;
    mutable QAppointment todoAppt;

    QTextEdit *inputNotes;
    QDLEditClient *inputNotesQC;

    QTime defaultReminderTime;
    QGroupBox *dueCheck, *startedCheck, *completedCheck;
    QDateEdit *dueEdit, *startedEdit, *completedEdit;
    QLineEdit *inputDescription;
    QComboBox *comboPriority, *comboStatus;
    QSpinBox *spinComplete;
    TodoCategorySelector *comboCategory;
    RecurrenceDetails *recurDetails;
    ReminderPicker *reminderPicker;
    QStackedLayout *recurStack;
    QWidget *recurControls;
    bool newTask;
};

#endif
