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

#ifndef HOMESCREENWIDGETS_H
#define HOMESCREENWIDGETS_H

// Qt includes
#include <QWidget>
#include <QMap>
#include <QString>
#include <QList>

// Qtopia includes
#include <QContent>
#include <QAppointmentModel>
#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <QWorldmap>
#include <QAnalogClock>

// ============================================================================
//
// LauncherIcon
//
// ============================================================================

class LauncherIcon : public QWidget
{
    Q_OBJECT

public:
    LauncherIcon( const QContent& app, QWidget* parent = 0 );

protected:
    void paintEvent( QPaintEvent* /*event*/ );
    void keyPressEvent( QKeyEvent* event );

public slots:
    void launch();

private:
    const QContent mApp;
};

// ============================================================================
//
// LauncherHSWidget
//
// ============================================================================

class LauncherHSWidget : public QWidget
{
    Q_OBJECT

public:
    LauncherHSWidget( QWidget* parent = 0, Qt::WFlags flags = 0 );

public slots:
    void launch();

private:
    QList<QContent> mApps;
    QList<LauncherIcon*> mIcons;
};

// ============================================================================
//
// AppointmentsHSWidget
//
// ============================================================================

class AppointmentsHSWidget : public QWidget
{
    Q_OBJECT

public:
    AppointmentsHSWidget( QWidget* parent = 0, Qt::WFlags flags = 0 );

public slots:
    void showNextAppointment();

private slots:
    void update();

private:
    bool updateModel();
    QString appProgress( const int minutesPast );
    QString appScheduled( const QOccurrence& occurence );

    QValueSpaceObject* mVsObject;
    QValueSpaceItem* mVsItem;
    QUniqueId mUid;
    QDate     mDate;
    QOccurrenceModel* mModel;
};

// ============================================================================
//
// WorldmapHSWidget
//
// ============================================================================

class WorldmapHSWidget : public QWorldmap
{
    Q_OBJECT

public:
    WorldmapHSWidget( QWidget* parent = 0, Qt::WFlags flags = 0 );

public slots:
    void showCity();

protected:
    void paintEvent( QPaintEvent *event );

private slots:
    void showTZ();

private:
    int mCheck;
};

// ============================================================================
//
// AnalogClockHSWidget
//
// ============================================================================

class AnalogClockHSWidget : public QAnalogClock
{
    Q_OBJECT

public:
    AnalogClockHSWidget( QWidget* parent = 0, Qt::WFlags flags = 0 );

private slots:
    void update();
};

#endif
