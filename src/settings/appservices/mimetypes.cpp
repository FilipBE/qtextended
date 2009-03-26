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

#include "mimetypes.h"
#include "itemfactory.h"

#include <QListWidget>
#include <QLabel>
#include <QWaitWidget>
#include <QVBoxLayout>

#include <QCoreApplication>
#include <QTimer>

#include <QMimeType>

#include <QDebug>

MimeTypes::MimeTypes(const QContent &app, QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_application(app)
{
    setWindowTitle(tr("Mime Types"));

    setObjectName("mimetypes");

    QVBoxLayout *vbox = new QVBoxLayout();

    QLabel *label = new QLabel(tr("Use %1 for the following file types", "%1=<application name>").arg(m_application.name()));
    label->setWordWrap(true);
    vbox->addWidget(label);

    m_types = new QListWidget();
    m_types->setWordWrap(true);
    m_types->setSortingEnabled(true);
    m_types->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbox->addWidget(m_types);

    setLayout(vbox);

    QTimer::singleShot(0, this, SLOT(loadState()));
}

MimeTypes::~MimeTypes()
{
}

void MimeTypes::accept()
{
    QMap<QString, QListWidgetItem *>::const_iterator i = m_typeDict.constBegin();
    while (i != m_typeDict.constEnd()) {
        if (i.value()->checkState() == Qt::Checked) {
            QMimeType::setDefaultApplicationFor(i.key(), m_application);
        } else {
            QContentList apps = QMimeType(i.key()).applications();

            apps.removeAll(m_application);

            if (!apps.isEmpty())
                QMimeType::setDefaultApplicationFor(i.key(), apps.first());
        }

        i++;
    }

    QDialog::accept();
}

void MimeTypes::loadState()
{
    QWaitWidget *waitWidget = new QWaitWidget(this);
    waitWidget->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loadMimeTypes();

    delete waitWidget;
}

void MimeTypes::loadMimeTypes()
{
    foreach (const QString &mimeType, m_application.mimeTypes()) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QListWidgetItem *item = createTypeItem(mimeType);

        if (item) {
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

            if (QMimeType(mimeType).application() == m_application)
                item->setCheckState(Qt::Checked);
            else
                item->setCheckState(Qt::Unchecked);

            m_types->addItem(item);
            m_typeDict.insert(mimeType, item);
        }
    }
}

