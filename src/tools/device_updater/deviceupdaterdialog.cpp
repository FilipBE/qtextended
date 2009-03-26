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

#include "deviceupdaterdialog.h"
#include "deviceupdater.h"

DeviceUpdaterDialog::DeviceUpdaterDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    ui = new DeviceUpdaterWidget( this );
    QVBoxLayout *vb = new QVBoxLayout;
    vb->addWidget( ui );
    vb->setMargin( 1 );
    setLayout( vb );
    QObject::connect( ui, SIGNAL(done(int)),
            this, SLOT(done(int)) );
}

DeviceUpdaterDialog::~DeviceUpdaterDialog()
{
}
