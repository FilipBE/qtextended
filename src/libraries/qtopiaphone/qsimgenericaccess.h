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

#ifndef QSIMGENERICACCESS_H
#define QSIMGENERICACCESS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QSimGenericAccess : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimGenericAccess
            ( const QString& service = QString(), QObject *parent = 0,
              QCommInterface::Mode mode = Client );
    ~QSimGenericAccess();

    QString command( const QByteArray& data );

public slots:
    virtual void command( const QString& reqid, const QByteArray& data );

signals:
    void response( const QString& reqid, QTelephony::Result result,
                   const QByteArray& data );
};

#endif
