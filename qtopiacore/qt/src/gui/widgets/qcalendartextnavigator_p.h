/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QCALENDARTEXTNAVIGATOR_P_H
#define QCALENDARTEXTNAVIGATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qbasictimer.h>

#ifndef QT_NO_CALENDARWIDGET

QT_BEGIN_NAMESPACE

class QLabel;
class QCalendarDateValidator;
class QFrame;

class QCalendarTextNavigator: public QObject
{
    Q_OBJECT
public:
    QCalendarTextNavigator(QObject *parent = 0)
        : QObject(parent), m_dateText(0), m_dateFrame(0), m_dateValidator(0), m_widget(0), m_editDelay(1500), m_date(QDate::currentDate()) { }

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    int dateEditAcceptDelay() const;
    void setDateEditAcceptDelay(int delay);

    QDate date() const;
    void setDate(const QDate &date);

    bool eventFilter(QObject *o, QEvent *e);
    void timerEvent(QTimerEvent *e);

signals:
    void dateChanged(const QDate &date);
    void editingFinished();

private:
    void applyDate();
    void updateDateLabel();
    void createDateLabel();
    void removeDateLabel();

    QLabel *m_dateText;
    QFrame *m_dateFrame;
    QBasicTimer m_acceptTimer;
    QCalendarDateValidator *m_dateValidator;
    QWidget *m_widget;
    int m_editDelay;

    QDate m_date;
};

QT_END_NAMESPACE

#endif // QT_NO_CALENDARWIDGET

#endif

