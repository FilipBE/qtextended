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

#ifndef QMODEMPPPDMANAGER_P_H
#define QMODEMPPPDMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qdialoptions.h>
#include <qphonecall.h>
#include <qphonecallmanager.h>
#include <qbytearray.h>
#include <qprocess.h>

class QSerialIODeviceMultiplexer;
class QProcess;
class QtopiaChannel;

class QModemPPPdManager : public QObject
{
    Q_OBJECT
public:
    QModemPPPdManager( QSerialIODeviceMultiplexer *mux, QObject *parent );
    ~QModemPPPdManager();

    bool isActive() const;
    bool dataCallsPossible() const;
    bool start( const QDialOptions& options );
    void stop();

private:
    void dataCallStop();
    void raiseDtr();
    void runChatFile( const QString& filename, const char *slot );
    void changeState( int value );

private slots:
    void pppdListen( const QString& msg, const QByteArray& data );
    void raiseDtrAndHangup();
    void connected( bool ok );
    void disconnected();
    void pppdStateChanged( QProcess::ProcessState state );

signals:
    void dataCallActive();
    void dataCallInactive();
    void dataStateUpdate( QPhoneCall::DataState state );

private:
    QSerialIODeviceMultiplexer *mux;
    QProcess *process;
    QDialOptions options;
    QtopiaChannel *pppdChannel;
    bool active;
    bool terminated;
};

#endif
