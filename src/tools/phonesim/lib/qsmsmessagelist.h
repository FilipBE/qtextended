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

#ifndef QSMSMESSAGELIST_H
#define QSMSMESSAGELIST_H

#include <QList>
#include <QByteArray>

class QSMSMessageList
{
public:
    enum SMSStatus{
        REC_UNREAD  =0,
        REC_READ    =1,
        STO_UNSENT  =2,
        STO_SENT    =3
    };

    QSMSMessageList();
    ~QSMSMessageList();

    void appendSMS( const QByteArray & );
    void deleteSMS( int );  //note SMS's are not actually physically deleted. A flag is set

    int count() const; //returns the total number, even those that are 'deleted'

    SMSStatus getStatus( int ) const;
    void  setStatus( const SMSStatus &, int );

    bool getDeletedFlag( int ) const;
    void setDeletedFlag( bool, int );

    QByteArray & readSMS( int );//returns and sets the status of an SMS
    QByteArray & operator[]( int );//only returns an SMS, does not set status

private:
    QList<QByteArray> SMSList;
    QList<SMSStatus> statusList;
    QList<bool> deletedFlagList;
};

#endif
