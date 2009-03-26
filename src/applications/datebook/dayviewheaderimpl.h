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
#ifndef DATEBOOKDAYHEADER_H
#define DATEBOOKDAYHEADER_H
#include <qwidget.h>
#include <qdatetime.h>

class QToolButton;
class QDateEdit;
class QFrame;
class QButtonGroup;

class DayViewHeader : public QWidget
{
    Q_OBJECT

public:
    DayViewHeader( bool bUseMonday, QWidget* parent = 0 );
    ~DayViewHeader();
    void setStartOfWeek( bool onMonday );

public slots:
    void goBack();
    void goForward();
    void setDate( int, int, int );
    void setDay( int );

signals:
    void dateChanged( const QDate & );

private slots:
    void init();
    void setupNames();

private:
    QDate currDate;
    bool bUseMonday;

    QToolButton *back;
    QToolButton *forward;
    QFrame *fraDays;
    QButtonGroup *grpDays;
    QToolButton *cmdDay1, *cmdDay2, *cmdDay3, *cmdDay4, *cmdDay5, *cmdDay6, *cmdDay7;
    QDateEdit *dButton;
};

#endif
