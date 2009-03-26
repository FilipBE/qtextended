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

#ifndef E2_LAUNCHER_H
#define E2_LAUNCHER_H

#include "qabstractserverinterface.h"
#include <QList>
#include <QContent>
#include <QPixmap>
#include <QTimer>

class QValueSpaceItem;
class QAbstractBrowserScreen;
class QExportedBackground;
class E2TelephonyBar;
class QAppointmentModel;
class QOccurrenceModel;
class E2Dialer;
class E2ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E2ServerInterface(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual bool event(QEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void e2Received(const QString &, const QByteArray &);
    void applicationLaunched(const QString &);
    void operatorChanged();
    void updateAppointment();

private:
    void doAppointmentTimer(bool);
    QTimer appointmentTimer;

    E2TelephonyBar *m_bar;

    QList<QContent> m_lastUsedApps;
    QPixmap m_ringProf;
    QAbstractBrowserScreen *m_browser;
    QPixmap m_wallpaper;
    QString operatorName;
    QValueSpaceItem *operatorItem;
    QString occurrenceText;
    QPixmap datebook;

    QOccurrenceModel *m_model;
    QAppointmentModel *m_appointment;
};

#endif
