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

#include "example.h"
#include <qpushbutton.h>
#include <qsoftmenubar.h>

/*
 *  Constructs a Example which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
Example::Example(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    setupUi(this); // reads the .ui file to determine widgets and layout
    // Construct context menu, available to the user via Qtopia's soft menu bar.
    QMenu *menu = QSoftMenuBar::menuFor(this);
    Q_UNUSED(menu); // Normally, we would use "menu" to add more actions.
    // The context menu will not be active unless it has at least one action.
    // Tell Qtopia to provide the "Help" option, which will enable the user to
    // read the Help documentation for this application.
    QSoftMenuBar::setHelpEnabled(this,true);
}

/*
 *  Destroys the object and frees any allocated resources
 */
Example::~Example()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Every time the quit button is clicked, this slot will be invoked
 *  (see QMetaObject::connectSlotsByName()).
 */
void Example::on_QuitButton_clicked()
{
    close();
}

