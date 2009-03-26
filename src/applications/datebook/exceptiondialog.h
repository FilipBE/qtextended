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
#ifndef EXCEPTIONDIALOG_H
#define EXCEPTIONDIALOG_H

#include "ui_exceptiondialogbase.h"


class ExceptionDialog : public QDialog, public Ui::ExceptionDialogBase
{
    Q_OBJECT
public:
    ExceptionDialog( QWidget *parent, Qt::WFlags f = 0 );

    enum SeriesRange
    {
        Earlier= 0x1,
        Selected= 0x02,
        Later= 0x04,

        All= Earlier| Selected | Later,
        Cancel = 0x0,
        RetainSelected = Earlier | Later,
        NotEarlier = Selected | Later,
        NotLater = Earlier | Selected,
    };

    int exec(bool editmode);
    int result() const;
};


#endif
