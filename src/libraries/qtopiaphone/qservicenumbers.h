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

#ifndef QSERVICENUMBERS_H
#define QSERVICENUMBERS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QServiceNumbers : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(NumberId)
public:
    explicit QServiceNumbers( const QString& service = QString(),
                              QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QServiceNumbers();

    enum NumberId
    {
        VoiceMail,
        SmsServiceCenter,
        SubscriberNumber
    };

public slots:
    virtual void requestServiceNumber( QServiceNumbers::NumberId id );
    virtual void setServiceNumber
        ( QServiceNumbers::NumberId id, const QString& number );

signals:
    void serviceNumber( QServiceNumbers::NumberId id, const QString& number );
    void setServiceNumberResult
            ( QServiceNumbers::NumberId id, QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QServiceNumbers::NumberId)

#endif
