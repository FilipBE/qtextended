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
#ifndef QSERIALIODEVICE_P_H
#define QSERIALIODEVICE_P_H

#include <qobject.h>
#include <qprocess.h>

namespace qdsync {

class QSerialIODevice;

class QPseudoTtyProcess : public QProcess
{
    Q_OBJECT
public:
    QPseudoTtyProcess( QSerialIODevice *device, int masterFd,
                       int slaveFd, bool isPPP );
    ~QPseudoTtyProcess();

    void clearDevice() { device = 0; }
    void deviceReadyRead();

protected:
    void setupChildProcess();

private slots:
    void masterReadyRead();
    void childStateChanged( QProcess::ProcessState state );
    void deviceReady();

private:
    QSerialIODevice *device;
    int masterFd;
    int slaveFd;
    bool isPPP;
    bool readySeen;
    char buffer[1024];
};

};

using namespace qdsync;

#endif
