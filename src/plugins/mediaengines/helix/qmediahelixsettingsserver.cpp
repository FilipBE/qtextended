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

#include <qstringlist.h>

#include "qmediahelixsettingsserver.h"
#include "helixutil.h"

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxccf.h>
#include <hxprefs.h>
#include <ihxpckts.h>

#include <qdebug.h>
#include <reporterror.h>

namespace qtopia_helix
{

class QMediaHelixSettingsServerPrivate
{
public:
    QStringList     options;
    IHXClientEngine *engine;

    QVariant value( QString const& name ) const;
    void setValue( QString const& name, QVariant const& vlaue );
};

QVariant QMediaHelixSettingsServerPrivate::value( QString const& name ) const
{
    QString s;

    IHXPreferences *preferences = 0;
    IHXBuffer *buffer = 0;

    if (queryInterface(engine, IID_IHXPreferences, preferences) == HXR_OK) {
        if( preferences->ReadPref(name.toLatin1().constData(), buffer ) == HXR_OK ) {
            s = (const char*)buffer->GetBuffer();
        }
    } else {
        REPORT_ERROR( ERR_UNSUPPORTED );
    }

    HX_RELEASE( buffer );
    HX_RELEASE( preferences );

    return s;
}

void QMediaHelixSettingsServerPrivate::setValue( QString const& name, QVariant const& value )
{
    IHXPreferences *preferences = 0;
    IHXCommonClassFactory *factory = 0;
    IHXBuffer *buffer = 0;

    if (queryInterface(engine, IID_IHXPreferences, preferences) == HXR_OK &&
        queryInterface(engine, IID_IHXCommonClassFactory, factory) == HXR_OK ) {
        createInstance(factory, CLSID_IHXBuffer, buffer);
        QString s = value.toString();
        buffer->Set( (const UCHAR*)s.toLatin1().data(), s.length() );

        preferences->WritePref( name.toLatin1().constData(), buffer );
    } else {
        REPORT_ERROR( ERR_UNSUPPORTED );
    }

    HX_RELEASE( buffer );
    HX_RELEASE( factory );
    HX_RELEASE( preferences );
}

/*!
    \class qtopia_helix::QMediaHelixSettingsServer
    \internal
*/


QMediaHelixSettingsServer::QMediaHelixSettingsServer(IHXClientEngine *engine):
    QAbstractIpcInterface("/Media/Control/Helix/",
                          "Settings",
                          "HelixGlobalSettings",
                          NULL,
                          QAbstractIpcInterface::Server),
    d(new QMediaHelixSettingsServerPrivate)
{
    d->options << QLatin1String("Bandwidth") << QLatin1String("ServerTimeOut") <<
                  QLatin1String("ConnectionTimeOut");

    d->engine = engine;

    setValue(QLatin1String("AvailableSettings"), d->options);

    setOption(QLatin1String("Bandwidth"), d->value(QLatin1String("Bandwidth")));
    setOption(QLatin1String("ServerTimeOut"), d->value(QLatin1String("ServerTimeOut")));
    setOption(QLatin1String("ConnectionTimeOut"), d->value(QLatin1String("ConnectionTimeOut")));

    proxyAll(*metaObject());
}

QMediaHelixSettingsServer::~QMediaHelixSettingsServer()
{
    delete d;
}


//public slots:
void QMediaHelixSettingsServer::setOption(QString const& name, QVariant const& value)
{
    if (d->options.contains(name))
    {
        d->setValue(name, value);       // to helix
        setValue(name, value);          // for clients

        emit optionChanged(name, value);
    }
}

}   // ns qtopia_helix

