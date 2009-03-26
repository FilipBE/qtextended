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
#include "iothread.h"
#include <trace.h>
QD_LOG_OPTION(Modem)

#include <windows.h>

IOThread::IOThread( QObject *parent )
    : QThread( parent ), handle( INVALID_HANDLE_VALUE ), dsr( false )
{
    writeSem = CreateEvent(NULL, false, false, NULL); // This lets the main thread signal the IO thread
    Q_ASSERT(writeSem);
    quitSem = CreateEvent(NULL, false, false, NULL); // This lets the main thread signal the IO thread
    Q_ASSERT(quitSem);
    currentState = Idle;
    nextState = Idle;
}

IOThread::~IOThread()
{
    CloseHandle(writeSem);
    CloseHandle(quitSem);
}

void IOThread::run()
{
    TRACE(Modem) << "IOThread::run";
    int maxlen = 200;
    char data[201];
    DWORD dwBytesRead;
    DWORD evt;
    DWORD dwBytesWritten;
    LONG ret;
    bool ok;

    // Check for an initial DSR ON state
    if ( !brokenSerial && GetCommModemStatus( handle, &evt ) ) {
        if ( (evt & MS_DSR_ON) ) {
            dsr = true; // We don't notify about this because the connection isn't actually "up" yet
            LOG() << "DSR up (initially).";
        }
    }

    OVERLAPPED o;
    memset( &o, 0, sizeof(OVERLAPPED) );
    o.hEvent = CreateEvent(NULL, true, false, NULL);
    Q_ASSERT(o.hEvent);

#define HANDLES 3
    HANDLE handles[HANDLES] = { o.hEvent, writeSem, quitSem };

    // Set the comm event mask
    abort();

    for ( ;; ) {

        if ( currentState == Quit ) {
            TRACE(Modem) << "IOThread::run::for" << "currentState" << "Quit";
            break;
        }

        if ( currentState == Idle ) {
            TRACE(Modem) << "IOThread::run::for" << "currentState" << "Idle";
            evt = 0;
            if ( !WaitCommEvent( handle, &evt, &o ) ) {
                if ( GetLastError() != ERROR_IO_PENDING ) {
                    LOG() << "WaitEvent died with an error";
                    currentState = Quit;
                    continue;
                }
            }
            nextState = Wait;
        }

        if ( currentState == Wait ) {
            TRACE(Modem) << "IOThread::run::for" << "currentState" << "Wait";
            LOG() << "WaitForMultipleObjects";
            LONG ret = WaitForMultipleObjects( HANDLES, handles, false, INFINITE );
            if ( ret == WAIT_OBJECT_0 ) { // o.hEvent
                LOG() << "COMM Event" << evt;
                if ( evt == 0 ) {
                    LOG() << "Aborted WaitCommEvent";
                } else if ( evt == EV_RXCHAR ) {
                    LOG() << "RXCHAR";
                    nextState = Read;
                } else if ( evt == EV_DSR ) {
                    LOG() << "DSR";
                    if ( !brokenSerial && GetCommModemStatus( handle, &evt ) ) {
                        if ( (evt & MS_DSR_ON) ) {
                            dsr = true;
                            emit dsrChanged(dsr);
                            LOG() << "DSR up.";
                        } else {
                            dsr = false;
                            emit dsrChanged(dsr);
                            LOG() << "DSR dropped.";
                        }
                    }
                } else {
                    LOG() << "Unknown COMM Event";
                }
            } else if ( ret == WAIT_OBJECT_0+1 ) { // writeSem
                LOG() << "Write Event";
                abort();
                currentState = Wait;
                nextState = Write;
                continue;
            } else if ( ret == WAIT_OBJECT_0+2 ) { // quitSem
                LOG() << "Quit Event";
                abort();
                currentState = Wait;
                nextState = Quit;
                QByteArray data( "Quit\n" );
                WriteFile(handle, data.constData(), data.count(), &dwBytesWritten, 0);
                continue;
            } else if ( ret == WAIT_FAILED ) {
                LOG() << "WAIT_FAILED" << GetLastError();
                nextState = Quit;
            } else {
                LOG() << "ERROR" << ret << GetLastError();
                nextState = Quit;
            }
        }

        if ( currentState == Read ) {
            TRACE(Modem) << "IOThread::run::for" << "currentState" << "Read";
            LOG() << "Calling ReadFile";
            ok = ReadFile(handle, data, maxlen, &dwBytesRead, &o);
            ret = GetLastError();
            if ( !ok && ret != ERROR_IO_PENDING ) {
                LOG() << "ReadFile failed with" << ret;
                nextState = Quit;
            }
            if ( !ok && ret == ERROR_IO_PENDING ) {
                ok = GetOverlappedResult( handle, &o, &dwBytesRead, true );
            }
            if ( ok && !dwBytesRead ) {
                LOG() << "0 bytes read... Go back to waiting";
            }
            if ( ok && dwBytesRead ) {
                QByteArray newData( data, dwBytesRead );
                LOG() << dwBytesRead << "bytes" << newData;
                rbm.lock();
                rb.append( newData );
                rbm.unlock();
                // don't emit unless we've seen a newline because data comes in really slowly
                // and we don't want to waste everyone's time for a few bytes at a time
                // (given that we're using a line-oriented protocol).
                if ( newData.contains("\n") )
                    emit emitReadyRead();
            }
        }

        if ( currentState == Write ) {
            TRACE(Modem) << "IOThread::run::for" << "currentState" << "Write";
            LOG() << "Get data to write";
            wbm.lock();
            QByteArray data = wb;
            wbm.unlock();
            if ( data.count() ) {
                LOG() << "data" << QString::fromLatin1( data.constData(), data.count() ).trimmed();
                LOG() << "Calling WriteFile";
                ok = WriteFile(handle, data.constData(), data.count(), &dwBytesWritten, &o);
                ret = GetLastError();
                if ( !ok && ret != ERROR_IO_PENDING ) {
                    LOG() << "WriteFile failed with" << ret;
                    nextState = Quit;
                }
                if ( !ok && ret == ERROR_IO_PENDING ) {
                    ok = GetOverlappedResult( handle, &o, &dwBytesWritten, true );
                }
                if ( ok && !dwBytesWritten ) {
                    LOG() << "0 bytes written... Try again?";
                    nextState = Write;
                } if ( ok && dwBytesWritten ) {
                    LOG() << dwBytesWritten << "bytes written";
                    wbm.lock();
                    wb = wb.mid( dwBytesWritten ); // remove the bytes we just wrote
                    wbm.unlock();
                    if ( brokenSerial ) {
                        nextState = Read;
                        // give the device a chance to process our message
                        QThread::msleep( 100 );
                    }
                }
            } else {
                LOG() << "No data... going back to waiting.";
            }
        }

        currentState = nextState;
        nextState = Idle;
    }

    CloseHandle( o.hEvent );
    emit ioThreadStopped();
}

void IOThread::abort()
{
    TRACE(Modem) << "IOThread::abort";
    SetCommMask( handle, EV_RXCHAR|EV_DSR );
}

void IOThread::quit()
{
    TRACE(Modem) << "IOThread::quit";
    SetEvent( quitSem );
}

void IOThread::write()
{
    TRACE(Modem) << "IOThread::write";
    SetEvent( writeSem );
}

