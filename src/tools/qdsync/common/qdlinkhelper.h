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
#ifndef QDLINKHELPER_H
#define QDLINKHELPER_H

#include <qdglobal.h>
#include <QObject>
#include <QPointer>

class QIODevice;
class PingThread;

class QD_EXPORT QDLinkHelper : public QObject
{
    Q_OBJECT
public:
    QDLinkHelper( QIODevice *device, QObject *parent = 0 );
    ~QDLinkHelper();

    QIODevice *socket();
    QIODevice *rawSocket();

signals:
    void timeout();

private:
    void init( QIODevice *device );

    QPointer<QIODevice> rawDevice;
    QPointer<QIODevice> wrapperDevice;
    PingThread *thread;
};

#endif
