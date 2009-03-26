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

#include <QApplication>
#include <QDesktopWidget>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <QCopChannel>
#endif
#include <QPainter>
#include <QKeyEvent>
#include <QShowEvent>
#include <QMouseEvent>
#include <QLabel>
#include "e1_launcher.h"
#include "e1_header.h"
#include "e1_phonebrowser.h"
#include "e1_dialer.h"
#include "e1_dialog.h"
//#include "phone/dialer.h"
#include <qtopiaipcenvelope.h>
#include "dialercontrol.h"
#include "e1_popup.h"
#include "e1_telephony.h"
#include <QListWidget>
#include <QDebug>
#include "e1_incoming.h"
#include <qtopiaipcenvelope.h>
#include "qtopiaserverapplication.h"
#include "qabstractserverinterface.h"
#include <custom.h>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <qexportedbackground.h>
#endif

E1ServerInterface::E1ServerInterface(QWidget *parent, Qt::WFlags flags)
: QAbstractServerInterface(parent, flags), m_dialer(0), m_newMessages("/Communications/Messages/NewMessages")
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    m_eb = new QExportedBackground(0, this);
    QObject::connect(m_eb, SIGNAL(wallpaperChanged()),
                     this, SLOT(wallpaperChanged()));
#endif

    wallpaperChanged();

    setGeometry(qApp->desktop()->rect());

    // Listen for showDialer message
    QCopChannel* dialerChannel = new QCopChannel( "QPE/Application/qpe", this );
    connect( dialerChannel, SIGNAL(received(QString,QByteArray)), this, SLOT(showDialer(QString,QByteArray)) );

    m_header = new E1Header(0, Qt::FramelessWindowHint |
                                Qt::Tool |
                                Qt::WindowStaysOnTopHint);

    DialerControl::instance();

    m_browser = new E1PhoneBrowser;

    E1Telephony *telephony = new E1Telephony(0);


    E1Incoming * incoming = new E1Incoming();
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(popupCallscreen()));
    QObject::connect(incoming, SIGNAL(showCallscreen()),
                     telephony, SLOT(display()));

    connect( &m_newMessages, SIGNAL(contentsChanged()), this, SLOT(messageCountChanged()) );
        
    QtopiaIpcEnvelope env("QPE/E1", "showHome()");
}

void E1ServerInterface::messageCountChanged()
{
    if( m_newMessages.value().toInt() != 0 ) {
        E1Dialog* dialog = new E1Dialog( this, E1Dialog::NewMessage );
        if( dialog->exec() == QDialog::Accepted )
            QtopiaIpcEnvelope env("QPE/Application/qtmail", "raise()");
        delete dialog;
    }
}

void E1ServerInterface::wallpaperChanged()
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    m_wallpaper = m_eb->wallpaper();
#endif
    update();
}

void E1ServerInterface::paintEvent(QPaintEvent *)
{
    if(m_wallpaper.isNull())
        return;

    QPainter p(this);
    p.drawTiledPixmap(rect(), m_wallpaper);
}

void E1ServerInterface::keyPressEvent(QKeyEvent *e)
{
    e->accept();
}

void E1ServerInterface::showEvent(QShowEvent *e)
{
    m_header->show();
    QWidget::showEvent(e);
}

void E1ServerInterface::mousePressEvent(QMouseEvent *e)
{
    if(m_header->isVisible())
        m_header->hide();
    else
        m_header->show();
    e->accept();
}

void E1ServerInterface::showDialer( const QString&, const QByteArray& )
{
}

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, E1ServerInterface);
