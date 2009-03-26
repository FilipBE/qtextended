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
#ifndef BEAMING_H
#define BEAMING_H

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QButtonGroup;

#include "ircontroller.h"

class Beaming : public QDialog
{
    Q_OBJECT

public:
    Beaming( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Beaming();

private slots:
    void chooseState(int c);
    void chooseProtocol(QListWidgetItem *);
    void stateChanged(IRController::State state);

private:
    IRController *irc;
    QListWidget *lb;
    int state;
    int protocol;
    QButtonGroup *bg;
};

#endif
