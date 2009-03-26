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
#ifndef PHONESECURITY_H
#define PHONESECURITY_H

#include <QObject>
#include <qpinmanager.h>

class PhoneSecurity : public QObject {
    Q_OBJECT
public:
    PhoneSecurity(QObject * parent);
    void setLockType(int t);
    void markProtected(int t, bool b, const QString& pw);
    void changePassword(int t, const QString& old, const QString& new2);

signals:
    void changed(bool);
    void locked(bool);
    void lockDone(bool);

private slots:
    void lockStatus(const QString& type, bool enabled );
    void setLockStatusResult(const QString& type, bool valid );
    void changePinResult(const QString& type, bool valid );

private:
    QPinManager *pinManager;
    int locktype;
};

#endif
