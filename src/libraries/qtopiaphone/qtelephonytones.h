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

#ifndef QTELEPHONYTONES_H
#define QTELEPHONYTONES_H

#include <qcomminterface.h>
#include <qtopiaipcmarshal.h>

class QTOPIAPHONE_EXPORT QTelephonyTones : public QCommInterface
{
    Q_OBJECT
public:
    explicit QTelephonyTones( const QString& service = QString(),
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QTelephonyTones();

    enum Tone
    {
	Dtmf0,
	Dtmf1,
	Dtmf2,
	Dtmf3,
	Dtmf4,
	Dtmf5,
	Dtmf6,
	Dtmf7,
	Dtmf8,
	Dtmf9,
	DtmfStar,
	DtmfHash,
	DtmfA,
	DtmfB,
	DtmfC,
	DtmfD,
	Busy,
        Dial,
        Dial2,
        Alerting,
        CallWaiting,
        MessageWaiting,
        NoService,
        User = 100
    };

    QString startTone( QTelephonyTones::Tone tone, int duration = -1 );

public slots:
    virtual void startTone
	( const QString& id, QTelephonyTones::Tone tone, int duration = -1 );
    virtual void stopTone( const QString& id );

signals:
    void toneStopped( const QString& id );
};

Q_DECLARE_USER_METATYPE_ENUM(QTelephonyTones::Tone)

#endif
