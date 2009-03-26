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

#ifndef ATBLUETOOTHCOMMANDS_H
#define ATBLUETOOTHCOMMANDS_H

#include <QObject>

class AtCommands;
class AtInterface_HandsfreeIpcAdaptor;

class AtBluetoothCommands : public QObject
{
    Q_OBJECT

public:
    AtBluetoothCommands( AtCommands * parent );
    ~AtBluetoothCommands();

    void setSpeakerVolume(int);
    void setMicrophoneVolume(int);

public slots:
    void atbldn();
    void atbrsf( const QString& params );
    void atvgm(const QString &params);
    void atvgs(const QString &params);
    void atnrec(const QString &params);
    void atbvra(const QString &params);
    void atbinp(const QString &params);
    void atbtrh(const QString &params);

private:
    AtCommands *atc;
    AtInterface_HandsfreeIpcAdaptor *m_adaptor;

};

#endif
