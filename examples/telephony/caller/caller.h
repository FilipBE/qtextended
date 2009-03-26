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

#ifndef CALLER_H
#define CALLER_H

#include <QMainWindow>
#include <QPhoneCall>
#include <QPhoneCallManager>
#include "ui_caller.h"

class Caller : public QWidget, public Ui::Form
{
    Q_OBJECT
public:
    Caller(QWidget *parent = 0, Qt::WindowFlags flags = 0);

private slots:
    void dial();
    void dialData();
    void accept();
    void stateChanged(const QPhoneCall& call);
    void dataStateChanged(const QPhoneCall& call);
    void newCall(const QPhoneCall& call);
    void dataReady();
    void dataClosed();

private:
// Left-aligned for correct indenting in docs.
QPhoneCallManager *mgr;

    QIODevice *dataDevice;

    void dataWrite(const char *buf, int len);
};

#endif
