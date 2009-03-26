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
#ifndef DATEBOOK_H
#define DATEBOOK_H

#include <qappointmentmodel.h>
#include <qappointment.h>

#include "appointmentdetails.h"
#include "datebooksettings.h"

#include <QDateTime>
#include <QtopiaAbstractService>
#include <QDSData>
#include <QStack>
#include <QMainWindow>

class QStackedWidget;
class AlarmView;
class DayView;
class MonthView;
class QAppointment;
class ExceptionDialog;
class AppointmentDetails;
class QDSActionRequest;
class EntryDialog;
class QCategoryDialog;

class DateBook : public QMainWindow
{
    friend class AppointmentPicker;
    friend class CalendarService;
    Q_OBJECT

public:
    DateBook( QWidget *parent = 0, Qt::WFlags f = 0);
    ~DateBook();

public slots:
    void selectToday();     // Select today's date without changing view
    void viewToday();       // View today's date in day view
    void viewDay();         // View currently selected date in day view
    void viewDay(const QDate& d);
    void viewMonth();
    void viewMonth(const QDate& d);
    void nextView();
    void closeView();

    void unfoldAllDay();
    void foldAllDay();

    void checkToday();

    void showSettings();
    void selectSources();

    void changeClock();
    void changeWeek(bool newDay);

    void newAppointment(bool useCurrentCategory = true);
    bool newAppointment(const QString &str, bool useCurrentCategory = true);
    bool newAppointment(const QDateTime &dstart, const QDateTime &dend, bool useCurrentCategory = true);
    bool newAppointment(const QDateTime &dstart, const QDateTime &dend, const QString &description, const QString &notes, bool useCurrentCategory = true);

    void addAppointment(const QAppointment &e);

    void editCurrentOccurrence();
    QOccurrence editOccurrence(const QOccurrence &o);
    QOccurrence editOccurrence(const QOccurrence &o, bool preview);

    void removeCurrentOccurrence();
    void removeOccurrence(const QOccurrence &o);
    void removeOccurrencesBefore(const QDate &date);

    void beamCurrentAppointment();
    void beamAppointment(const QAppointment &a);
    void setDocument(const QString &filename);

    void showAppointmentDetails();
    void showAppointmentDetails(const QOccurrence &o);
    void hideAppointmentDetails();

    void showAlarms(const QDateTime &when, int warn);
    void showSavedAlarms(int index);

    void showAccountSettings();

    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );

    void categorySelected( const QCategoryFilter &c );
    void selectCategory();

    /*void find();
    void doFind(const QString &, const QDate &, Qt::CaseSensitivity, bool, const QCategoryFilter &);*/

    void updateIcons();

    void appMessage(const QString &, const QByteArray &);
signals:
    void searchNotFound();
    void searchWrapAround();
    void categoryChanged( const QCategoryFilter & c );

protected:
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

    void init();
    void initAlarmView();
    void initDayView();
    void initMonthView();
    void initAppointmentDetails();

    void initExceptionDialog();
    int askException(bool editMode);

    void loadSettings();
    void saveSettings();

    bool receiveFile(const QString &filename);

    bool occurrenceSelected() const;
    QAppointment currentAppointment() const;
    QOccurrence currentOccurrence() const;

    void raiseView(QWidget *widget);

    QSet<QPimSource> addressbookSources();

    QDate currentDate();
    bool checkSyncing();
    QString validateAppointment(const QAppointment &a);

private:
    QDSData occurrenceQDLLink( const QOccurrence& o);
    void removeAppointmentQDLLink( QAppointment& appointment );

    QAppointmentModel *model;

    AlarmView *alarmView;
    DayView *dayView;
    MonthView *monthView;
    AppointmentDetails *appointmentDetails;
    QStackedWidget *viewStack;
    QLabel *categoryLbl;

    EntryDialog* editorView;

    QCategoryDialog *categoryDialog;

    ExceptionDialog *exceptionDialog;

    // Configuration values
    QAppointment::AlarmFlags aPreset;    // have everything set to alarm?
    int presetTime;  // the standard time for the alarm
    int startTime;
    bool ampm;
    bool onMonday;
    bool compressDay;
    DateBookSettings::ViewType defaultView;

    bool syncing;
    QWidget *closeAfterView;

    QDate lastToday; // last date that was the selected as 'Today'
    QTimer *midnightTimer;
    QTimer *updateIconsTimer;

    QStack<QOccurrence> prevOccurrences;
    QString beamfile;

    QAction *actionMonth;
    QAction *actionNew;
    QAction *actionEdit;
    QAction *actionDelete;
    QAction *actionBeam;
    QAction *actionToday;
    QAction *actionSettings;
    QAction *actionPurge;
    QAction *actionShowAll;
    QAction *actionHideSome;
    QAction *actionCategory;
    QAction *actionAccounts;
    QAction *actionShowSources;
};

class CalendarService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class DateBook;
private:
    CalendarService( DateBook *parent )
        : QtopiaAbstractService( "Calendar", parent )
        { datebook = parent; publishAll(); }

public:
    ~CalendarService();

public slots:
    void newAppointment();
    void newAppointment( const QDateTime& start, const QDateTime& end,
                         const QString& description, const QString& notes );
    void addAppointment( const QAppointment& appointment );
    void updateAppointment( const QAppointment& appointment );
    void removeAppointment( const QAppointment& appointment );
    void raiseToday();
    void nextView();
    void showAppointment( const QUniqueId& uid );
    void showAppointment( const QUniqueId& uid, const QDate& date );
    void cleanByDate( const QDate& date );
    void activateLink( const QDSActionRequest& request );
    void requestLinks( const QDSActionRequest& request );

    void alarm(const QDateTime &, int);

    void snooze(const QDateTime &, int);

private:
    DateBook *datebook;
};

#endif
