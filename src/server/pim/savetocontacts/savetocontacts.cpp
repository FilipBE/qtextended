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

#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include "qabstractmessagebox.h"

#include "savetocontacts.h"

void SavePhoneNumberDialog::savePhoneNumber(const QString &number, QWidget *parent)
{
    QAbstractMessageBox *box = QAbstractMessageBox::messageBox( parent, 
            qApp->translate("SavePhoneNumberDialog", "Save to Contacts"),
            "<qt>" + qApp->translate("SavePhoneNumberDialog", "Create a new contact?") + "</qt>",
            QAbstractMessageBox::Warning,
            QAbstractMessageBox::Yes, QAbstractMessageBox::No );

    if (!box) {
        qLog(Component) << "SavePhoneNumberDialog: No message box available";
        return;
    }

    if (QtopiaApplication::execDialog(box) == QAbstractMessageBox::Yes) {
        QtopiaServiceRequest req( "Contacts", "createNewContact(QString)" );
        req << number;
        req.send();
    } else {
        QtopiaServiceRequest req( "Contacts", "addPhoneNumberToContact(QString)" );
        req << number;
        req.send();
    }

    delete box;
}
