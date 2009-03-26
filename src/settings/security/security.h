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
#ifndef SECURITY_H
#define SECURITY_H

#ifdef QTOPIA_CELL
#include "ui_securityphone.h"
#else
#include "ui_securitybase.h"
#endif

#include <QDialog>
#include <QDateTime>

class WaitScreen;
class PhoneSecurity;

class Security
    : public QDialog, public Ui::SecurityBase
{
    Q_OBJECT

public:
    Security( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Security();

    void setVisible(bool vis);

protected:
    void accept();
    void done(int);
    void applySecurity();

private slots:
    void changePassCode();
    void clearPassCode();

    void markProtected(bool);
    void updateGUI();

    void phoneChanged(bool success);
    void phoneLocked(bool success);
    void phoneLockDone(bool success);

private:
    QString enterPassCode(const QString&, bool encrypt = true, bool last = true);
    QString passcode;
    bool valid;
    bool reentryCheck;
    QTime timeout;

#ifdef QTOPIA_CELL
    WaitScreen *mStatus;
    PhoneSecurity *phonesec;
                    // confirm pin number.
#endif
};

#endif
