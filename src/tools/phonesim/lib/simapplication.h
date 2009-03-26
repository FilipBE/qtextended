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

#ifndef SIMAPPLICATION_H
#define SIMAPPLICATION_H

#include "phonesim.h"
#include <qsimcommand.h>
#include <qsimterminalresponse.h>
#include <qsimenvelope.h>
#include <qsimcontrolevent.h>

class SimApplicationPrivate;

class SimApplication : public QObject
{
    Q_OBJECT
    friend class SimRules;
public:
    SimApplication( QObject *parent = 0 );
    ~SimApplication();

    void command( const QSimCommand& cmd,
                  QObject *target, const char *slot,
                  QSimCommand::ToPduOptions options
                        = QSimCommand::NoPduOptions );

public slots:
    void controlEvent( const QSimControlEvent& event );
    virtual void start();
    virtual void abort();

protected slots:
    virtual void mainMenu() = 0;
    virtual void mainMenuSelection( int id );
    virtual void mainMenuHelpRequest( int id );

private:
    SimApplicationPrivate *d;

    void setSimRules( SimRules *rules );
    bool execute( const QString& cmd );
    void response( const QSimTerminalResponse& resp );
};

class DemoSimApplication : public SimApplication
{
    Q_OBJECT
public:
    DemoSimApplication( QObject *parent = 0 );
    ~DemoSimApplication();

protected slots:
    void mainMenu();
    void mainMenuSelection( int id );
    void sendSportsMenu();
    void sportsMenu( const QSimTerminalResponse& resp );
    void startSticksGame();
    void sticksGameShow();
    void sticksGameLoop( const QSimTerminalResponse& resp );
    void getInputLoop( const QSimTerminalResponse& resp );
    void sticksGamePlayAgain( const QSimTerminalResponse& resp );
    void sendToneMenu();
    void toneMenu( const QSimTerminalResponse& resp );
    void sendIconMenu();
    void iconMenu( const QSimTerminalResponse& resp );
    void sendIconSEMenu();
    void iconSEMenu( const QSimTerminalResponse& resp );
    void sendDisplayText();
    void displayTextResponse( const QSimTerminalResponse& resp );
    void sendBrowserMenu();
    void browserMenu( const QSimTerminalResponse& resp );

private:
    int sticksLeft;
    bool immediateResponse;
};

#endif
