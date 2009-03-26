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

#include "entrydialog.h"
#include "datebookcategoryselector.h"
#include "../todolist/qtopiatabwidget.h"

#include <qtopiaapplication.h>
#include <QTimeZoneSelector>

#include <QDL>
#include <QDLEditClient>

#include <QStyle>
#include <QLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QScrollArea>
#include <QDateEdit>
#include <QTimeEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>

#include "qtopia/qtimezoneselector.h"

#include "../todolist/reminderpicker.h"
#include "../todolist/recurrencedetails.h"

//------------------------------------------------------------------------------

EntryDialog::EntryDialog( bool startOnMonday, const QAppointment &appointment, const QTime& defaultAllDayReminder, int defaultTimedReminder,
                          QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ), mAppointment(appointment), mOrigAppointment( appointment ),
    startWeekOnMonday( startOnMonday ), allDayReminder(defaultAllDayReminder), timedReminder(defaultTimedReminder), 
    comboCategory(0), recurDetails(0), reminderPicker(0), editNote(0),
    editnoteQC(0), mDescription(0), mLocation(0), checkAllDay(0),
    startDate(0), endDate(0), startTime(0), endTime(0), startTimeLabel(0),
    endTimeLabel(0), timezone(0)
{
    init();
}

/*
    Destroys the object and frees any allocated resources
*/
EntryDialog::~EntryDialog()
{
}

void EntryDialog::init()
{
    setObjectName( "edit-event" );

    QVBoxLayout *vl = new QVBoxLayout;
    QtopiaTabWidget *tw = new QtopiaTabWidget;
    vl->setSpacing( 3 );
    vl->setMargin( 0 );

    vl->addWidget(tw);

    // connected here, in case delayed loading is disabled.
    connect(tw, SIGNAL(prepareTab(int,QScrollArea*)), this, SLOT(initTab(int,QScrollArea*)));

    tw->addTab( QIcon(":icon/DateBook"), tr("Event") );
    tw->addTab( QIcon(":icon/repeat"), tr("Recurrence"));
    tw->addTab( QIcon(":icon/addressbook/notes"), tr("Notes") );

    setLayout(vl);
}


void EntryDialog::initTab(int index, QScrollArea *sc)
{
    switch(index)
    {
        case 0:
            initEventDetails(sc);
            break;
        case 1:
            initRepeatDetails(sc);
            break;
        case 2:
            initNoteDetails(sc);
            break;
    }
}

void EntryDialog::initEventDetails(QScrollArea *sc)
{
    /* construct */
    QWidget *eventTab = new QWidget;
    QFormLayout *fl = new QFormLayout;

    mDescription = new QLineEdit();
    fl->addRow(tr("Desc."), mDescription);

    // Location
    mLocation = new QLineEdit();
    fl->addRow(tr("Loc."), mLocation);

    QHBoxLayout *hb = new QHBoxLayout();
    checkAllDay = new QCheckBox(tr("All day event"));
    hb->addStretch();
    hb->addWidget(checkAllDay);
    hb->addStretch();

    fl->addRow(hb);

    startDate = new QDateEdit();
    startTime = new QTimeEdit();
    startTime->setWrapping(true);
    startTimeLabel = new QLabel(tr("Time"));
    endDate = new QDateEdit();
    endTime = new QTimeEdit();
    endTime->setWrapping(true);
    endTimeLabel = new QLabel(tr("Time"));

    QGroupBox *groupbox = new QGroupBox();
    QFormLayout *gfl = new QFormLayout();
    gfl->addRow(tr("Date"), startDate);
    gfl->addRow(startTimeLabel, startTime);
    groupbox->setTitle(tr("Start"));
    groupbox->setLayout(gfl);

    fl->addRow(groupbox);

    groupbox = new QGroupBox();
    gfl = new QFormLayout();
    gfl->addRow(tr("Date"), endDate);
    gfl->addRow(endTimeLabel, endTime);
    groupbox->setTitle(tr("End"));
    groupbox->setLayout(gfl);

    fl->addRow(groupbox);

    // timezone
    timezone = new QTimeZoneSelector();
    timezone->setAllowNoZone(true);
    timezone->setCurrentZone("None");

    fl->addRow(tr("T.Z.", "short string for Time Zone"), timezone);

    // Reminder (stick it in an anonymous groupbox)
    groupbox = new QGroupBox();
    gfl = new QFormLayout();
    reminderPicker = new ReminderPicker(this, gfl, mAppointment);
    groupbox->setLayout(gfl);
    fl->addRow(groupbox);

    eventTab->setLayout(fl);

    /* init */

    if (mAppointment.timeZone().isValid()) {
        timezone->setCurrentZone(mAppointment.timeZone().id());
    }

    setDates(mAppointment.start(),mAppointment.end());
    mDescription->setText(mAppointment.description());
    mLocation->setText(mAppointment.location());

    checkAllDay->setChecked( mAppointment.isAllDay() );

    updateTimeUI();

    /* connect */
    connect( startDate, SIGNAL(dateChanged(QDate)),
             this, SLOT(updateStartDateTime()));
    connect( startTime, SIGNAL(timeChanged(QTime)),
             this, SLOT(updateStartTime()));
    connect( startTime, SIGNAL(editingFinished()),
             this, SLOT(updateStartTime()));
    connect( endDate, SIGNAL(dateChanged(QDate)),
             this, SLOT(updateEndDateTime()));
    connect( endTime, SIGNAL(timeChanged(QTime)),
             this, SLOT(updateEndTime()));
    connect( endTime, SIGNAL(editingFinished()),
             this, SLOT(updateEndTime()));

    connect( checkAllDay, SIGNAL(stateChanged(int)),
            this, SLOT(updateTimeUI()));

    connect( qApp, SIGNAL(weekChanged(bool)),
             this, SLOT(setWeekStartsMonday(bool)) );

    sc->setWidget(eventTab);
    eventTab->setFocusPolicy(Qt::NoFocus);
}

void EntryDialog::initRepeatDetails(QScrollArea *sc)
{
    /* construct */
    QWidget *repeatTab = new QWidget;
    QFormLayout *fl = new QFormLayout();

    recurDetails = new RecurrenceDetails(mAppointment);
    recurDetails->initGui(fl);
    repeatTab->setLayout(fl);

    /* init */
    recurDetails->updateUI();

    /* if connections, put them here */

    sc->setWidget(repeatTab);
    repeatTab->setFocusPolicy(Qt::NoFocus);
}

void EntryDialog::initNoteDetails(QScrollArea *sc)
{
    /* construct */
    QWidget *noteTab = new QWidget();
    QFormLayout *fl = new QFormLayout();

    editNote = new QTextEdit( noteTab );
    editNote->setLineWrapMode( QTextEdit::WidgetWidth );
    QFontMetrics fmTxtNote( editNote->font() );
    editNote->setFixedHeight( fmTxtNote.height() * 5 );

    editnoteQC = new QDLEditClient( editNote, "editnote" );
    editnoteQC->setupStandardContextMenu();

    comboCategory = new DateBookCategorySelector();
    fl->addRow( tr("Notes"), editNote );
    fl->addRow( tr("Category"), comboCategory);

    noteTab->setLayout(fl);
    /* init */
    comboCategory->selectCategories( mAppointment.categories() );

    if(!mAppointment.notes().isEmpty())
        editNote->setHtml(mAppointment.notes()); //PlainText(mAppointment.notes());


    QDL::loadLinks( mAppointment.customField( QDL::CLIENT_DATA_KEY ),
                    QDL::clients( this ) );
    editnoteQC->verifyLinks();

    /* if connections, put them here */
    sc->setWidget(noteTab);
    noteTab->setFocusPolicy(Qt::NoFocus);
}


void EntryDialog::setDates( const QDateTime& s, const QDateTime& e )
{
    mAppointment.setStart(s);
    mAppointment.setEnd(e);
    startTime->setTime(s.time());
    startDate->setDate(s.date());
    endTime->setTime(e.time());
    endDate->setDate(e.date());
}

/*
    public slot
*/
void EntryDialog::updateEndTime()
{
    // Filter time edits, only process the time when we are not in edit mode
    if ( !endTime->hasEditFocus() ) {
        updateEndDateTime();
    }
}

/*
    public slot
*/
void EntryDialog::updateEndDateTime()
{
    startDate->blockSignals(true);
    startTime->blockSignals(true);
    endDate->blockSignals(true);
    endTime->blockSignals(true);

    // TODO - update recurrence details enddate API

    QDateTime target = QDateTime(endDate->date(), endTime->time());

    // since setting the start can change the end, do this first.
    if (target.addSecs(-300) < mAppointment.start()) {
        mAppointment.setStart(target.addSecs(-300));
    }

    mAppointment.setEnd(target);
    startDate->setDate(mAppointment.start().date());
    startTime->setTime(mAppointment.start().time());

    if (recurDetails)
        recurDetails->updateUI();

    startDate->blockSignals(false);
    startTime->blockSignals(false);
    endDate->blockSignals(false);
    endTime->blockSignals(false);
}

/*
    public slot
*/
void EntryDialog::updateStartTime()
{
    // Filter time edits, only process the time when we are not in edit mode
    if ( !startTime->hasEditFocus() ) {
        updateStartDateTime();
    }
}

/*
    public slot
*/
void EntryDialog::updateStartDateTime()
{
    startDate->blockSignals(true);
    startTime->blockSignals(true);
    endDate->blockSignals(true);
    endTime->blockSignals(true);

    // start always works.
    QDateTime s = QDateTime(startDate->date(), startTime->time());

    mAppointment.setStart(s);

    // modifying start modifies end, so no need check or modify anything.
    // just ensure we update the widget.
    endDate->setDate(mAppointment.end().date());
    endTime->setTime(mAppointment.end().time());

    if (recurDetails)
        recurDetails->updateUI();

    startDate->blockSignals(false);
    startTime->blockSignals(false);
    endDate->blockSignals(false);
    endTime->blockSignals(false);
}

void EntryDialog::setWeekStartsMonday( bool onMonday )
{
    startWeekOnMonday = onMonday;
}

QAppointment EntryDialog::appointment( const bool includeQdlLinks )
{
    // first tab widgets.
    if (mDescription) {
        mAppointment.setDescription( mDescription->text() );
        mAppointment.setLocation( mLocation->text() );
        mAppointment.setAllDay( checkAllDay->isChecked() );

        // Since we don't always update start/end time
        // if the changes happen with edit focus, and
        // the controls don't lose focus until after this is called
        // (to emit editingFinished), force an update
        if (startTime->hasFocus())
            updateStartDateTime();
        if (endTime->hasFocus())
            updateEndDateTime();

        // don't set the time if theres no need too
        if (timezone->currentZone() == "None")
            mAppointment.setTimeZone(QTimeZone());
        else
            mAppointment.setTimeZone(QTimeZone(timezone->currentZone().toAscii().constData()));
    }

    // third tab widgets
    if (editNote) {
        if ( editNote->toPlainText().isEmpty() )
            mAppointment.setNotes( QString() );
        else
            mAppointment.setNotes( editNote->toHtml() );

        if ( includeQdlLinks ) {
            QString links;
            QDL::saveLinks( links, QDL::clients( this ) );
            mAppointment.setCustomField( QDL::CLIENT_DATA_KEY, links );
        }
        mAppointment.setCategories( comboCategory->selectedCategories() );
    }

    if (recurDetails)
        recurDetails->updateAppointment();

    return mAppointment;
}

void EntryDialog::updateTimeUI()
{
    if (checkAllDay->checkState() == Qt::Checked) {
        startTime->hide();
        startTimeLabel->hide();
        endTime->hide();
        endTimeLabel->hide();
    } else {
        startTime->show();
        startTimeLabel->show();
        endTime->show();
        endTimeLabel->show();
    }

    if (mAppointment.isAllDay() != checkAllDay->isChecked()) {
        mAppointment.setAllDay( checkAllDay->isChecked() );

        // Try to keep the days/weeks before portion of all day/timed events the same
        int daysportion = (24 * 60) * (timedReminder / (24 * 60));

        // Since we've toggled the state, reset the reminder
        if (mAppointment.isAllDay())
            mAppointment.setAlarm(daysportion - (allDayReminder.hour() * 60 + allDayReminder.minute()), mAppointment.alarm());
        else
            mAppointment.setAlarm(timedReminder, mAppointment.alarm());
    }

    reminderPicker->updateUI();

    // Force this now, since otherwise it can take an event loop
    // and you can see the change in state
    startTime->parentWidget()->layout()->activate();
    endTime->parentWidget()->layout()->activate();
}

void EntryDialog::accept()
{
    // see if anything changed - if not, just close
    if ( appointment( false ) == mOrigAppointment ) {
        hide();
        QDialog::reject();
        return;
    }

    // otherwise, see if we now have an empty description
    if ( mDescription && mDescription->text().isEmpty() )
    {
        if (QMessageBox::warning(this, tr("New Event"),
                    tr("<qt>An event description is required. Cancel editing?</qt>"),
                    QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            QDialog::reject();
            return;
        } else {
            mDescription->setFocus();
            return;
        }

    }

    // Otherwise, we're done
    hide();
    appointment();
    QDialog::accept();
}
