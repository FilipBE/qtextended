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

#ifndef LOCALSOCKET_H
#define LOCALSOCKET_H

#define LOCAL_SOCKET_PATH ".device_updater.sock"

#include <QObject>

class LocalSocket : public QObject
{
    Q_OBJECT
public:
    LocalSocket();
    virtual ~LocalSocket();
    virtual void sendRequest( const QString & );
    bool isErrorCondition() const { return mDescriptor == -1; }
protected:
    void setDescriptor(int desc) { mDescriptor = desc; }
    int descriptor() { return mDescriptor; }
private:
    void connect();
    int mDescriptor;
};

#endif
