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

#include "phoneheader.h"
#include <QTimer>
#include "windowmanagement.h"
#include "qtopiaserverapplication.h"
#include <qscreen_qws.h>
#include "themecontrol.h"
#include <QList>
#include <QThemeItem>
#include <QThemeWidgetItem>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QPushButton>

/*!
    \class PhoneHeader
    \inpublicgroup QtUiModule
    \ingroup QtopiaServer::PhoneUI
    \brief The PhoneHeader class provides a dockable, themeable phone header.

    An image of this dialer screen using the Qt Extended theme can be found in
    the \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
   Create a new phone header with the specified \a parent widget and \a fl flags .
*/

PhoneHeader::PhoneHeader(QWidget *parent, Qt::WFlags fl)
    : QAbstractHeader(parent,fl), 
      themedView(new QThemedView())
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool |
                   Qt::WindowStaysOnTopHint);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addWidget(themedView);
    vb->setContentsMargins(0, 0, 0, 0);
    setLayout(vb);
    setWindowTitle("_decoration_");
    reservedSize();
    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl ) {
        ctrl->registerThemedView(themedView, "Title");
    }
    else 
        qLog(Component) << "PhoneHeader: ThemeControl not available, Theme will not work properly";
}
/*!
  \fn PhoneHeader::~PhoneHeader()

  Destroys the phone header.
 */

PhoneHeader::~PhoneHeader()
{
    delete themedView;
}

/*! \internal */
QSize PhoneHeader::reservedSize() const
{
    // reading the config file to know the percentage of the desktop
    // reserved for the header
    QSettings qpeCfg("Trolltech", "qpe");
    qpeCfg.beginGroup("Appearance");
    QString themeDir = Qtopia::qtopiaDir() + "etc/themes/";
    QString theme = qpeCfg.value("Theme").toString();
    QSettings themeCfg(themeDir + theme, QSettings::IniFormat);
    themeCfg.beginGroup("Theme");
    double percentage = themeCfg.value("HeaderSize", 0.15 ).toDouble();
    QRect headRect = qApp->desktop()->screenGeometry();
    headRect.setHeight(qRound(percentage * headRect.height()));
    themedView->setFixedSize(headRect.size());
    return headRect.size();
}

QTOPIA_REPLACE_WIDGET(QAbstractHeader,PhoneHeader);
