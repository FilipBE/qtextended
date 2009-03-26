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

#ifndef QPHONERFFUNCTIONALITY_H
#define QPHONERFFUNCTIONALITY_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QPhoneRfFunctionality : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(Level)
    Q_PROPERTY(QPhoneRfFunctionality::Level level READ level WRITE setLevel)
public:
    explicit QPhoneRfFunctionality( const QString& service = QString(),
                                    QObject *parent = 0,
                                    QCommInterface::Mode mode = Client );
    ~QPhoneRfFunctionality();

    enum Level
    {
        Minimum,
        Full,
        DisableTransmit,
        DisableReceive,
        DisableTransmitAndReceive
    };

    QPhoneRfFunctionality::Level level() const;

public slots:
    virtual void forceLevelRequest();
    virtual void setLevel( QPhoneRfFunctionality::Level level );

signals:
    void levelChanged();
    void setLevelResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QPhoneRfFunctionality::Level)

#endif
