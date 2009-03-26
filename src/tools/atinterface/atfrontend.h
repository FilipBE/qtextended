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

#ifndef ATFRONTEND_H
#define ATFRONTEND_H

#include <qserialiodevice.h>
#include <qatresult.h>

class AtFrontEndPrivate;
class AtOptions;
class QAtResult;

class AtFrontEnd : public QObject
{
    Q_OBJECT
public:
    AtFrontEnd( const QString& startupOptions, QObject *parent=0 );
    ~AtFrontEnd();

    QSerialIODevice *device() const;
    void setDevice( QSerialIODevice *device );
    void setDataSource( QIODevice* dataSourceDevice );

    enum State
    {
        Offline,
        OnlineData,
        OnlineCommand
    };

    AtFrontEnd::State state() const;
    AtOptions *options() const;

    bool canMux() const;
    void setCanMux( bool value );

public slots:
    void setState( AtFrontEnd::State state );
    void requestExtra();
    void send( const QString& line );
    void send( QAtResult::ResultCode result );
    void stopRepeatLast();

signals:
    void commands( const QStringList& cmds );
    void extra( const QString& line, bool cancel );
    void remoteHangup();
    void enterCommandState();

private slots:
    void raiseDtr();
    void readyRead();
    void dsrChanged( bool value );
    void dataSourceReadyRead();
    void dataSourceClosed();

private:
    AtFrontEndPrivate *d;

    void writeCRLF();
    void writeBackspace();
    void parseCommandLine( const QString& line );
};

#endif
