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

#ifndef SYSMESSAGES_H
#define SYSMESSAGES_H

#include <qobject.h>
#include <qstring.h>
#include <qtopiaabstractservice.h>
#include <qmap.h>
#include <qtopiachannel.h>
#include <qdatetime.h>

#ifdef MAIL_EXISTS
/*!
  \internal
  \class SysMail
    \inpublicgroup QtPkgManagementModule
  \brief Helper class for SysMessagesService

  The SysMail class encapsulates the information in a system sms message.
*/
class SysMail
{
public:
    SysMail(){}
    SysMail(const QDateTime &time, const QString& subject, const QString &text) :
        _time(time), _subject(subject),_text(text){}

    QDateTime time() { return _time; }
    void setTime( const QDateTime &time ) { this->_time = time; }

    QString subject() { return this->_subject; }
    void setSubject( const QString &subject) {this->_subject = subject; }

    QString text() { return this->_text; }
    void setText( const QString &text) { this->_text = text; }

private:
    QDateTime _time;
    QString _subject;
    QString _text;
};
#endif

class SysMessagesService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SysMessages;

public:
    SysMessagesService( QObject *parent );
    ~SysMessagesService();

public slots:
    void showDialog( const QString &title, const QString &text );

#ifdef MAIL_EXISTS
    void sendMessage( const QString &subject, const QString &text );
    void collectMessages();

private slots:
    void received(const QString &message, const QByteArray &data);

private:
    void passOnMessage( int messageId );
    void ackMessage( int messageId );

    QtopiaChannel *sysMessagesChannel;
    QMap< int, SysMail > sysMailMap;
    int maxMessageId;
    bool keepRunning;
#endif
};
#endif
