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

#ifndef QMODEMSMSREADER_H
#define QMODEMSMSREADER_H

#include <qsmsreader.h>

class QModemService;
class QModemSMSReaderPrivate;
class QAtResult;
class QSMSTaggedMessage;

class QTOPIAPHONEMODEM_EXPORT QModemSMSReader : public QSMSReader
{
    Q_OBJECT
public:
    explicit QModemSMSReader( QModemService *service );
    ~QModemSMSReader();

public slots:
    void check();
    void firstMessage();
    void nextMessage();
    void deleteMessage( const QString& id );
    void setUnreadCount( int value );

protected:
    virtual QString messageStore() const;
    virtual QString messageListCommand() const;
    virtual void simDownload( const QSMSMessage& message );

private slots:
    void resetModem();
    void smsReady();
    void nmiStatusReceived( bool ok, const QAtResult& result );
    void newMessageArrived();
    void pduNotification( const QString& type, const QByteArray& pdu );
    void cpmsDone( bool ok, const QAtResult& result );
    void storeListDone( bool ok, const QAtResult& result );

private:
    QModemSMSReaderPrivate *d;

    void extractMessages( const QString& store, const QAtResult& result );
    void check(bool force);
    void fetchMessages();
    void joinMessages();
    void updateUnreadCount();
    void updateUnreadList();
    bool joinMessages( QList<QSMSTaggedMessage *>& messages,
                       QStringList& toBeDeleted );
    bool dispatchDatagram( QSMSTaggedMessage *m );
};

#endif
