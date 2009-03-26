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

#include "inputdialog.h"

#include <QVBoxLayout>
#include <QKeyEvent>


InputDialog::InputDialog(QWidget *parent) 
: QDialog(parent) 
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    _edit = new QLineEdit(this);
    layout->addWidget(_edit);
    setLayout(layout);
    setFocusProxy(_edit);
}


void InputDialog::setText(const QString &text) 
{
    _edit->setText(text);
}

QString InputDialog::text() 
{
    return _edit->text();
}

void InputDialog::keyPressEvent(QKeyEvent *event) 
{
    if (event->key() == Qt::Key_Select) {
        accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}
