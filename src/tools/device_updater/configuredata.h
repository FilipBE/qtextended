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

#ifndef CONFIGUREDATA_H
#define CONFIGUREDATA_H

#include <QString>

class QSettings;

class ConfigureData
{
public:
    ConfigureData();
    ConfigureData( const ConfigureData & );
    ~ConfigureData();
    QString server() const  { return mServer; }
    quint16 port() const    { return mPort; }
    QString command() const { return mCommand; }
    QString docRoot() const { return mDocRoot; }
    void setServer( QString s )  { mDirty = true; mServer = s; }
    void setPort( quint16 p )    { mDirty = true; mPort = p; }
    void setCommand( QString c ) { mDirty = true; mCommand = c; }
    void setDocRoot( QString d ) { mDirty = true; mDocRoot = d; }
    ConfigureData &operator=( const ConfigureData &rhs );
    void load();
    void save() const;
private:
    QString mServer;
    quint16 mPort;
    QString mCommand;
    QString mDocRoot;
    mutable bool mDirty;
};

#endif
