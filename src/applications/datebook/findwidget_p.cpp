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

#include "findwidget_p.h"

#include <qcategorymanager.h>

FindWidget::FindWidget( const QString &appName, QWidget *parent )
    : QWidget( parent ),
      mStrApp( appName ),
      mDate( QDate::currentDate() )
{
    setupUi( this );
    setMaximumSize( sizeHint() );
    cmbCat->selectFilter( QCategoryFilter(QCategoryFilter::All) );
    // hide junk for the moment...
    lblStartDate->hide();
    cmdDate->hide();
    QObject::connect( cmdDate, SIGNAL(valueChanged(QDate)),
                      this, SLOT(setDate(QDate)) );

    QObject::connect( cmdFind, SIGNAL(clicked()),
                      this, SLOT(slotFindClicked()) );
}

FindWidget::~FindWidget()
{
}

QString FindWidget::findText() const
{
    return txtFind->text();
}

void FindWidget::slotFindClicked()
{
    lblStatus->setText( "" );
    if ( cmdDate->isVisible() )
        emit signalFindClicked( findText(),
                                mDate,
                                chkCase->isChecked(),
                                false,
                                cmbCat->selectedFilter() );
    else
        emit signalFindClicked( findText(), chkCase->isChecked(),
                                false,
                                cmbCat->selectedFilter() );
}

void FindWidget::setUseDate( bool show )
{
    if ( show ) {
        lblStartDate->show();
        cmdDate->show();
    } else {
        lblStartDate->hide();
        cmdDate->hide();
    }
    //chkBackwards->setDisabled( show );
}

void FindWidget::setDate( const QDate &dt )
{
    mDate = dt;
}

void FindWidget::slotNotFound()
{
    lblStatus->setText( tr("String Not Found.") );
}

void FindWidget::slotWrapAround()
{
    lblStatus->setText( tr("End reached, starting at beginning") );
}
