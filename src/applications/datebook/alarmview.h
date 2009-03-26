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

#ifndef ALARMVIEW_H
#define ALARMVIEW_H

#include <qtopiapim/qappointment.h>
#include <QDialog>
#include <QBasicTimer>
#include <QPointer>

class QListView;
class QModelIndex;
class QOccurrenceModel;
class QStandardItemModel;
class QComboBox;
class QPushButton;
class QScrollArea;

class AlarmView : public QWidget
{
    Q_OBJECT
public:
    AlarmView( QWidget *parent, Qt::WFlags f = 0 );

    bool showAlarms(QOccurrenceModel *m, const QDateTime& startTime, int warnDelay);
    QOccurrence selectedOccurrence() const;

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent( QKeyEvent * ke);
    bool focusNextPrevChild(bool);

signals:
    void showAlarmDetails(const QOccurrence&);
    void closeView();

private slots:
    void currentAlarmChanged(const QModelIndex& idx);
    void alarmSelected(const QModelIndex &idx);
    void snoozeClicked();
    bool updateAlarms();

private:
    void init();
    void formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime);

    int mAlarmCount;
    QBasicTimer mAlarmTimer;

    QDateTime mStartTime;
    int mDelay;
    QPointer<QOccurrenceModel> mModel;
    QStandardItemModel *mStandardModel;
    /* Widgetty stuff */
    QComboBox *mSnoozeChoices;
    QPushButton *mSnoozeButton;
    QScrollArea *scrollArea;
    QListView *mAlarmList;
};


#endif
