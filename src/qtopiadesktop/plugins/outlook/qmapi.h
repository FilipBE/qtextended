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
#ifndef QMAPI_H
#define QMAPI_H

#include <QObject>
#include <QStringList>

struct IUnknown;

namespace QMAPI {

    class Session;
    class SessionPrivate;
    class Contact;
    class ContactPrivate;
    class Appointment;
    class AppointmentPrivate;
    class Task;
    class TaskPrivate;

    class Session : public QObject
    {
    public:
        Session( QObject *parent = 0 );
        ~Session();

        bool connected() const;

        Contact *openContact( IUnknown *obj );
        Appointment *openAppointment( IUnknown *obj, bool isException );
        Task *openTask( IUnknown *obj );

    private:
        SessionPrivate *d;
    };

    class Contact : public QObject
    {
        friend class Session;

    private:
        Contact( QObject *parent = 0 );

    public:
        ~Contact();

        QString Body();
        QString Email1Address();
        QString Email2Address();
        QString Email3Address();

    private:
        ContactPrivate *d;

    };

    class Appointment : public QObject
    {
        friend class Session;

    private:
        Appointment( QObject *parent = 0 );

    public:
        ~Appointment();

        QString Body();
        bool IsBirthday();

    private:
        AppointmentPrivate *d;

    };

    class Task : public QObject
    {
        friend class Session;

    private:
        Task( QObject *parent = 0 );

    public:
        ~Task();

        QString Body();

    private:
        TaskPrivate *d;

    };

};

#endif

