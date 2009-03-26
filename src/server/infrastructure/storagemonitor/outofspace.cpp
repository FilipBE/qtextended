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

#include "outofspace.h"

#include <QMessageBox>
#include <QtopiaService>
#include <QSoftMenuBar>

OutOfSpace::OutOfSpace(const QString &msg, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setupUi(this);

    message->setText(msg);
    icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Critical));

    connect(remind, SIGNAL(clicked(bool)), this, SLOT(updateContext(bool)));
    connect(cleanup, SIGNAL(clicked(bool)), this, SLOT(updateContext(bool)));
    if (QtopiaService::apps("CleanupWizard").isEmpty()) {
        // no cleanup wizard
        cleanup->setEnabled(false);
        remind->setChecked(true);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
    }
}

void OutOfSpace::updateContext(bool /*checked*/)
{
    if ( sender() == cleanup ) //cleanup option jumps straight to cleanup wizard
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
    else if ( sender() == remind )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
}

void OutOfSpace::accept()
{
    if (cleanup->isChecked()) {
        done(CleanupNow);
    } else {
        switch (delay->currentIndex()) {
        case 0:
            done(HourDelay);
            break;
        case 1:
            done(DayDelay);
            break;
        case 2:
            done(WeekDelay);
            break;
        case 3:
            done(Never);
            break;
        }
    }
}

