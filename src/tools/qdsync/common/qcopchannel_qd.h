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
#ifndef QCOPCHANNEL_QD_H
#define QCOPCHANNEL_QD_H

#include <qdglobal.h>

#include <QObject>
#include <QString>
#include <QByteArray>

namespace qdsync {

class QD_EXPORT QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel( const QString &channel, QObject *parent = 0 );
    ~QCopChannel();

    QString channel() const;

    static bool isRegistered( const QString &channel );
    static void send( const QString &ch, const QString &msg, const QByteArray &data = QByteArray() );

signals:
    void received(const QString &msg, const QByteArray &data);

private:
    void init(const QString &channel);
    void receive(const QString &msg, const QByteArray &data);

    QString mChannel;
};

};

using namespace qdsync;

#endif
