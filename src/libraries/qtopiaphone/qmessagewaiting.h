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

#ifndef QMESSAGEWAITING_H
#define QMESSAGEWAITING_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QMessageWaitingPrivate;
class QMessageWaitingStatusPrivate;

class QTOPIAPHONE_EXPORT QMessageWaitingStatus
{
public:
    QMessageWaitingStatus();
    QMessageWaitingStatus( const QMessageWaitingStatus& other );
    ~QMessageWaitingStatus();

    QMessageWaitingStatus& operator=( const QMessageWaitingStatus& other );

    QTelephony::CallClass callClass() const;
    void setCallClass( QTelephony::CallClass value );

    QString number() const;
    void setNumber( const QString& value );

    bool unreadMessagesWaiting() const;
    void setUnreadMessagesWaiting( bool value );

    int unreadCount() const;
    void setUnreadCount( int value );

    int readCount() const;
    void setReadCount( int value );

    int urgentUnreadCount() const;
    void setUrgentUnreadCount( int value );

    int urgentReadCount() const;
    void setUrgentReadCount( int value );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QMessageWaitingStatusPrivate *d;
};

class QTOPIAPHONE_EXPORT QMessageWaiting : public QCommInterface
{
    Q_OBJECT
public:
    explicit QMessageWaiting( const QString& service = QString::null,
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QMessageWaiting();

    QList<QMessageWaitingStatus> status() const;
    QMessageWaitingStatus totalStatus() const;

signals:
    void changed();

protected:
    void updateStatus( const QMessageWaitingStatus& status );
    void clearAllStatus();

private:
    QMessageWaitingPrivate *d;

    void updateValueSpace();
};

Q_DECLARE_USER_METATYPE(QMessageWaitingStatus)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QMessageWaitingStatus>)

#endif
