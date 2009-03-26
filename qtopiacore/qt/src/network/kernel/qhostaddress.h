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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include <QtCore/qstring.h>
#include <QtNetwork/qabstractsocket.h>

struct sockaddr;

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

class QHostAddressPrivate;

class Q_NETWORK_EXPORT QIPv6Address
{
public:
    inline quint8 &operator [](int index) { return c[index]; }
    inline quint8 operator [](int index) const { return c[index]; }
    quint8 c[16];
};

typedef QIPv6Address Q_IPV6ADDR;

class Q_NETWORK_EXPORT QHostAddress
{
public:
    enum SpecialAddress {
        Null,
        Broadcast,
        LocalHost,
        LocalHostIPv6,
        Any,
        AnyIPv6
    };

    QHostAddress();
    explicit QHostAddress(quint32 ip4Addr);
    explicit QHostAddress(quint8 *ip6Addr);
    explicit QHostAddress(const Q_IPV6ADDR &ip6Addr);
    explicit QHostAddress(const sockaddr *sockaddr);
    explicit QHostAddress(const QString &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &other);
    QHostAddress &operator=(const QString &address);

    void setAddress(quint32 ip4Addr);
    void setAddress(quint8 *ip6Addr);
    void setAddress(const Q_IPV6ADDR &ip6Addr);
    void setAddress(const sockaddr *sockaddr);
    bool setAddress(const QString &address);

    QAbstractSocket::NetworkLayerProtocol protocol() const;
    quint32 toIPv4Address() const;
    Q_IPV6ADDR toIPv6Address() const;

    QString toString() const;

    QString scopeId() const;
    void setScopeId(const QString &id);

    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    inline bool operator !=(const QHostAddress &address) const
    { return !operator==(address); }
    inline bool operator !=(SpecialAddress address) const
    { return !operator==(address); }
    bool isNull() const;
    void clear();

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT quint32 ip4Addr() const { return toIPv4Address(); }
    inline QT3_SUPPORT bool isIPv4Address() const { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIp4Addr() const  { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIPv6Address() const { return protocol() == QAbstractSocket::IPv6Protocol; }
#endif

private:
    QHostAddressPrivate *d;
};

inline bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2)
{ return address2 == address1; }

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QHostAddress &);
#endif


Q_NETWORK_EXPORT uint qHash(const QHostAddress &key);

#ifndef QT_NO_DATASTREAM
Q_NETWORK_EXPORT QDataStream &operator<<(QDataStream &, const QHostAddress &);
Q_NETWORK_EXPORT QDataStream &operator>>(QDataStream &, QHostAddress &);
#endif

QT_END_NAMESPACE

QT_END_HEADER
#endif // QHOSTADDRESS_H
