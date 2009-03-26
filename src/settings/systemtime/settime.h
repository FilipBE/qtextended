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

#ifndef SETTIME_H
#define SETTIME_H

#include <qdatetime.h>
#include <QDialog>
#include <QBasicTimer>
#include <qtopiaabstractservice.h>

class QToolButton;
class QLabel;
class QTimeZoneSelector;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QValueSpaceItem;

class SetDateTime : public QDialog
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, Qt::WFlags f=0 );

    void setTimezoneEditable(bool tze);
    void setExternalTimeSourceWarnings(bool warn);

protected slots:
    void tzChange( const QString &tz );
    void dateChange(const QDate &);
    void timeChange(const QTime &);
    void updateDateFormat();
    void updateTimeFormat(int);

public slots:
    void editTime();
    void editDate();
    void setAutomatic(int on);
    void sysTimeZoneChanged();

protected:
    virtual void timerEvent( QTimerEvent* );
    virtual void accept();
    virtual void reject();

private:
    QTimeEdit *time;
    QDateEdit *date;
    QComboBox *atz;
    QTimeZoneSelector *tz;
    QComboBox *weekStartCombo;
    QComboBox *ampmCombo;
    QComboBox *dateFormatCombo;
    QLabel *tz_label, *time_label, *date_label;

    QStringList date_formats;
    bool dateChanged;
    bool timeChanged;
    bool tzChanged;
    bool tzEditable;
    bool externalWarnings;
    QLabel *tzLabel;

    QBasicTimer clocktimer;

    QString selectedDateFormat() const;
    void storeSettings();
};

class TimeService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SetDateTime;
private:
    TimeService( SetDateTime *parent );

public:
    ~TimeService();

public slots:
    void editTime();

private:
    SetDateTime *parent;
};

class DateService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SetDateTime;
private:
    DateService( SetDateTime *parent );

public:
    ~DateService();

public slots:
    void editDate();

private:
    SetDateTime *parent;
};

#endif
