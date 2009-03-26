/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QNETWORKPROXY_H
#define QNETWORKPROXY_H

#include <QtNetwork/qhostaddress.h>

#ifndef QT_NO_NETWORKPROXY

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

class QNetworkProxyPrivate;

class Q_NETWORK_EXPORT QNetworkProxy
{
    Q_DECLARE_PRIVATE(QNetworkProxy)

public:
    enum ProxyType {
        DefaultProxy,
        Socks5Proxy,
        NoProxy,
        HttpProxy,
        HttpCachingProxy,
        FtpCachingProxy
    };

    QNetworkProxy();
    QNetworkProxy(ProxyType type, const QString &hostName = QString(), quint16 port = 0,
                  const QString &user = QString(), const QString &password = QString());
    QNetworkProxy(const QNetworkProxy &other);
    QNetworkProxy &operator=(const QNetworkProxy &other);
    ~QNetworkProxy();
    bool operator==(const QNetworkProxy &other) const;
    inline bool operator!=(const QNetworkProxy &other) const
    { return !(*this == other); }

    void setType(QNetworkProxy::ProxyType type);
    QNetworkProxy::ProxyType type() const;
    bool isCachingProxy() const;
    bool isTransparentProxy() const;

    void setUser(const QString &userName);
    QString user() const;

    void setPassword(const QString &password);
    QString password() const;

    void setHostName(const QString &hostName);
    QString hostName() const;

    void setPort(quint16 port);
    quint16 port() const;

    static void setApplicationProxy(const QNetworkProxy &proxy);
    static QNetworkProxy applicationProxy();

private:
    QNetworkProxyPrivate *d_ptr;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_NETWORKPROXY

#endif // QHOSTINFO_H
