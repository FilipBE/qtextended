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

#ifndef QWAPACCOUNT_H
#define QWAPACCOUNT_H

#include <qtopiaglobal.h>

#include <QUrl>
#include <QString>

class QWapAccountPrivate;

class QTOPIACOMM_EXPORT QWapAccount
{
public:
    enum MMSVisibility {
        Default,
        SenderHidden,
        SenderVisible
    };

    QWapAccount();
    QWapAccount( const QString& wapConfig );
    QWapAccount( const QWapAccount& copy );

    virtual ~QWapAccount();

    QWapAccount& operator=(const QWapAccount& copy);
    bool operator==(const QWapAccount& other );

    QString configuration() const;

    QString name() const;
    void setName( const QString& name );

    QString dataInterface() const;
    void setDataInterface( const QString& ifaceHandle );

    QUrl gateway() const;
    void setGateway( const QUrl& );

    QUrl mmsServer() const;
    void setMmsServer( const QUrl& url );

    int mmsExpiry() const;
    void setMmsExpiry( int mmsExpiry );

    QWapAccount::MMSVisibility mmsSenderVisibility() const;
    void setMmsSenderVisibility( QWapAccount::MMSVisibility vis );

    bool mmsDeliveryReport() const;
    void setMmsDeliveryReport( bool allowReport );

private:
    QWapAccountPrivate* d;
};

#endif
