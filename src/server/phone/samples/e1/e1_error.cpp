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

#include "e1_error.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "e1_bar.h"
#include <QPixmap>
#include <QLabel>
#include <QSizePolicy>

void E1Error::error(const QString &str)
{
    E1Error * e = new E1Error(str);
    e->exec();
    delete e;
}

E1Error::E1Error(const QString &error)
: E1Dialog(0, Generic)
{
    QWidget *myWid = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(myWid);
    QLabel *pix = new QLabel(myWid);
    pix->setPixmap(QPixmap(":image/alert_warning"));
    layout->addWidget(pix);
    QLabel *desc = new QLabel(myWid);
    desc->setText(error);
    layout->addWidget(desc);

    setContentsWidget(myWid);

    E1Button *button = new E1Button;
    button->setFlag(E1Button::Expanding);
    button->setText("OK");
    bar()->addItem(button);
    QObject::connect(button, SIGNAL(clicked()),
                     this, SLOT(ok()));

}

void E1Error::ok()
{
    accept();
}
