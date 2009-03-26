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

#ifndef QCOPROUTER_H
#define QCOPROUTER_H

#if !defined(QT_NO_COP)

#include <QMultiMap>
#include "applicationlauncher.h"
#include <QSet>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif

class QCopRouter : public ApplicationIpcRouter
{
    Q_OBJECT
public:
    QCopRouter();
    ~QCopRouter();

    virtual void addRoute(const QString &app, RouteDestination *);
    virtual void remRoute(const QString &app, RouteDestination *);

private slots:
    void applicationMessage( const QString& msg, const QByteArray& data );
    void serviceMessage( const QString& msg, const QByteArray& data );

private:
    void routeMessage(const QString &, const QString &, const QByteArray &);
    QMultiMap<QString, RouteDestination *> m_routes;

    // In-progress route
    QString m_cDest;
    QString m_cMessage;
    QByteArray m_cData;
    QSet<RouteDestination *> m_cRouted;
};

#endif

#endif
