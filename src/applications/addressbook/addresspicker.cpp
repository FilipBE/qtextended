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

#include "addresspicker.h"

#include <qlayout.h>

/*!
  \a tab is reparented for use in the picker. Take it back out if you want
  to regain ownership.
*/
AddressPicker::AddressPicker(AbTable* tab, QWidget* parent, const char* name, bool modal) :
    QDialog(parent,name,modal)
{
    QVBoxLayout* vb = new QVBoxLayout(this);
    tab->reparent(this,QPoint(0,0));
    table = tab;
    vb->addWidget(table);
}

void AddressPicker::setChoiceNames(const QStringList& list)
{
    table->setChoiceNames(list);
}

void AddressPicker::setSelection(int index, const QStringList& list)
{
    table->setChoiceSelection(index,list);
}

QStringList AddressPicker::selection(int index) const
{
    return table->choiceSelection(index);
}
