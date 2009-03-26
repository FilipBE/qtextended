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

#ifndef ATCOMMANDS_H
#define ATCOMMANDS_H

#include "atsessionmanager.h"
#include "atoptions.h"
#include "atindicators.h"
#include "atfrontend.h"
#include "atcallmanager.h"
#include "atcustom.h"

#include <QServiceNumbers>
#include <QSlotInvoker>
#include <QMap>

#ifdef QTOPIA_CELL
class AtGsmCellCommands;
class AtSmsCommands;
#endif
#ifdef QTOPIA_BLUETOOTH
class AtBluetoothCommands;
#endif
class AtGsmNonCellCommands;
class AtV250Commands;
class AtManager;
class AtFrontEnd;
class QNetworkRegistration;

class AtCommands : public QObject
{
    Q_OBJECT
public:
    AtCommands( AtFrontEnd *frontEnd, AtSessionManager *manager );
    ~AtCommands();

    AtFrontEnd *frontEnd() const;
    AtSessionManager *manager() const;
    AtOptions *options() const;
    AtGsmNonCellCommands *atgnc() const;

    bool invokeCommand( const QString& cmd, const QString& params );
    void add( const QString& name, QObject *target, const char *slot );

    QAtResult::ResultCode extendedError;
    QAtResult::ResultCode result;
    QStringList cmds;
    int cmdsPosn;

public slots:
    void send( const QString& line );
    void done( QAtResult::ResultCode result = QAtResult::OK );
    void doneWithExtendedError( QAtResult::ResultCode result );
    void ignore();
    void notAllowed();

    // these AT commands are "ambiguous" - used by both cell & noncell.
    void atd( const QString& params );
    void atcfun( const QString& params );
    void atcimi( const QString& params );

private slots:
    void commands( const QStringList& cmds );
    void processNextCommand();

private:
    AtFrontEnd *atFrontEnd;
    AtSessionManager *atManager;

    AtGsmNonCellCommands *m_atgnc;
#ifdef QTOPIA_CELL
    AtGsmCellCommands *m_atgcc;
    AtSmsCommands *m_atsms;
#endif
#ifdef QTOPIA_BLUETOOTH
    AtBluetoothCommands *m_atbtc;
#endif
    AtV250Commands *m_atv250c;

    bool dataCallRequested;
    QMap<QString, QSlotInvoker *> invokers;

private slots:
    void stateChanged( int callID, AtCallManager::CallState state,
                       const QString& number, const QString& type );
    void deferredResult( AtCommands *handler, QAtResult::ResultCode result );
    void ring( const QString& number, const QString& type );
    void dialingOut( bool asynchronous, bool transparent, bool gprs );
    void outgoingConnected( const QString& number );
    void callWaiting( const QString& number, const QString& type );
    void noCarrier();


};

#endif
