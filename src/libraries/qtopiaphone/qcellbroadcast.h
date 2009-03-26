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

#ifndef QCELLBROADCAST_H
#define QCELLBROADCAST_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <qcbsmessage.h>

class QTOPIAPHONE_EXPORT QCellBroadcast : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(QList<int> channels READ channels WRITE setChannels)
public:
    explicit QCellBroadcast( const QString& service = QString(),
                             QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QCellBroadcast();

    QList<int> channels() const;

public slots:
    virtual void setChannels( const QList<int>& list );

signals:
    void setChannelsResult( QTelephony::Result result );
    void broadcast( const QCBSMessage& message );
};

Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<int>)

#endif
