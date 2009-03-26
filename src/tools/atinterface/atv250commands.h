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

#ifndef ATV250COMMANDS_H
#define ATV250COMMANDS_H

#include <QObject>

class AtCommands;

class AtV250Commands : public QObject
{
    Q_OBJECT

public:
    AtV250Commands( AtCommands * parent );
    ~AtV250Commands();

public slots:
    void ata();
    void ate( const QString& params );
    void atgcap();
    void atgmi( const QString& params );
    void atgmm( const QString& params );
    void atgmr( const QString& params );
    void atgsn( const QString& params );
    void ath( const QString& params );
    void ati( const QString& params );
    void ato();
    void atq( const QString& params );
    void ats3( const QString& params );
    void ats4( const QString& params );
    void ats5( const QString& params );
    void atv( const QString& params );
    void atz();
    void atampf();
    void atampw();

private:
    int soption( const QString& params, int prev, int min, int max );

    AtCommands *atc;

};

#endif
