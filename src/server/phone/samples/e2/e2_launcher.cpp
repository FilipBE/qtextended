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

#include "e2_launcher.h"
#include "e2_telephonybar.h"
#include "qtopiaserverapplication.h"
#include <QDesktopWidget>
#include <QEvent>
#include <QFontMetrics>
#include "applicationlauncher.h"
#include "dialercontrol.h"
#include <QResizeEvent>
#include <QSize>
#include <QPainter>
#include <QMouseEvent>
#include "e2_header.h"
#include "qabstractbrowserscreen.h"
#include <QtopiaChannel>
#include <QContent>
#include <QContentSet>
#include <custom.h>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <QExportedBackground>
#endif
#include <QDesktopWidget>
#include "e2_bar.h"
#include "e2_frames.h"
#include <QAppointmentModel>
#include <QOccurrenceModel>
#include <QDateTime>
#include <QAppointment>
#include <QtopiaServiceRequest>
#include <QFontDatabase>
#include <QVBoxLayout>
#include "e2_telephony.h"
#include "e2_callscreen.h"
#include <QPoint>

/*
    Shows:
        telephony bar.
        Next meeting.
        Operator.
 */

static QWidget *e2_callhistory()
{
    E2FSCallHistory *history = new E2FSCallHistory(0);
    return history;
}

E2ServerInterface::E2ServerInterface(QWidget *parent, Qt::WFlags flags)
: QAbstractServerInterface(parent, flags),
  m_ringProf(":image/profileedit/Note"),
  m_browser(0), m_wallpaper(":image/themes/qtopia/ladybug.png"), 
  operatorItem(0), datebook(":image/datebook/DateBook_16"),
  m_model(0), m_appointment(0)
{
    // XXX - remove me
    DialerControl::instance();

    // Replace default call history
    BuiltinApplicationLauncher::install("callhistory", e2_callhistory);

    E2Header *header = new E2Header(0);
    header->show();

    setGeometry(qApp->desktop()->availableGeometry());

    m_bar = new E2TelephonyBar(this);

    E2Telephony *telephony = new E2Telephony(0);
    E2Incoming * incoming = new E2Incoming();
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(popupCallscreen()));
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(display()));

    (void)new E2NewMessage();

    QtopiaChannel *e2 = new QtopiaChannel("QPE/E2", this);
    QObject::connect(e2, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(e2Received(QString,QByteArray)));

    operatorItem = new QValueSpaceItem("/Telephony/Status/OperatorName",
                                       this);
    QObject::connect(operatorItem, SIGNAL(contentsChanged()),
                     this, SLOT(operatorChanged()));

    QObject::connect(&appointmentTimer, SIGNAL(timeout()),
                     this, SLOT(updateAppointment()));

}

void E2ServerInterface::updateAppointment()
{
    if(m_model->rowCount() == 0) {
        occurrenceText = "No meetings today";
    } else {
        QAppointment appointment;
        bool found = false;
        int secs = 0;
        for(int ii = 0; !found && ii < m_model->rowCount(); ++ii) {
            appointment = m_model->appointment(ii);
            occurrenceText = appointment.description();
            QOccurrence occurrence =
                appointment.nextOccurrence(QDate::currentDate());
            secs = QDateTime::currentDateTime().secsTo(occurrence.start());
            if(secs >= 0) {
                found = true;
            }
        }

        if(!found) {
            occurrenceText = "No meetings today";
        }
    }

    update();
}

void E2ServerInterface::operatorChanged()
{
    operatorName = operatorItem->value().toString();
    update();
}

void E2ServerInterface::resizeEvent(QResizeEvent *e)
{
    QSize s = e->size();

    m_bar->setGeometry(0, 0, s.width(), m_bar->height());
}

void E2ServerInterface::mousePressEvent(QMouseEvent *e)
{
    for(int ii = 0; ii < 5; ++ii) {
        int w = width() / 5;
        int x1 = ii * w;
        QRect r(x1, height() - 44, w, 44);
        if(r.contains(e->pos())) {
            // Found
            if(0 == ii) {
                Qtopia::execute("profileedit");
            } else if(m_lastUsedApps.count() > (ii - 1)) {
                m_lastUsedApps.at(ii - 1).execute();
            }

            QAbstractServerInterface::mousePressEvent(e);
            return;
        }
    }

    QRect calrect(0, (height() - datebook.height()) / 2,
                  width(), datebook.height());
    if(calrect.contains(e->pos())) {
        QtopiaServiceRequest sr("Calendar", "raiseToday()");
        sr.send();
        return;
    }

    QAbstractServerInterface::mousePressEvent(e);
}

bool E2ServerInterface::event(QEvent *e)
{
    if(e->type() == QEvent::WindowActivate) {
        doAppointmentTimer(true);
    } else if(e->type() == QEvent::WindowDeactivate) {
        doAppointmentTimer(false);
    }
    return QAbstractServerInterface::event(e);
}

void E2ServerInterface::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Back)
        e->accept();
    else
        QAbstractServerInterface::keyPressEvent(e);
}

void E2ServerInterface::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

void E2ServerInterface::doAppointmentTimer(bool on)
{
    if(on) {
        if(m_model) delete m_model;
        if(m_appointment) delete m_appointment;

        m_appointment = new QAppointmentModel(this);
        m_model =
            new QOccurrenceModel(QDateTime::currentDateTime(),
                                 QDateTime(QDate(2099,1,1), QTime(0,0,0)),
                                 this);
        m_model->setDurationType( QAppointmentModel::AnyDuration );
        QObject::connect(m_model, SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(updateAppointment()));
        QObject::connect(m_model, SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SLOT(updateAppointment()));
        QObject::connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)) , this, SLOT(updateAppointment()));
        QObject::connect(m_model, SIGNAL(modelReset()), this, SLOT(updateAppointment()));
        QObject::connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateAppointment()));
        QObject::connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateAppointment()));

        updateAppointment();

        appointmentTimer.start(60 * 1000);
    } else {
        appointmentTimer.stop();
    }

}

void E2ServerInterface::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QPoint point = mapFromGlobal(QPoint(0, 0));

    if(!m_wallpaper.isNull())
        p.drawPixmap(point, m_wallpaper);
    else
        p.fillRect(rect(), QColor(114, 146, 102));

    // Divide the bottom of the screen into 5 and paint the icon for each of
    // the last used applications, with profile edit always visible
    for(int ii = 0; ii < 5; ++ii) {
        QPixmap pix;
        if(0 == ii) {
            pix = m_ringProf;
        } else {
            if(m_lastUsedApps.count() > (ii - 1)) {
                pix = m_lastUsedApps.at(ii - 1).icon().pixmap(44, 44);
            }
        }

        if(!pix.isNull()) {
            int w = width() / 5;
            int x1 = ii * w;

            p.drawPixmap(x1 + (w - pix.width()) / 2, height() - 1 - pix.height() - 4, pix);
        }
    }

    // Draw the next appointment
    QFontMetrics metrics(QApplication::font());
    p.drawPixmap(4, (height() - datebook.height()) / 2, datebook);
    p.drawText(0, (height() - metrics.height()) / 2, width(),
               metrics.height(), Qt::AlignHCenter | Qt::AlignVCenter,
               occurrenceText);

    // Draw operator name (if any), 2/3rds the way down the screen
    QFontMetrics fm(font());
    p.drawText(0, (2 * (height() - fm.height())) / 3, width(), fm.height(),
               Qt::AlignHCenter | Qt::AlignVCenter, operatorName);
}

void E2ServerInterface::e2Received(const QString &name,
                                      const QByteArray &)
{
    if("showHome()" == name) {
        if(!m_browser) {
            m_browser = qtopiaWidget<QAbstractBrowserScreen>();
            QObject::connect(m_browser, SIGNAL(applicationLaunched(QString)),
                    this, SLOT(applicationLaunched(QString)));
        }
        m_browser->resetToView("Main");
        m_browser->showMaximized();
        m_browser->raise();
    }
}

void E2ServerInterface::applicationLaunched(const QString &name)
{
    if(name == "profileedit")
        return;

    QContent app(name,false);
    if(app.isNull()) return;

    for(int ii = 0; ii < m_lastUsedApps.count(); ++ii)
        if(m_lastUsedApps.at(ii) == app)
            return;

    if(m_lastUsedApps.count() == 4)
        m_lastUsedApps.removeLast();
    m_lastUsedApps.prepend(app);

    update();
}

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, E2ServerInterface);

