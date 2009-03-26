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

#include "datebook.h"
#include "dayview.h"
#include "monthview.h"
#include "datebookcategoryselector.h"

#include "appointmentdetails.h"
#include "appointmentpicker.h"
#include "datebooksettings.h"
#include "exceptiondialog.h"
#include "entrydialog.h"
#include "alarmview.h"
#include "finddialog.h"
#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
#include "accounteditor.h"
#endif

#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qappointment.h>
#include <qappointmentmodel.h>
#include <qpimsourcedialog.h>

#include <QDL>
#include <QDLLink>
#include <QDSActionRequest>
#include <QDSData>
#include <QDebug>

#include <QtopiaServiceRequest>

#include <QSettings>
#include <QTimer>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QRegExp>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QMenu>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <qtopiasendvia.h>
#include <qtopiaipcenvelope.h>
#include <qcontent.h>
#include <qtopia/qsoftmenubar.h>

static const unsigned int   DATEBOOK_UPDATE_ICON_TIMEOUT    =   500;

DateBook::DateBook(QWidget *parent, Qt::WFlags f)
: QMainWindow(parent, f),
  editorView(0),
  categoryDialog(0),
  exceptionDialog(0),
  aPreset(QAppointment::NoAlarm),
  presetTime(-1),
  startTime(8), // an acceptable default
  compressDay(true),
  syncing(false),
  closeAfterView(0),
  updateIconsTimer(0),
  prevOccurrences(),
  actionBeam(0)
{
    QtopiaApplication::loadTranslations( QLatin1String("libqtopiapim") );
    init();

    if ( updateIconsTimer )
        updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );
}

DateBook::~DateBook()
{
}

void DateBook::selectToday()
{
    lastToday = QDate::currentDate();
    if (viewStack->currentWidget() == dayView) {
        viewDay(lastToday);
    } else if (viewStack->currentWidget() == monthView) {
        monthView->setSelectedDate( lastToday );
    }
}

void DateBook::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay(lastToday);
}

void DateBook::viewDay()
{
    viewDay(currentDate());
}

void DateBook::viewDay(const QDate& d)
{
    initDayView();
    dayView->selectDate(d);
    raiseView(dayView);

    updateIcons();
}

void DateBook::viewMonth()
{
    viewMonth(currentDate());
}

void DateBook::viewMonth(const QDate& d)
{
    initMonthView();
    raiseView(monthView);
    monthView->setSelectedDate(d);

    updateIcons();
}

void DateBook::nextView()
{
    QWidget* cur = viewStack->currentWidget();
    if (cur) {
        if (cur == dayView)
            viewMonth();
        else if (cur == monthView)
            viewDay();
    }
}

void DateBook::unfoldAllDay()
{
    if (dayView)
        dayView->setAllDayFolded(false);

    updateIcons();
}

void DateBook::foldAllDay()
{
    if (dayView)
        dayView->setAllDayFolded(true);

    updateIcons();
}

void DateBook::checkToday()
{
    if (lastToday != QDate::currentDate()) {
        if (lastToday == currentDate())
            selectToday();
        else
            lastToday = QDate::currentDate();
    }

    midnightTimer->start((QTime::currentTime().secsTo(QTime(23, 59, 59)) + 3) * 1000);
}

void DateBook::showSettings()
{
    QSettings config("Trolltech", "qpe");
    config.beginGroup("Time");
    bool whichclock = config.value("AMPM").toBool();

    DateBookSettings frmSettings(whichclock, this);
    frmSettings.setStartTime(startTime);
    frmSettings.setPresetAlarm(aPreset, presetTime);
    frmSettings.setDefaultView(defaultView);

    if (QtopiaApplication::execDialog(&frmSettings)) {
        aPreset = frmSettings.alarmType();
        presetTime = frmSettings.alarmDelay();
        startTime = frmSettings.startTime();
        defaultView = frmSettings.defaultView();

        int endTime = qMin(qMax(startTime + 8, 17), 24);

        if (dayView)
            dayView->setDaySpan( startTime, endTime );

        saveSettings();
    }
}

void DateBook::showAccountSettings()
{
    /* TODO possible not a dialog in that cancel might not make sense. */
#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
    QDialog diag;
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    AccountEditor *editor = new AccountEditor;
    editor->setModel(model);
    vl->addWidget(editor);
    diag.setWindowTitle(tr("Accounts", "window title" ));
    diag.setLayout(vl);
    diag.setObjectName("accounts");
    diag.showMaximized();
    QtopiaApplication::execDialog(&diag);
#endif
}

void DateBook::changeClock()
{
}

void DateBook::changeWeek(bool m)
{
    /* no need to redraw, each widget catches.  Do need to
    store though for widgets we haven't made yet */
    onMonday = m;
}

QSet<QPimSource> DateBook::addressbookSources()
{
    static QSet<QPimSource> sources;

    if (sources.count() == 0) {
        // We remove sources from other applications
        sources = model->availableSources();
        QMutableSetIterator<QPimSource> it(sources);

        // We only allow the default and google contexts, for now...
        static QUuid googleContext("672cd357-c984-40e2-b47d-ffde5a65137c");
        QUuid defContext = model->defaultSource().context;

        while(it.hasNext()) {
            QPimSource p = it.next();
            if (p.context != googleContext && p.context != defContext)
                it.remove();
        }
    }
    return sources;
}


void DateBook::qdlActivateLink( const QDSActionRequest& request )
{
    // If we are currently viewing an appointment, chuck it on the
    // previous appointments stack
    if ( appointmentDetails &&
         viewStack->currentWidget() == appointmentDetails ) {
        prevOccurrences.push( appointmentDetails->occurrence() );
    }

    // Grab the link from the request and check that is one of ours
    QDLLink link( request.requestData() );
    if ( link.service() != "Calendar" ) {
        QDSActionRequest( request ).respond( "Link doesn't belong to Calendar" );
        return;
    }

    const QByteArray data = link.data();
    QDataStream refStream( data );
    QUniqueId u;
    QDate date;
    refStream >> u >> date;

    QAppointment a = model->appointment( u );
    if ( a.isValid() ) {
        if (!date.isValid())
            date = a.start().date();
        initDayView();
        showAppointmentDetails( QOccurrence( date, a ) );
        QDSActionRequest( request ).respond();
    } else {
        QMessageBox::warning(
            this,
            tr("Calendar"),
            "<qt>" + tr("The selected event no longer exists.") + "</qt");
        QDSActionRequest( request ).respond( "Event doesn't exist" );
    }

}

void DateBook::qdlRequestLinks( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    AppointmentPicker evtPick( this, addressbookSources(), this );
    evtPick.setModal( true );
    evtPick.showMaximized();
    if ( evtPick.exec() ) {
        if ( !evtPick.appointmentSelected() ) {
            processingRequest.respond( "No Event Selected" );
        } else {
            QList<QDSData> links;
            QAppointment appointment = evtPick.currentAppointment();
            QOccurrence o(evtPick.currentDate(), appointment);
            links.push_back( occurrenceQDLLink(o) );

            QByteArray array;
            {
                QDataStream ds( &array, QIODevice::WriteOnly );
                ds << links;
            }

            processingRequest.respond(
                QDSData( array, QDLLink::listMimeType() ) );
        }
    } else {
        processingRequest.respond( "Event selection cancelled" );
    }
}

QDSData DateBook::occurrenceQDLLink( const QOccurrence& occ )
{
    if ( occ == QOccurrence() )
        return QDSData();

    QAppointment appointment = occ.appointment();

    // Check if we need to create the QDLLink
    QString keyString = appointment.customField( QDL::SOURCE_DATA_KEY );
    QHash<int, QString> keyhash;

    // We actually store a date->keystring mapping, since we can
    // have multiple QDL links to the same appointment if it has
    // repetitions (but links to different dates)
    if (!keyString.isEmpty()) {
        // Just in case we did store a string in a previous version..
        QUniqueId u(keyString);
        if (!u.isNull()) {
            // Don't bother creating a new style of link, we don't know the day
            // and recreating the qdsdata is a pain.
            // Also note that in the case of a nonrecurring appointment,
            // the start().date() is the same as the given occurrence
            // date, and we don't end up storing the new link as a QHash
            keyhash[appointment.start().date().toJulianDay()] = keyString;
        } else {
            QByteArray ba = QByteArray::fromBase64(keyString.toAscii());
            QDataStream ds(ba);
            ds >> keyhash;
        }
    }

    // See if we've previously used this one
    if (keyhash.contains(occ.date().toJulianDay())) {
        QDSData linkData(QUniqueId(keyhash.value(occ.date().toJulianDay())));
        if (linkData.isValid())
            return linkData;
    }

    // No (or invalid), so create a new entry
    QByteArray dataRef;
    {
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << occ.uid() << occ.date();
    }

    QDLLink link( "Calendar",
            dataRef,
            occ.description(),
            QString( "pics/datebook/DateBook" ) );

    QDSData linkData = link.toQDSData();
    QUniqueId key = linkData.store();

    // Update the hash
    keyhash[occ.date().toJulianDay()] = key.toString();

    // and reconvert the hash to base64 etc.
    QByteArray ba;
    {
        QDataStream newhash( &ba, QIODevice::WriteOnly);
        newhash << keyhash;
    }

    appointment.setCustomField( QDL::SOURCE_DATA_KEY, ba.toBase64() );
    model->updateAppointment( appointment );

    return linkData;
}

void DateBook::removeAppointmentQDLLink( QAppointment& appointment )
{
    if ( appointment == QAppointment() )
        return;

    // Release any client QDLLinks
    QString links = appointment.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the Appointment is a QDLLink source, if so break it
    QHash<int, QString> keyhash;

    // Check if we need to create the QDLLink
    QString keyString = appointment.customField( QDL::SOURCE_DATA_KEY );

    // We actually store a date->keystring mapping, since we can
    // have multiple QDL links to the same appointment if it has
    // repetitions (but links to different dates)
    if (!keyString.isEmpty()) {
        QUniqueId u(keyString);
        if (!u.isNull()) {
            // Don't bother creating a new style of link, we don't know the day
            // and recreating the qdsdata is a pain.
            keyhash[appointment.start().date().toJulianDay()] = keyString;
        } else {
            QByteArray ba = QByteArray::fromBase64(keyString.toAscii());
            QDataStream ds(ba);
            ds >> keyhash;
        }
    }

    foreach(QString key, keyhash.values()) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();
    }

    if (!keyString.isEmpty()) {
        // Finally remove the stored key
        appointment.removeCustomField( QDL::SOURCE_DATA_KEY );
        model->updateAppointment( appointment );
    }
}

void DateBook::newAppointment(bool useCurrentCategory)
{
    newAppointment("", useCurrentCategory);
}

bool DateBook::newAppointment(const QString &description, bool useCurrentCategory)
{
    QDateTime current = QDateTime::currentDateTime();
    current.setDate(currentDate());
    QDateTime start = current;
    QDateTime end = current;

    int mod = QTime(0, 0, 0).secsTo(current.time()) % 900;
    if (mod != 0) {
        mod = 900 - mod;
        current = current.addSecs(mod);
    }

    start.setTime(current.time());
    start.setDate(current.date());
    end = current.addSecs(3600);

    return newAppointment(start, end, description, QString(), useCurrentCategory);
}

bool DateBook::newAppointment(const QDateTime& start, const QDateTime& end, bool useCurrentCategory)
{
    return newAppointment(start, end, QString(), QString(), useCurrentCategory);
}

void DateBook::editCurrentOccurrence()
{
    bool inViewMode = (appointmentDetails && viewStack->currentWidget() == appointmentDetails);

    if (inViewMode) {
        QOccurrence o = editOccurrence(appointmentDetails->occurrence());
        if (o.isValid())
            showAppointmentDetails(o);
    } else {
        editOccurrence(currentOccurrence());
    }
}

QOccurrence DateBook::editOccurrence(const QOccurrence &o)
{
    return editOccurrence( o, false );
}

QOccurrence DateBook::editOccurrence(const QOccurrence &o, bool )
{
    if (checkSyncing())
        return QOccurrence();
    if ( editorView != 0 )
        return QOccurrence();

    QAppointment mainApp = o.appointment();
    QAppointment editedApp = o.appointment();

    int exceptionType = ExceptionDialog::All;
    bool hasEarlier = o.start() != mainApp.start();
    bool hasLater = o.nextOccurrence().isValid();
    bool hasRepeat = mainApp.hasRepeat();

    if (hasRepeat && (hasEarlier || hasLater)) {
        exceptionType = askException(true);
        QAppointment a = editedApp;
        /* fix up exception types.
           for starting occurrence
                Earlier+Selected->Selected;
                Later->NotSelected;
                Later+Selected->All;
                Earlier->Cancel;
            for ending occrrence
                Later+Selected->Selected;
                Earlier->NotSelected;
                Earlier+Selected->All
                Later->Cancel;
        */
        if (!hasEarlier) {
            if (exceptionType & ExceptionDialog::Later) {
                exceptionType |= ExceptionDialog::Earlier;
            } else {
                exceptionType &= ExceptionDialog::NotEarlier;
            }
        } else if (!hasLater) {
            if (exceptionType & ExceptionDialog::Earlier) {
                exceptionType |= ExceptionDialog::Later;
            } else {
                exceptionType &= ExceptionDialog::NotLater;
            }
        }
        switch (exceptionType) {
            case ExceptionDialog::Selected:
            case ExceptionDialog::RetainSelected:
                // create exception.
                a.clearExceptions();
                a.setRepeatRule(QAppointment::NoRepeat);
                a.setStart(o.start());
                a.setUid(QUniqueId());
                if (exceptionType == ExceptionDialog::Selected)
                    editedApp = a;
                else
                    mainApp = a;
                break;
            case ExceptionDialog::Later:
            case ExceptionDialog::Earlier:
            case ExceptionDialog::NotLater:
            case ExceptionDialog::NotEarlier:
                // split appointment
                a.setUid(QUniqueId());
                a.clearExceptions();
                if (exceptionType == ExceptionDialog::NotEarlier || exceptionType == ExceptionDialog::Earlier)
                    a.setStart(o.start());
                else
                    a.setStart(o.nextOccurrence().start());
                if (exceptionType == ExceptionDialog::NotEarlier || exceptionType == ExceptionDialog::Later)
                    editedApp = a;
                else
                    mainApp = a;
                break;
            case ExceptionDialog::All:
                break;
            case ExceptionDialog::Cancel:
                return QOccurrence();
        }
    }

    int daysportion = (24 * 60) * (presetTime / (24 * 60));

    if (editedApp.alarm() == QAppointment::NoAlarm)
        editedApp.setAlarm( editedApp.isAllDay() ? (daysportion - (startTime * 60)) : presetTime, QAppointment::NoAlarm);

    editorView = new EntryDialog(onMonday, editedApp, QTime(startTime, 0), presetTime, this);
    editorView->setModal(true);
    editorView->setWindowTitle(tr("Edit Event", "window title" ));

    while (QtopiaApplication::execDialog(editorView)) {
        editedApp = editorView->appointment();

        QString error = validateAppointment(editedApp);
        if (!error.isNull()) {
            if ( QMessageBox::warning(
                    this->isVisible() ? this : 0,
                    "Error",
                    error,
                    QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Cancel )
            {
                delete editorView;
                editorView = 0;
                return QOccurrence();
            }
        } else {
            break;
        }
    }

    delete editorView;
    editorView = 0;

    QUniqueId newAppId;

    QOccurrence show;
    switch (exceptionType) {
        // exceptions
        case ExceptionDialog::RetainSelected:
            /* Exceptions are date based, so if the user changed the date of the appointment, we
               need to make sure we update the appointment, then add the exception... */
            newAppId = model->replaceOccurrence(editedApp, QOccurrence(o.start().date(), mainApp));
            model->updateAppointment(editedApp);
            /* Show the original occurrence (but need the new appointment) */
            show = model->appointment(newAppId).nextOccurrence(o.start().date());
            break;

        case ExceptionDialog::Selected:
            newAppId = model->replaceOccurrence(mainApp, editedApp.firstOccurrence(), o.start().date());
            /* Show the new occurrence of the new app (need to fetch again to setExceptionParent) */
            editedApp = model->appointment(newAppId); // exceptionParent is not set by replaceOccurrence
            show = editedApp.firstOccurrence();
            break;

        case ExceptionDialog::Earlier:
            model->updateAppointment(editedApp);
            newAppId = model->replaceRemaining(editedApp, mainApp, o.start().date());
            /* Show the original occurrence (which is in the new appointment) */
            mainApp = model->appointment(newAppId);
            show = mainApp.nextOccurrence(o.start().date());
            break;

        case ExceptionDialog::NotLater:
            model->updateAppointment(editedApp);
            model->replaceRemaining(editedApp, mainApp);
            /* Show the new edited occurrence, if we can, which will be the LAST occurrence of editedApp */
            // XXX this could be hideous for perf.
            editedApp = model->appointment(editedApp.uid());
            show = editedApp.firstOccurrence();
            while (show.nextOccurrence().isValid())
                show = show.nextOccurrence();
            break;

        case ExceptionDialog::Later:
            model->replaceRemaining(mainApp, editedApp, o.start().date().addDays(1));
            /* Show the original occurrence (but need the new appointment) */
            mainApp = model->appointment(mainApp.uid());
            show = mainApp.nextOccurrence(o.start().date());
            break;

        case ExceptionDialog::NotEarlier:
            newAppId = model->replaceRemaining(mainApp, editedApp, o.start().date());
            /* Show the first occurrence of the new app (need to fetch again to setExceptionParent) */
            editedApp = model->appointment(newAppId);
            show = editedApp.firstOccurrence();
            break;

        case ExceptionDialog::All:
            model->updateAppointment(editedApp);
            editedApp = model->appointment(editedApp.uid());
            /* If this was a repeating appointment, try to find the occurrence after the original date */
            if (hasRepeat) {
                show = editedApp.nextOccurrence(o.start().date());
                /* no occurrences after the original date, just find the last occurrence, then */
                if (!show.isValid()) {
                    show = editedApp.firstOccurrence();
                    while (show.nextOccurrence().isValid())
                        show = show.nextOccurrence();
                }
            } else {
                /* Not originally a repeating appointment, so show the first occurrence */
                show = editedApp.firstOccurrence();
            }
            break;
        default:
            return QOccurrence();
    }

    if (show.isValid())
        showAppointmentDetails(show);

    return show;
}

void DateBook::removeCurrentOccurrence()
{
    if (!occurrenceSelected())
        return;
    QOccurrence o = currentOccurrence();

    removeOccurrence(o);
}

void DateBook::removeOccurrence(const QOccurrence &o)
{
    if( checkSyncing() )
        return;

    QAppointment a = o.appointment();
    if (a.hasRepeat()) {
        //  Ask if just this one or the entire series
        int e = askException(false);
        switch (e) {
            case ExceptionDialog::Cancel:
                return;

            case ExceptionDialog::Selected:
                // Should check if the only one of its kind.. e.g. is
                // apointment next o, and o.n
                if (a.firstOccurrence() == o &&
                        !o.nextOccurrence().isValid()) {
                    removeAppointmentQDLLink(a);
                    model->removeAppointment(a);
                } else {
                    model->removeOccurrence(o);
                }
                if (viewStack->currentWidget() == appointmentDetails)
                    hideAppointmentDetails();
                break;

            case ExceptionDialog::All:
                removeAppointmentQDLLink(a);
                model->removeAppointment(a);
                if (viewStack->currentWidget() == appointmentDetails)
                    hideAppointmentDetails();
                break;

            case ExceptionDialog::RetainSelected:
                {
                    QAppointment a = o.appointment();
                    a.clearExceptions();
                    a.setStart(o.start());
                    a.setRepeatRule(QAppointment::NoRepeat);
                    model->updateAppointment(a);
                }
                // refresh the details
                if (viewStack->currentWidget() == appointmentDetails) {
                    a = model->appointment(a.uid());
                    showAppointmentDetails(a.firstOccurrence());
                }
                break;

            case ExceptionDialog::Earlier:
            case ExceptionDialog::NotLater:
                // update start date
                {
                    QDateTime start = e == ExceptionDialog::Earlier ? o.start() : o.nextOccurrence().start();
                    if (start.isNull()) {
                        removeAppointmentQDLLink(a);
                        model->removeAppointment(a);
                        if (viewStack->currentWidget() == appointmentDetails)
                            hideAppointmentDetails();
                    } else {
                        a.setStart(start);
                        if (!a.nextOccurrence(a.start().date().addDays(1)).isValid())
                            a.setRepeatRule(QAppointment::NoRepeat);
                        model->updateAppointment(a);
                        if (viewStack->currentWidget() == appointmentDetails) {
                            if (e == ExceptionDialog::NotLater)
                                hideAppointmentDetails();
                            else {
                                a = model->appointment(a.uid());
                                showAppointmentDetails(a.nextOccurrence(o.start().date()));
                            }
                        }
                    }
                }
                break;

            case ExceptionDialog::Later:
            case ExceptionDialog::NotEarlier:
                // update repeat till
                {
                    QDateTime start = e == ExceptionDialog::NotEarlier ? o.start() : o.nextOccurrence().start();
                    if (start.date() == a.start().date()) {
                        removeAppointmentQDLLink(a);
                        model->removeAppointment(a);
                        if (viewStack->currentWidget() == appointmentDetails)
                            hideAppointmentDetails();
                    } else {
                        a.setRepeatUntil(start.date().addDays(-1));
                        if (!a.nextOccurrence(a.start().date().addDays(1)).isValid())
                            a.setRepeatRule(QAppointment::NoRepeat);
                        model->updateAppointment(a);
                        if (viewStack->currentWidget() == appointmentDetails) {
                            if (e == ExceptionDialog::NotEarlier)
                                hideAppointmentDetails();
                            else {
                                a = model->appointment(a.uid());
                                showAppointmentDetails(a.nextOccurrence(o.start().date()));
                            }
                        }
                    }
                    break;
                }
        }
    }
    else
    {
        if (!Qtopia::confirmDelete(this->isVisible() ? this : 0, tr("Calendar"), Qt::escape(a.description())))
            return;
        if (viewStack->currentWidget() == appointmentDetails)
            hideAppointmentDetails();
        removeAppointmentQDLLink(a);
        model->removeAppointment(a);
    }

    updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );
}

void DateBook::removeOccurrencesBefore(const QDate &date)
{
    if (date.isNull())
        return;

    if (checkSyncing())
        return;

    for (int i = 0; i < model->rowCount(); i++) {
        QAppointment appointment = model->appointment(i);

        if (appointment.endInCurrentTZ() < QDateTime(date)) {
            removeAppointmentQDLLink(appointment);
            model->removeAppointment(appointment);
        }
    }
}

void DateBook::beamCurrentAppointment()
{
    if (occurrenceSelected())
        beamAppointment(currentAppointment());
}

void DateBook::beamAppointment(const QAppointment &e)
{
    ::unlink(beamfile.toLocal8Bit()); // delete if exists

    QAppointment::writeVCalendar(beamfile, e);
    QFile file(beamfile);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QtopiaSendVia::sendData(this, data, "text/x-vcalendar");
    }
}

void DateBook::setDocument(const QString &filename)
{
    QContent doc(filename);
    if (doc.isValid())
        receiveFile(doc.fileName());
    else
        receiveFile(filename);
}

void DateBook::showAppointmentDetails()
{
    if (occurrenceSelected())
        showAppointmentDetails(currentOccurrence());
}

void DateBook::showAppointmentDetails(const QOccurrence &o)
{
    bool showBuiltinDetails = true;

    QUniqueId parentId = o.appointment().parentDependency();

    if ( !parentId.isNull()) {
        QString depType = o.appointment().parentDependencyType();

        if (depType == "birthday" || depType == "anniversary") {
            // Contact birthday or anniversary
            showBuiltinDetails = false;

            QtopiaServiceRequest e("Contacts", "showContact(QUniqueId)");
            e << parentId;
            e.send();
        } else if (depType == "duedate") {
            // We have a task
            showBuiltinDetails = false;

            QtopiaServiceRequest e ("Tasks", "showTask(QUniqueId)");
            e << parentId;
            e.send();
        }
    }

    if (showBuiltinDetails) {
        initAppointmentDetails();
        if( isHidden() ) // only close after view if hidden on first activation
            closeAfterView = appointmentDetails;
        appointmentDetails->init(o);

        if (viewStack->currentWidget() != appointmentDetails) {
            appointmentDetails->previousDetails = viewStack->currentWidget();
            raiseView(appointmentDetails);
            appointmentDetails->setEditFocus(true);
            setWindowTitle( tr("Event Details") );
            updateIcons();
        }
    }

    if ( dayView) {
        dayView->setCurrentOccurrence(o);
    }
}

void DateBook::hideAppointmentDetails()
{
    if ( prevOccurrences.count() > 0 ) {
        showAppointmentDetails( prevOccurrences.top() );
        prevOccurrences.pop();
    } else {
        raiseView(appointmentDetails->previousDetails);
        appointmentDetails->previousDetails->setFocus();
        setWindowTitle( tr("Calendar") );
    }
    updateIcons();
}

void DateBook::updateIcons()
{
    bool showingDetails = (appointmentDetails && viewStack->currentWidget() == appointmentDetails);

    bool itemSelected = showingDetails ? occurrenceSelected() : false;
    bool itemEditable = itemSelected ? model->editable(currentOccurrence().uid()) : false;

    actionEdit->setVisible(itemEditable);
    actionDelete->setVisible(itemEditable);

    actionNew->setVisible(!showingDetails);
    actionToday->setVisible(!showingDetails);
    actionMonth->setVisible(!showingDetails && viewStack->currentWidget() != monthView);
    actionSettings->setVisible(!showingDetails);
#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
    actionAccounts->setVisible(!showingDetails && AccountEditor::editableAccounts(model));
#else
    actionAccounts->setVisible(false);
#endif
    actionCategory->setVisible(!showingDetails);
    actionShowSources->setVisible(!showingDetails);

    if ( showingDetails )
        categoryLbl->hide();
    else if ( ! model->categoryFilter().acceptAll() )
        categoryLbl->show();

    if(actionBeam)
        actionBeam->setVisible(itemSelected);

    if (occurrenceSelected())
        QSoftMenuBar::setLabel(dayView, Qt::Key_Select, QSoftMenuBar::View);
    else if (dayView)
        QSoftMenuBar::setLabel(dayView, Qt::Key_Select, "new", tr("New"));

    if (dayView &&
        viewStack->currentWidget() == dayView &&
        dayView->allDayFoldingAvailable()) {
        actionShowAll->setVisible(dayView->allDayFolded());
        actionHideSome->setVisible(!dayView->allDayFolded());
    } else {
        actionShowAll->setVisible(false);
        actionHideSome->setVisible(false);
    }
}

void DateBook::closeView()
{
    if (closeAfterView && viewStack->currentWidget()==closeAfterView) {
        closeAfterView = 0;
        close();
    } else if (viewStack->currentWidget() == appointmentDetails) {
        hideAppointmentDetails();
    } else if (defaultView == DateBookSettings::DayView &&
                (viewStack->currentWidget() == monthView || viewStack->currentWidget() == alarmView)) {
        viewDay();
    } else if (defaultView == DateBookSettings::MonthView &&
                (viewStack->currentWidget() == dayView || viewStack->currentWidget() == alarmView)) {
        viewMonth();
    } else {
        close();
    }
}

void DateBook::closeEvent(QCloseEvent *e)
{
    selectToday();
    if(syncing) {
        e->accept();
        return;
    }
    saveSettings();
}

bool DateBook::eventFilter( QObject *o, QEvent *e)
{
    if (o && o == dayView && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Select) {
            if (occurrenceSelected())
                return false;
            else {
                newAppointment(true);
                return true;
            }
        }
    }
    return false;
}

void DateBook::showEvent(QShowEvent *e)
{
    /* Now that we're here, see if we need to create some more UI (service requests can create UI before this) */
    if (viewStack->currentWidget() == 0) {
        switch (defaultView)
        {
            case DateBookSettings::DayView:
            default:
                initDayView();
                break;
            case DateBookSettings::MonthView:
                initMonthView();
                break;
        }
    }
    QMainWindow::showEvent(e);
}

void DateBook::init()
{
    alarmView = 0;
    dayView = 0;
    monthView = 0;
    appointmentDetails = 0;

    beamfile = Qtopia::tempDir() + "obex";

    QDir d;
    d.mkdir(beamfile);
    beamfile += "/appointment.vcs";

    model = new QAppointmentModel(this);

    loadSettings();

    setWindowTitle(tr("Calendar"));
    setWindowIcon(QPixmap(":image/DateBook"));

    //setToolBarsMovable( false );

    // Create the actions

    actionNew = new QAction(QIcon(":icon/new"), tr("New event"), this);
    actionNew->setWhatsThis(tr("Create a new event"));
    connect(actionNew, SIGNAL(triggered()), this, SLOT(newAppointment()));

    actionEdit = new QAction(QIcon(":icon/edit"), tr("Edit event"), this);
    actionEdit->setWhatsThis(tr("Edit the selected event"));
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(editCurrentOccurrence()));

    actionDelete = new QAction(QIcon(":icon/trash"), tr("Delete event"), this);
    actionDelete->setWhatsThis(tr("Delete the selected event"));
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(removeCurrentOccurrence()));

    if (QtopiaSendVia::isDataSupported("text/x-vcalendar")) {
        actionBeam = new QAction(QIcon(":icon/beam"), tr("Send"), this);
        actionBeam->setWhatsThis(tr("Beam the selected event"));
        connect(actionBeam, SIGNAL(triggered()), this, SLOT(beamCurrentAppointment()));
    }

    actionShowAll = new QAction(tr("Show All Events"), this);
    actionShowAll->setWhatsThis(tr("Show the all day events that are currently hidden"));
    connect(actionShowAll, SIGNAL(triggered()), this, SLOT(unfoldAllDay()));

    actionHideSome = new QAction(tr("Compress Events"), this);
    actionHideSome->setWhatsThis(tr("Show only a limited number of all day events"));
    connect(actionHideSome, SIGNAL(triggered()), this, SLOT(foldAllDay()));

    QActionGroup *g = new QActionGroup(this);
    g->setExclusive(true);

    actionToday = new QAction(QIcon(":icon/today"), tr("Today"), g);
    actionToday->setWhatsThis(tr("Show today's events"));
    connect(actionToday, SIGNAL(triggered()), this, SLOT(selectToday()));

    actionMonth = new QAction(QIcon(":icon/month"), tr("Month"), g);
    actionMonth->setWhatsThis(tr("Show selected month's events"));
    connect(actionMonth, SIGNAL(triggered()), this, SLOT(viewMonth()));

    actionAccounts = new QAction(QIcon(":icon/settings"), tr("Accounts"), this);
    connect(actionAccounts, SIGNAL(triggered()), this, SLOT(showAccountSettings()));
    // be default, dont' show this.  dependent on features of model loaded.
    actionAccounts->setVisible(false);

    actionCategory = new QAction(QIcon(":icon/viewcategory"), tr("View Category..."), this );
    connect( actionCategory, SIGNAL(triggered()), this, SLOT(selectCategory()));

    actionSettings = new QAction(QIcon(":icon/settings"), tr("Settings..."), g);
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));

    actionShowSources = new QAction(QIcon(":icon/viewcategory"),
        tr("Show Events From...", "e.g. Show Contacts From Phone/Contacts/Tasks"), this);
    connect(actionShowSources, SIGNAL(triggered()), this, SLOT(selectSources()));

    // Setup Menus
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction( actionNew );
    contextMenu->addAction( actionEdit );
    contextMenu->addAction( actionDelete );
    contextMenu->addAction( actionAccounts );
    contextMenu->addAction( actionShowAll );
    contextMenu->addAction( actionHideSome );

    if (actionBeam)
        contextMenu->addAction( actionBeam );

    contextMenu->addAction( actionToday );
    contextMenu->addAction( actionMonth );
    contextMenu->addAction( actionCategory);
    contextMenu->addAction( actionAccounts );
    contextMenu->addAction( actionSettings );
    contextMenu->addAction( actionShowSources );
#if defined(GOOGLE_CALENDAR_CONTEXT) && !defined(QT_NO_OPENSSL)
    actionAccounts->setVisible(AccountEditor::editableAccounts(model));
#else
    actionAccounts->setVisible(false);
#endif

    QWidget *widg = new QWidget();
    QVBoxLayout *vbl = new QVBoxLayout();
    vbl->setMargin(0);
    vbl->setSpacing(0);

    viewStack = new QStackedWidget(this);
    categoryLbl = new QLabel();
    categoryLbl->hide();

    vbl->addWidget(viewStack);
    vbl->addWidget(categoryLbl);
    widg->setLayout(vbl);

    setCentralWidget(widg);

    lastToday = QDate::currentDate();

    connect(qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock()));
    connect(qApp, SIGNAL(dateFormatChanged()), this, SLOT(changeClock()));
    connect(qApp, SIGNAL(weekChanged(bool)), this, SLOT(changeWeek(bool)));
    connect(qApp, SIGNAL(timeChanged()), this, SLOT(checkToday()));

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );

    connect(this, SIGNAL(categoryChanged(QCategoryFilter)),
            this, SLOT(categorySelected(QCategoryFilter)));

    categorySelected(model->categoryFilter());

    new CalendarService( this );

    // Set timer to go off 2 sections to midnight
    midnightTimer = new QTimer(this);
    connect(midnightTimer, SIGNAL(timeout()), this, SLOT(checkToday()));
    midnightTimer->start((QTime::currentTime().secsTo(QTime(23, 59, 59)) + 3) * 1000);

    // Update icons timer, used to allow datebase to refresh before applying
    // the update
    updateIconsTimer = new QTimer( this );
    updateIconsTimer->setSingleShot( true );
    connect( updateIconsTimer,
             SIGNAL(timeout()),
             this,
             SLOT(updateIcons()) );
}

void DateBook::initAlarmView()
{
    if (!alarmView) {
        alarmView = new AlarmView(0);
        viewStack->addWidget(alarmView);
        connect(alarmView, SIGNAL(showAlarmDetails(QOccurrence)), this, SLOT(showAppointmentDetails(QOccurrence)));
        connect(alarmView, SIGNAL(closeView()), this, SLOT(closeView()));
    }
}
void DateBook::initDayView()
{
    if (!dayView) {
        dayView = new DayView(0, model->categoryFilter(), model->visibleSources());
        viewStack->addWidget(dayView);
        int endTime = qMin(qMax(startTime + 8, 17), 24);
        dayView->setDaySpan(startTime, endTime);
        connect(dayView, SIGNAL(newAppointment()), this, SLOT(newAppointment()));
        connect(dayView, SIGNAL(removeOccurrence(QOccurrence)),
                this, SLOT(removeOccurrence(QOccurrence)));
        connect(dayView, SIGNAL(editOccurrence(QOccurrence)),
                this, SLOT(editOccurrence(QOccurrence)));
        connect(dayView, SIGNAL(beamAppointment(QAppointment)),
                this, SLOT(beamAppointment(QAppointment)));
        connect(dayView, SIGNAL(newAppointment(QString)),
                this, SLOT(newAppointment(QString)));
        connect(dayView, SIGNAL(newAppointment(QDateTime,QDateTime)),
                this, SLOT(newAppointment(QDateTime,QDateTime)));
        connect(dayView, SIGNAL(dateChanged()), this, SLOT(updateIcons()));
        connect(dayView, SIGNAL(showDetails()), this, SLOT(showAppointmentDetails()));
        connect(dayView, SIGNAL(selectionChanged()), this, SLOT(updateIcons()));
        connect(this, SIGNAL(categoryChanged(QCategoryFilter)),
                dayView, SLOT(categorySelected(QCategoryFilter)));
        connect(dayView, SIGNAL(closeView()), this, SLOT(closeView()));

        dayView->installEventFilter(this);
    }
}

void DateBook::initMonthView()
{
    if (!monthView) {
        monthView = new MonthView(0, model->categoryFilter(), model->visibleSources());
        monthView->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
        viewStack->addWidget(monthView);
        connect(monthView, SIGNAL(closeView()),
                this, SLOT(closeView()));
        connect(monthView, SIGNAL(activated(QDate)),
                this, SLOT(viewDay(QDate)));
        connect(this, SIGNAL(categoryChanged(QCategoryFilter)),
                monthView, SLOT(categorySelected(QCategoryFilter)));
    }
}

void DateBook::initAppointmentDetails()
{
    if (!appointmentDetails) {
        appointmentDetails = new AppointmentDetails(viewStack);
        appointmentDetails->setObjectName("eventview");
        viewStack->addWidget(appointmentDetails);

        connect(appointmentDetails, SIGNAL(done()), this, SLOT(closeView()));
    }

}

void DateBook::initExceptionDialog()
{
    if (!exceptionDialog) {
        exceptionDialog = new ExceptionDialog(this);
        exceptionDialog->setModal(true);
    }
}

int DateBook::askException(bool editMode)
{
    initExceptionDialog();
    if (editMode)
        exceptionDialog->setWindowTitle(tr("Edit Event"));
    else
        exceptionDialog->setWindowTitle(tr("Delete Event"));
    return exceptionDialog->exec(editMode);
}

void DateBook::loadSettings()
{
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup("Time");
        onMonday = config.value( "MONDAY" ).toBool();
    }

    {
        QSettings config("Trolltech","DateBook");
        config.beginGroup("Main");
        startTime = config.value("startviewtime", 8).toInt();
        aPreset = (QAppointment::AlarmFlags) config.value("alarmpreset").toInt();
        presetTime = config.value("presettime").toInt();
        defaultView = (DateBookSettings::ViewType) config.value("defaultview", DateBookSettings::DayView).toInt();
        compressDay = true;
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        if (count > 0) {
            model->setVisibleSources(set);
            if (dayView)
                dayView->setVisibleSources(set);
            if (monthView)
                monthView->setVisibleSources(set);
        }
        QCategoryFilter f;
        f.readConfig(config, "Category");
        model->setCategoryFilter( f );
        // Don't emit the changed signal here, we are called
        // before we set up most of the gui
    }
}

void DateBook::saveSettings()
{
    QSettings config("Trolltech","qpe");
    QSettings configDB("Trolltech","DateBook");
    configDB.beginGroup("Main");
    configDB.setValue("startviewtime",startTime);
    configDB.setValue("alarmpreset", (int) aPreset);
    configDB.setValue("presettime",presetTime);
    configDB.setValue("compressday", compressDay);
    configDB.setValue("defaultview", defaultView);
    model->categoryFilter().writeConfig(configDB, "Category");
    QSet<QPimSource> set = model->visibleSources();
    configDB.beginWriteArray("SelectedSources", set.count());
    int i = 0;
    foreach(QPimSource s, set) {
        configDB.setArrayIndex(i++);
        configDB.setValue("context", s.context.toString());
        configDB.setValue("identity", s.identity);
    }
    configDB.endArray();
}

void DateBook::appMessage(const QString&msg, const QByteArray &data)
{
    if ( msg == "receiveData(QString,QString)" )
    {
        QDataStream stream(data);
        QString f,t;
        stream >> f >> t;
        if ( t.toLower() == "text/x-vcalendar" )
            receiveFile(f);
        QFile::remove(f);
    }
}

void DateBook::selectSources()
{
    QPimSourceDialog diag(this);
    diag.setWindowTitle(tr("Show Events from"));
    diag.setPimModel(model);
    diag.setObjectName("select-sources");
    diag.showMaximized();

    if (QtopiaApplication::execDialog(&diag)) {
        QSet<QPimSource> set = model->visibleSources();
        if (dayView)
            dayView->setVisibleSources(set);
        if (monthView)
            monthView->setVisibleSources(set);
        saveSettings();
    }
}

bool DateBook::receiveFile(const QString &filename)
{
    QList<QAppointment> tl = QAppointment::readVCalendar(filename);

    QString msg = tr("<p>%1 new events.<p>Do you want to add them to your Calendar?",
                     "%1 number").
            arg(tl.count());

    if ( QMessageBox::information(this->isVisible() ? this : 0, tr("New Events"),
         msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
             QDateTime from,to;
             for( QList<QAppointment>::const_iterator it = tl.begin(); it != tl.end(); ++it ) {
                 if ( from.isNull() || (*it).startInCurrentTZ() < from )
                     from = (*it).startInCurrentTZ();
                 if ( to.isNull() || (*it).endInCurrentTZ() < to )
                     to = (*it).endInCurrentTZ();
                 model->addAppointment( *it );
             }

        // Change view to a sensible one...
        if ( from.date() == to.date() )
            viewDay( from.date() );
        else
            viewMonth( from.date() );

        return true;
    }
    return false;
}

bool DateBook::newAppointment(const QDateTime& dstart, const QDateTime& dend, const QString& description, const QString& notes, bool useCurrentCategory)
{
    if (checkSyncing())
        return false;

    if ( editorView != 0 )
        return false;

    //
    //  Figure out a start and end times for the new appointment, if none given
    //

    QDateTime start = dstart;
    QDateTime end = dend;
    bool startNull = start.isNull();
    bool endNull = end.isNull();

    QDateTime current = QDateTime::currentDateTime();
    current.setDate(currentDate());

    if (viewStack->currentWidget()) {
        if(viewStack->currentWidget() == monthView)
            start.setDate(monthView->selectedDate());
    }

    if (start.date().isNull())
        start.setDate(current.date());

    if (end.date().isNull())
        end.setDate(current.date());

    if ((startNull && end.time().isNull()) || !start.time().isValid()) {
        //  If no time is given, use the current time rounded up to the nearest 15 minutes
        int mod = QTime(0, 0, 0).secsTo(current.time()) % 900;
        if (mod != 0)
        {
            mod = 900 - mod;
            current = current.addSecs(mod);
        }
    }

    //  If no end time is given, default to one hour after start time
    if ((endNull && end.time().isNull()) || !end.time().isValid())
        end = start.addSecs(3600);

    //
    //  Prepare the appointment used by the dialog
    //

    QAppointment a;
    a.setDescription(description);
    a.setLocation("");
    a.setStart(start);
    a.setEnd(end);
    a.setNotes(notes);
    a.setAlarm(presetTime, aPreset);

    if (useCurrentCategory)
        a.setCategories(model->categoryFilter().requiredCategories());

    editorView = new EntryDialog(onMonday, a, QTime(startTime, 0), presetTime, this);
    editorView->setObjectName("edit-event");
    editorView->setModal(true);
    editorView->setWindowTitle(tr("New Event"));

    while ( QtopiaApplication::execDialog( editorView ) ) {
        a = editorView->appointment();

        QString error = validateAppointment(a);
        if(!error.isNull()) {
            if (QMessageBox::warning(this->isVisible() ? this : 0, tr("Error!"),
                error, tr("Fix it"), tr("Continue"), 0, 0, 1 ) == 0)
                continue;
        }

        QUniqueId id = model->addAppointment(a);
        a.setUid(id);

        if (dayView) {
            dayView->setCurrentAppointment(a);
        }

        updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );

        delete editorView;
        editorView = 0;

        return true;
    }

    delete editorView;
    editorView = 0;

    return false;
}

void DateBook::addAppointment(const QAppointment &e)
{
    QDate d = e.start().date();
    initDayView();
    dayView->selectDate(d);
}

bool DateBook::occurrenceSelected() const
{
    if (viewStack->currentWidget() && viewStack->currentWidget() == dayView)
        return dayView->currentIndex().isValid();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence().isValid();
    return false;
}

QAppointment DateBook::currentAppointment() const
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentAppointment();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence().appointment();

    return QAppointment();
}

QOccurrence DateBook::currentOccurrence() const
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentOccurrence();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence();

    return QOccurrence();
}

void DateBook::raiseView(QWidget *widget)
{
    if (!widget)
        return;

    if (this)
        this->setObjectName(widget->objectName());

    viewStack->setCurrentIndex(viewStack->indexOf(widget));

    showMaximized();
    activateWindow();
    raise();
}

QDate DateBook::currentDate()
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentDate();
    else if (monthView && viewStack->currentWidget() == monthView)
        return monthView->selectedDate();
    else
        return QDate(); // invalid;
}

bool DateBook::checkSyncing()
{
    if (syncing) {
        if (QMessageBox::warning(this->isVisible() ? this : 0, tr("Calendar"),
            tr("<qt>Can not edit data, currently syncing</qt>"),
            QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort)
        {
            // Okay, if you say so (eg. Qtopia Sync Agent may have crashed)....
            syncing = false;
        } else
            return true;
    }
    return false;
}

QString DateBook::validateAppointment(const QAppointment &e)
{
    // Check if overlaps with itself
    bool checkFailed = false;

    // check the next 12 repeats. should catch most problems
    QDate current_date = e.end().date();
    QOccurrence previous = e.nextOccurrence(current_date.addDays(1));
    int count = 12;
    while(count-- && previous.isValid()) {
        QOccurrence next = previous.nextOccurrence();
        if (!next.isValid())
            break;

        if(next.start() < previous.end()) {
            checkFailed = true;
            break;
        }

        previous = next;
    }

    if (checkFailed)
        return tr("<qt>Event duration is potentially longer "
                "than interval between repeats.</qt>");

    return QString();
}

void DateBook::categorySelected( const QCategoryFilter &c )
{
    model->setCategoryFilter( c );
    if (c.acceptAll()) {
        categoryLbl->hide();
    } else {
        categoryLbl->setText(tr("Category: %1").arg(c.label(DateBookCategoryScope)));
        categoryLbl->show();
    }
}

void DateBook::selectCategory()
{
    if (!categoryDialog) {
        categoryDialog = new QCategoryDialog(DateBookCategoryScope, QCategoryDialog::Filter | QCategoryDialog::SingleSelection, this);
        categoryDialog->setObjectName("Calendar");
    }
    categoryDialog->selectFilter(model->categoryFilter());
    categoryDialog->showMaximized();
    if (QtopiaApplication::execDialog(categoryDialog) == QDialog::Accepted)
        emit categoryChanged(categoryDialog->selectedFilter());
}

void DateBook::showAlarms(const QDateTime &when, int warn)
{
    //  May be more than one item.
    QDateTime alarmTime = when.addSecs(60 * warn);
    QOccurrenceModel *om = new QOccurrenceModel(alarmTime, alarmTime.addSecs(1), this);

    if (om->rowCount() > 0) {
        initAlarmView();
        if (alarmView->showAlarms(om, alarmTime, warn)) {
            if( isHidden() ) // only close after view if hidden on first activation
                closeAfterView = alarmView;
            raiseView(alarmView);
            showMaximized();
            // Make sure the screen comes on
            QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableLightOff);
            // but goes off again in the right number of seconds
            QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
        }

        // repeating events will have to have their alarms reset.
        QDateTime now = QDateTime::currentDateTime();
        for(int i = 0; i < om->count(); ++i) {
            QOccurrence o = om->occurrence(i);
            if (o.appointment().hasRepeat()) {
                o = o.nextOccurrence();
                while(o.isValid()) {
                    if (now <= o.alarmInCurrentTZ()) {
                        Qtopia::addAlarm(o.alarmInCurrentTZ(), "Calendar", "alarm(QDateTime,int)", o.alarmDelay());
                        break;
                    }
                }
            }
        }
    }
}

void DateBook::showSavedAlarms(int index)
{
    /* Grab the original alarm time etc */
    QSettings config("Trolltech","DateBook");
    config.beginGroup("ActiveAlarms");
    /* Use groups instead of arrays, because it will be sparse */
    config.beginGroup(QString("AlarmID-%1").arg(index));

    QDateTime when = config.value("EventTime").toDateTime();
    int delay = config.value("AlarmDelta").toInt();

    /* Remove this alarm */
    config.remove("");
    config.sync();

    /* And show things using the normal path */
    showAlarms(when.addSecs(-60 * delay), delay);
}

/*!
    \service CalendarService Calendar
    \inpublicgroup QtPimModule
    \brief The CalendarService class provides the Calendar service.

    The \i Calendar service enables applications to access features of
    the Calendar application.
*/

/*!
    \internal
*/
CalendarService::~CalendarService()
{
}

/*!
    Open a dialog so the user can create a new appointment. Default
    values are used.

    This slot corresponds to the QCop service message
    \c{Calendar::newAppointment()}.
*/
void CalendarService::newAppointment()
{
    datebook->newAppointment("", false);
}

/*!
    Open a dialog so the user can create a new appointment.  The new
    appointment will use the specified \a start and \a end times,
    \a description and \a notes.

    This slot corresponds to the QCop service message
    \c{Calendar::newAppointment(QDateTime,QDateTime,QString,QString)}.
*/
void CalendarService::newAppointment
            ( const QDateTime& start, const QDateTime& end,
              const QString& description, const QString& notes )
{
    datebook->newAppointment( start, end, description, notes, false);
}

/*!
    Add the specified \a appointment to the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::addAppointment(QAppointment)}.
*/
void CalendarService::addAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        datebook->model->addAppointment( appointment );
    }
}

/*!
    Update the specified \a appointment in the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::updateAppointment(QAppointment)}.
*/
void CalendarService::updateAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        datebook->model->updateAppointment( appointment );
    }
}

/*!
    Remove the specified \a appointment from the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::removeAppointment(QAppointment)}.
*/
void CalendarService::removeAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        QAppointment a = appointment;
        datebook->removeAppointmentQDLLink( a );
        datebook->model->removeAppointment( appointment );
    }
}

/*!
    Open the calendar user interface and show the appointments for today.

    This slot corresponds to the QCop service message
    \c{Calendar::raiseToday()}.
*/
void CalendarService::raiseToday()
{
    datebook->viewToday();
    QtopiaApplication::instance()->showMainWidget();
}

/*!
  Activates the alarm dialog for appointments with alarms occurring at the given \a time.
  The specified \a delay indicates how many minutes before the start of the appointment
  that the alarm has been activated.
*/
void CalendarService::alarm( const QDateTime &time, int delay )
{
    datebook->showAlarms(time, delay);
}

/*!
  \internal

  Reactivates the alarm dialog for appointments with alarms that have been snoozed,
  using the alarm information stored in our settings.  The \a time value is ignored,
  but the \a index is used to load the snoozed appointment settings.
*/
void CalendarService::snooze(const QDateTime &time, int index)
{
    Q_UNUSED(time);
    datebook->showSavedAlarms(index);
}

/*!
    Switch the calendar to the next view.

    This slot corresponds to the QCop service message
    \c{Calendar::nextView()}.
*/
void CalendarService::nextView()
{
    datebook->nextView();
}

/*!
    Show the appointment indicated by \a uid in the calendar application, wither
    the next upcoming occurrence from the current date, or the first occurrence
    if there is no upcoming occurrence.

    This slot corresponds to the QCop service message
    \c{Calendar::showAppointment(QUniqueId)}.
*/
void CalendarService::showAppointment( const QUniqueId& uid )
{
    QAppointment a = datebook->model->appointment(uid);
    QOccurrence o = a.nextOccurrence(QDate::currentDate());
    if (!o.isValid())
        o = a.firstOccurrence();

    if (o.isValid()) {
        datebook->showAppointmentDetails(o);
    }
}

/*!
    Show the occurrence of the appointment indicated by \a uid on \a date.

    This slot corresponds to the QCop service message
    \c{Calendar::showAppointment(QUniqueId,QDate)}.
*/
void CalendarService::showAppointment( const QUniqueId& uid, const QDate& date )
{
    QAppointment a = datebook->model->appointment(uid);
    QOccurrence o = a.nextOccurrence(date);

    if (o.isValid()) {
        datebook->showAppointmentDetails(o);
    }
}

/*!
    Allow the system cleanup wizard to recover some space.
    The \a date to clean from is a hint only. The calendar program
    should only remove appointments and occurrences where doing so
    will save space.

    This slot corresponds to the QCop service message
    \c{Calendar::cleanByDate(QDate)}.
*/
void CalendarService::cleanByDate( const QDate& date )
{
    datebook->removeOccurrencesBefore( date );
}

/*!
    Activate the QDL link contained within \a request.

    The slot corresponds to a QDS service with a request data type of
    QDLLink::mimeType() and no response data.

    The slot corresponds to the QCop service message
    \c{Calendar::activateLink(QDSActionRequest)}.
*/
void CalendarService::activateLink( const QDSActionRequest& request )
{
    datebook->qdlActivateLink( request );
}

/*!
    Request for one or more QDL links using the hint contained within
    \a request.

    The slot corresponds to a QDS service with a request data type of
    "text/x-qstring" and response data type of QDLLink::listMimeType().

    The slot corresponds to the QCop service message
    \c{Calendar::requestLinks(QDSActionRequest)}.

*/
void CalendarService::requestLinks( const QDSActionRequest& request )
{
    datebook->qdlRequestLinks( request );
}
