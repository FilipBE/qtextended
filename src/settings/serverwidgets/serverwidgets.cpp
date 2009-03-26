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

#include "serverwidgets.h"

#include <QtopiaIpcEnvelope>
#include <QtopiaApplication>

#include <QDebug>
#include <QSettings>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

static struct {
    QString description;
    const char *defaultmapping;
    const char *serverinterface;
    const char *browserscreen;
} configurations[] =
    { 
      { QObject::tr("Default Qt Extended"), 0, 0, 0 }, 
      { QObject::tr("Wheel browser"), 0, 0, "Wheel" },
      { QObject::tr("E1 example"), "E1", 0, 0 },
      { QObject::tr("E2 example"), "E2", 0, 0 } 
    };

ServerWidgetSettings::ServerWidgetSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowTitle(tr("Server Widgets"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QLabel *label = new QLabel(this);
    label->setWordWrap(true);
    layout->addWidget(label);

    label->setText(tr("<i>Select the Qt Extended server configuration:</i>"));
    for(uint ii = 0; ii < sizeof(configurations) / sizeof(configurations[0]); ++ii) {
        QPushButton *pb = new QPushButton(configurations[ii].description, this);
        pb->setProperty("configReference", ii);
        layout->addWidget(pb);
        QObject::connect(pb, SIGNAL(clicked()), this, SLOT(clicked()));
    }
}

ServerWidgetSettings::~ServerWidgetSettings()
{
}

void ServerWidgetSettings::clicked()
{
    QObject *button = sender();
    Q_ASSERT(button);
    int config = button->property("configReference").toInt();

    QSettings cfg("Trolltech", "ServerWidgets");
    cfg.beginGroup("Mapping");
    cfg.remove("Default");
    cfg.remove("ServerInterface");
    cfg.remove("BrowserScreen");

    if(configurations[config].defaultmapping)
        cfg.setValue("Default", configurations[config].defaultmapping);
    if(configurations[config].serverinterface)
        cfg.setValue("ServerInterface", configurations[config].serverinterface);
    if(configurations[config].browserscreen)
        cfg.setValue("BrowserScreen", configurations[config].browserscreen);

    {
        QtopiaIpcEnvelope env( "QPE/System", "restart()" );
    }
    accept();
}

