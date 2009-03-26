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
#if defined(QTOPIA_APPLICATION)
#include <qtopia/quniqueid.h>
#else
#include "quniqueid.h"
#endif

#include "pimgen.h"

#include <QDebug>
#include <QMessageBox>
#include <QtopiaIpcEnvelope>
#include <QtopiaApplication>

PimGen::PimGen(QWidget *parent, Qt::WFlags f) 
    : QDialog(parent, f)
{
    setupUi(this);
    progressLabel->hide();
    progressBar->hide();

    QtopiaChannel *myChannel = new QtopiaChannel( "QPE/Tools/PimDataGenerator", this );
    connect( myChannel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(handleReply(QString,QByteArray)));
}

PimGen::~PimGen()
{
}

void PimGen::accept()
{
    if (progressBar->isVisible())
            return;
    int cCount = contactCount->value();
    int aCount = appointmentCount->value();
    int tCount = taskCount->value();
    int count = cCount + aCount + tCount;


    if (count > 0) {
        progressBar->show();
        progressBar->setMaximum(count);
        progressBar->setValue(0);
        {
            QtopiaIpcEnvelope e("QPE/Application/pimdata", "generateData(int,int,int)");
            e << cCount;
            e << aCount;
            e << tCount;
        }
    } else {
        QDialog::accept();
    }
}

void PimGen::handleReply(const QString &msg, const QByteArray &data)
{
    if (msg == "recordsProcessed(int)") {
        QDataStream stream( data );
        int v;
        stream >> v;
        progressBar->setValue(v);
    } else if (msg == "failed()" ) {
        QMessageBox::warning(this, "Failed", "Failed to generate records");
        QDialog::reject();
    } else if (msg == "completed()" ) {
        QMessageBox::information(this,"Completed", "Records successfully generated");
        QDialog::accept();
    }
}
