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

#ifndef RECURRENCEDETAILS_H
#define RECURRENCEDETAILS_H

#include <qdialog.h>
#include <qdatetime.h>
#include "qtask.h"
#include "qappointment.h"

class QScrollArea;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QLabel;
class QFormLayout;

class RecurrenceDetails : public QObject
{
    Q_OBJECT

public:
    RecurrenceDetails( QAppointment & appointment, QObject *parent = 0 );
    ~RecurrenceDetails();

    void initGui(QFormLayout *fl);

    bool eventFilter(QObject*, QEvent*);

public slots:
    void updateAppointment();
    void updateUI();

private slots:
    void repeatTypeChanged();
    void updateRepeatUntil();

private:
    void refreshLabels();
    QString trSmallOrdinal(int n) const;

private:
    bool mGuiInited;

    QAppointment& mAppointment;
    bool startMonday;
    QList<QAbstractButton *> weekDayToggle;
    QScrollArea *scrollArea;
    QComboBox *repeatCB;
    QDateEdit *untilDate;
    QWidget *repeatFieldsW;
    QLabel *repEveryL;
    QSpinBox *repEverySB;
    QGroupBox *monthlyGB, *weeklyGB, *untilGB;
    QRadioButton *dayOfMonth, *weekDayOfMonth, *lastWeekDayOfMonth;
};

#endif
