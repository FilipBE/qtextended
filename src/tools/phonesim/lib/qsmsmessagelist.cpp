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

#include "qsmsmessagelist.h"
#include <qdebug.h>

QSMSMessageList::QSMSMessageList()
{
SMSList.clear();
statusList.clear();
deletedFlagList.clear();
}

QSMSMessageList::~QSMSMessageList()
{
}

void QSMSMessageList::appendSMS( const QByteArray &m )
{
    SMSList.append(m);
    statusList.append(QSMSMessageList::REC_UNREAD);
    deletedFlagList.append(false);
}


void QSMSMessageList::deleteSMS( int i )
{
    deletedFlagList[i] = true;
}

int QSMSMessageList::count() const
{
    return SMSList.count();
}

QSMSMessageList::SMSStatus QSMSMessageList::getStatus( int i ) const
{
   return statusList[i];
}

void QSMSMessageList::setStatus( const SMSStatus &s, int i )
{
    statusList[i] = s;
}


bool QSMSMessageList::getDeletedFlag( int i ) const
{
    return deletedFlagList[i];
}

void QSMSMessageList::setDeletedFlag( bool b, int i )
{
    deletedFlagList[i] = b;
}

QByteArray & QSMSMessageList::readSMS( int i )
{
   if ( statusList[i] == QSMSMessageList::REC_UNREAD ) {
        statusList[i] = QSMSMessageList::REC_READ;
  }

   return SMSList[i];
}

QByteArray & QSMSMessageList::operator[]( int i )
{
    return SMSList[i];

}



