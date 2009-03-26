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
#ifndef AddressPicker_H
#define AddressPicker_H

#include <qdialog.h>

class AbTable;

class AddressPicker : public QDialog {
public:
    AddressPicker(AbTable* table, QWidget* parent, const char* name=0, bool modal=false);

    void setChoiceNames(const QStringList&);
    void setSelection(int index, const QStringList&);
    QStringList selection(int index) const;

private:
    AbTable* table;
};

#endif
