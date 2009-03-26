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
#ifndef QSIMTOOLKIT_H
#define QSIMTOOLKIT_H


#include <qcomminterface.h>
#include <qsimterminalresponse.h>
#include <qsimenvelope.h>
#include <qsimcontrolevent.h>

class QTOPIAPHONE_EXPORT QSimToolkit : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimToolkit( const QString& service = QString(), QObject *parent = 0,
                          QCommInterface::Mode mode = Client );
    ~QSimToolkit();

public slots:
    virtual void begin();
    virtual void end();
    virtual void sendResponse( const QSimTerminalResponse& resp );
    virtual void sendEnvelope( const QSimEnvelope& env );

    // Deprecated functions: use sendResponse() and sendEnvelope() instead.
    void mainMenuSelected( uint item );
    void clearText();
    void key( const QString& value );
    void input( const QString& value );
    void allowCallSetup();
    void denyCallSetup();
    void subMenuSelected( uint item );
    void subMenuExited();
    void idleScreen();
    void userActivity();
    void endSession();
    void moveBackward();
    void cannotProcess();
    void temporarilyUnavailable();
    void noResponseFromUser();
    void aborted();
    void help( QSimCommand::Type command, uint item=0 );

signals:
    void command( const QSimCommand& command );
    void beginFailed();
    void controlEvent( const QSimControlEvent& event );

protected:
    void emitCommandAndRespond( const QSimCommand& command );
};

#endif
