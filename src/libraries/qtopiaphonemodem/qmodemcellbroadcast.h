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

#ifndef QMODEMCELLBROADCAST_H
#define QMODEMCELLBROADCAST_H

#include <qcellbroadcast.h>

class QModemService;
class QModemCellBroadcastPrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCellBroadcast : public QCellBroadcast
{
    Q_OBJECT
public:
    explicit QModemCellBroadcast( QModemService *service );
    ~QModemCellBroadcast();

public slots:
    void setChannels( const QList<int>& list );

private slots:
    void sendChange();
    void sendAdd();
    void sendRemove();
    void cscb( bool ok, const QAtResult& result );
    void cscbQuery( bool ok, const QAtResult& result );
    void pduNotification( const QString& type, const QByteArray& pdu );
    void registrationStateChanged();
    void smsReady();

protected:
    void groupInitialized( QAbstractIpcInterfaceGroup *group );
    virtual bool removeBeforeChange() const;
    virtual bool removeOneByOne() const;

private:
    QModemCellBroadcastPrivate *d;

    QString command( int mode, const QList<int>& channels ) const;
};

#endif
