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

#include "qprintdialogcreator_p.h"
#include "ui_printdialogbase.h"

#include <QDebug>

/*!
    \internal
    Creates a print dialog.
*/
static void createPrintDialog(QPrintDialog *parent)
{
    Ui::PrintDialogBase *dialogBase = new Ui::PrintDialogBase();
    dialogBase->setupUi(parent);
}

typedef void (*QPrintDialogCreator)(QPrintDialog *parent);
extern QPrintDialogCreator _qt_print_dialog_creator;
/*!
    \internal
    _qt_print_dialog_creator is defined in QPrintDialog
    and called when a QPrintDialog is contructed to ensure
    that graphic representation of QPrintDialog fits to handheld devices.
*/
static int installCreatePrintDialog()
{
    _qt_print_dialog_creator = &createPrintDialog;
    return 0;
}

Q_CONSTRUCTOR_FUNCTION(installCreatePrintDialog)
