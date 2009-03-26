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

#include "appdetails.h"
#include "mimetypes.h"
#include "itemfactory.h"

#include <QListWidget>
#include <QLabel>
#include <QWaitWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <QtopiaApplication>
#include <QTimer>

#include <QtopiaService>
#include <QSettings>

#include <QDebug>

AppDetails::AppDetails(const QContent &app, QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_application(app)
{
    setWindowTitle(tr("Application Details"));

    QVBoxLayout *vbox = new QVBoxLayout();

    QLabel *label = new QLabel(tr("Use %1 for", "%1=<application name>").arg(m_application.name()));
    label->setWordWrap(true);
    vbox->addWidget(label);

    m_services = new QListWidget();
    m_services->setWordWrap(true);
    m_services->setSortingEnabled(true);
    m_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbox->addWidget(m_services);

    QPushButton *button = new QPushButton(tr("Mime Types"));
    vbox->addWidget(button);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(showMimeTypes(bool)));

    setLayout(vbox);

    QTimer::singleShot(0, this, SLOT(loadState()));
}

AppDetails::~AppDetails()
{
}

void AppDetails::accept()
{
    QMap<QString, QListWidgetItem *>::const_iterator i = m_serviceDict.constBegin();
    while (i != m_serviceDict.constEnd()) {
        QSettings binding("Trolltech", QtopiaService::binding(i.key()));
        binding.beginGroup("Service");

        if (i.value()->checkState() == Qt::Checked) {
            binding.setValue("default", m_application.executableName());
        } else if (binding.value("default").toString() == m_application.executableName()) {
            binding.remove("default");
        }
        i++;
    }

    QDialog::accept();
}

void AppDetails::loadState()
{
    QWaitWidget *waitWidget = new QWaitWidget(this);
    waitWidget->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loadServices();

    delete waitWidget;
}

void AppDetails::showMimeTypes(bool)
{
    MimeTypes *mimeTypes = new MimeTypes(m_application);
    QtopiaApplication::execDialog(mimeTypes);
    delete mimeTypes;
}

void AppDetails::loadServices()
{
    foreach (const QString &service, QtopiaService::services(m_application.executableName())) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QListWidgetItem *item = createServiceItem(service);

        if (item) {
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

            if (QtopiaService::app(service) == m_application.executableName())
                item->setCheckState(Qt::Checked);
            else
                item->setCheckState(Qt::Unchecked);

            m_services->addItem(item);
            m_serviceDict.insert(service, item);
        }
    }
}

