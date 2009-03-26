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

#include "finddialog.h"
#include "findwidget_p.h"

#include <qlayout.h>
#include <qpushbutton.h>

FindDialog::FindDialog( const QString &appName, QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( tr("Find") );
    QVBoxLayout *vb = new QVBoxLayout(this);
    fw = new FindWidget( appName, this );
    vb->addWidget(fw);
    QObject::connect( fw, SIGNAL(signalFindClicked(QString,bool,bool,int)),
                      this, SIGNAL(signalFindClicked(QString,bool,bool,int)) );
    QObject::connect( fw, SIGNAL(signalFindClicked(QString,QDate,bool,bool,int)),
                      this, SIGNAL(signalFindClicked(QString,QDate,bool,bool,int)) );
    d = 0;
}

FindDialog::~FindDialog()
{
}

QString FindDialog::findText() const
{
    return fw->findText();
}

void FindDialog::setUseDate( bool show )
{
    fw->setUseDate( show );
}

void FindDialog::setDate( const QDate &dt )
{
    fw->setDate( dt );
}

void FindDialog::slotNotFound()
{
    fw->slotNotFound();
}

void FindDialog::slotWrapAround()
{
    fw->slotWrapAround();
}
