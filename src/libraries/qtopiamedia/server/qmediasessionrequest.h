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

#ifndef QMEDIASESSIONREQUEST_H
#define QMEDIaSESSIONREQUEST_H

#include <QObject>
#include <QUrl>
#include <QVariant>

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>

class QMediaSessionRequestPrivate;

class QTOPIAMEDIA_EXPORT QMediaSessionRequest
{
public:
    QMediaSessionRequest();
    QMediaSessionRequest(QString const& domain, QString const& type);
    QMediaSessionRequest(QMediaSessionRequest const& copy);
    ~QMediaSessionRequest();

    QUuid const& id() const;
    QString const& domain() const;
    QString const& type() const;

    QMediaSessionRequest& operator=(QMediaSessionRequest const& rhs);

    template <typename DataType>
    QMediaSessionRequest& operator<<(DataType const& data)
    {
        enqueueRequestData(qVariantFromValue(data));
        return *this;
    }

    template <typename DataType>
    QMediaSessionRequest& operator>>(DataType& data)
    {
        data = qVariantValue<DataType>(dequeueRequestData());
        return *this;
    }

    // {{{ Serialization
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
    // }}}

private:
    void enqueueRequestData(QVariant const& data);
    QVariant dequeueRequestData();

    QMediaSessionRequestPrivate*  d;
};

Q_DECLARE_USER_METATYPE(QMediaSessionRequest);

#endif
