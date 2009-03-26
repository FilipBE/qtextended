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

#include "configuredata.h"

#include <QSettings>

#define ORG_STRING "com.trolltech.qtopia"
#define APP_STRING "device_updater"

/*!
  \class ConfigureData
  \brief Auto-saving Data Transfer Object (DTO) for configuration date.

  This simple class serves as a container to allow passing the configuration data
  around between classes.

  Use the load() and save() methods to retrieve or store the configuration to a
  settings file, to persist between runs.
*/

ConfigureData::ConfigureData()
    : mDirty( false )
{
}

ConfigureData::ConfigureData( const ConfigureData &rhs )
{
    this->mServer = rhs.mServer;
    this->mPort = rhs.mPort;
    this->mCommand = rhs.mCommand;
    this->mDocRoot = rhs.mDocRoot;
    this->mDirty = rhs.mDirty;
}

ConfigureData::~ConfigureData()
{
}

void ConfigureData::load()
{
    QSettings conf( ORG_STRING, APP_STRING );
    mServer = conf.value( "listen/address", "10.10.10.21" ).toString();
    mPort = (quint64)( conf.value( "listen/port", QVariant( 8080 )).toInt() );
    mCommand = conf.value( "package/command", "mkPackages" ).toString();
    mDocRoot = conf.value( "package/documentRoot", "." ).toString();
    mDirty = false;
}

void ConfigureData::save() const
{
    if ( mDirty )
    {
        QSettings conf( ORG_STRING, APP_STRING );
        conf.setValue( "listen/address", mServer );
        conf.setValue( "listen/port", mPort );
        conf.setValue( "package/command", mCommand );
        conf.setValue( "package/documentRoot", mDocRoot );
        mDirty = false;
    }
}

ConfigureData &ConfigureData::operator=( const ConfigureData &rhs )
{
    this->mServer = rhs.mServer;
    this->mPort = rhs.mPort;
    this->mCommand = rhs.mCommand;
    this->mDocRoot = rhs.mDocRoot;
    return *this;
}
