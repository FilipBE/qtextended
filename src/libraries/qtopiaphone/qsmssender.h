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

#ifndef QSMSSENDER_H
#define QSMSSENDER_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <qsmsmessage.h>

class QTOPIAPHONE_EXPORT QSMSSender : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSMSSender( const QString& service = QString(),
                         QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QSMSSender();

    QString send( const QSMSMessage& msg );

public slots:
    virtual void send( const QString& id, const QSMSMessage& msg );

signals:
    void finished( const QString& id, QTelephony::Result result );
};

#endif
