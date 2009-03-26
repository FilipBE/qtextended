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
#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <qdglobal.h>
#include <QThread>
#include <QMutex>
#include <QByteArray>

typedef void* HANDLE;

class QD_EXPORT IOThread : public QThread
{
    Q_OBJECT
public:
    IOThread( QObject *parent = 0 );
    ~IOThread();

    void run();
    void write();
    void abort();
    void quit();

signals:
    void emitReadyRead();
    void ioThreadStopped();
    void dsrChanged(bool);

public:
    HANDLE handle;
    QByteArray rb;
    QMutex rbm;
    QByteArray wb;
    QMutex wbm;
    HANDLE writeSem;
    HANDLE quitSem;
    enum State {
        Idle, Wait, Read, Write, Quit
    } currentState, nextState;
    bool brokenSerial;
    bool dsr;
};

#endif
