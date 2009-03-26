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

#ifndef SERVERTASKPLUGIN_H
#define SERVERTASKPLUGIN_H

#include <qtopiaglobal.h>
#include <QByteArray>
#include <QObject>

struct QTOPIA_EXPORT ServerTaskFactoryIface
{
    virtual ~ServerTaskFactoryIface();
    virtual QByteArray name() const = 0;
    virtual QObject* initTask( void* createArg = 0 ) const = 0;
    virtual bool demand() const = 0;
};


#define ServerTaskFactoryIface_iid "com.trolltech.Qtopia.ServerTaskFactoryIface"
Q_DECLARE_INTERFACE(ServerTaskFactoryIface, ServerTaskFactoryIface_iid)

class QTOPIA_EXPORT ServerTaskPlugin : public QObject, public ServerTaskFactoryIface
{
    Q_OBJECT
    Q_INTERFACES(ServerTaskFactoryIface)
public:
    explicit ServerTaskPlugin(QObject *parent = 0);
    virtual ~ServerTaskPlugin();
};

#endif
