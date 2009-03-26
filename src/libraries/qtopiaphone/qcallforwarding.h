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

#ifndef QCALLFORWARDING_H
#define QCALLFORWARDING_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallForwarding : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(Reason)
public:
    explicit QCallForwarding( const QString& service = QString::null,
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QCallForwarding();

    enum Reason
    {
        Unconditional,
        MobileBusy,
        NoReply,
        NotReachable,
        All,
        AllConditional
    };

    struct Status
    {
        QTelephony::CallClass cls;
        QString number;
        int time;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

public slots:
    virtual void requestForwardingStatus( QCallForwarding::Reason reason );
    virtual void setForwarding( QCallForwarding::Reason reason,
                                const QCallForwarding::Status& status,
                                bool enable );

signals:
    void forwardingStatus( QCallForwarding::Reason reason,
                           const QList<QCallForwarding::Status>& status );
    void setForwardingResult
            ( QCallForwarding::Reason reason, QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallForwarding::Reason)
Q_DECLARE_USER_METATYPE(QCallForwarding::Status)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QCallForwarding::Status>)

#endif
