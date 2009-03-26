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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef QTOPIASYSTEMTESTMODEM_H
#define QTOPIASYSTEMTESTMODEM_H

#if ! defined Q_QDOC

#ifdef QTUITEST_USE_PHONESIM

#include <QObject>
#include <qtopiabase/qtopiaglobal.h>

class QtopiaSystemTestPrivate;
class QtopiaSystemTestModemPrivate;
class SimApplication;

class QTUITEST_EXPORT QtopiaSystemTestModem : public QObject
{
Q_OBJECT

public slots:
    void startIncomingCall( const QString& number, bool dialBack );
    void startIncomingCall( const QString& number );

    void incomingSMS( const int type, const QString &sender, const QString &serviceCenter, const QString &text );

    void send(QString const&);
    QString getVariable(QString const& key) const;
    void setVariable(QString const& key, QString const& value, bool persistent = false);
    void resetPersistentVariables();

    bool holdWillFail() const;
    void setHoldWillFail( bool value );
    bool activateWillFail() const;
    void setActivateWillFail( bool value );
    bool joinWillFail() const;
    void setJoinWillFail( bool value );
    bool deflectWillFail() const;
    void setDeflectWillFail( bool value );
    void hangupAll();
    void hangupConnected();
    void hangupHeld();
    void hangupConnectedAndHeld();
    void hangupCall( int id );
    int multipartyLimit() const;
    void setMultipartyLimit( int value );
    void expectCommand( QString const&, QString const& );
    void expectResponse( QString const&, QString const& );
    bool waitCommand( QString const& key, int timeout = 10000 );
    bool waitResponse( QString const& key, int timeout = 10000 );

    void installToolkitApp( SimApplication *app );

    void destroy();

private:
    bool isValid() const;

    QtopiaSystemTestModem(QtopiaSystemTestPrivate *parent);
    ~QtopiaSystemTestModem();
    QtopiaSystemTestModemPrivate *d;
    friend class QtopiaSystemTestModemPrivate;
    friend class QtopiaSystemTestPrivate;
    friend class QtopiaSystemTest;


};
#endif // QTUITEST_USE_PHONESIM

#endif

#endif
