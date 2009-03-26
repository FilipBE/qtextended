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

#ifndef QSMSREADER_H
#define QSMSREADER_H

#include <qcomminterface.h>
#include <qsmsmessage.h>

class QTOPIAPHONE_EXPORT QSMSReader : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSMSReader( const QString& service = QString(),
                         QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QSMSReader();

    int unreadCount() const;
    QStringList unreadList() const;

    int usedMessages() const;
    int totalMessages() const;

    bool ready() const;

public slots:
    virtual void check();
    virtual void firstMessage();
    virtual void nextMessage();
    virtual void deleteMessage( const QString& id );
    virtual void setUnreadCount( int value );

protected:
    void setReady( bool value );

signals:
    void unreadCountChanged();
    void messageCount( int total );
    void fetched( const QString& id, const QSMSMessage& m );
    void readyChanged();
};

#endif
