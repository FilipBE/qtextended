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

//#define QHOSTINFO_DEBUG

static const int RESOLVER_TIMEOUT = 2000;

#include "qplatformdefs.h"

#include "qhostinfo_p.h"
#include "qiodevice.h"
#include <qbytearray.h>
#include <qlibrary.h>
#include <private/qmutexpool_p.h>

extern "C" {
#include <netdb.h>
#include <arpa/inet.h>
}

#if defined (QT_NO_GETADDRINFO)
#include <qmutex.h>
QT_BEGIN_NAMESPACE
Q_GLOBAL_STATIC(QMutex, getHostByNameMutex)
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

// Almost always the same. If not, specify in qplatformdefs.h.
#if !defined(QT_SOCKOPTLEN_T)
# define QT_SOCKOPTLEN_T QT_SOCKLEN_T
#endif

// HP-UXi has a bug in getaddrinfo(3) that makes it thread-unsafe
// with this flag. So disable it in that platform.
#if defined(AI_ADDRCONFIG) && !defined(Q_OS_HPUX)
#  define Q_ADDRCONFIG          AI_ADDRCONFIG
#endif

typedef int (*res_init_proto)(void);
static res_init_proto local_res_init = 0;

static void resolveLibrary()
{
#ifndef QT_NO_LIBRARY
    QLibrary lib(QLatin1String("resolv"));
    if (!lib.load())
        return;
    local_res_init = res_init_proto(lib.resolve("__res_init"));
    if (!local_res_init)
        local_res_init = res_init_proto(lib.resolve("res_init"));
#endif
}

QHostInfo QHostInfoAgent::fromName(const QString &hostName)
{
    QHostInfo results;
    results.setHostName(hostName);

#if defined(QHOSTINFO_DEBUG)
    qDebug("QHostInfoAgent::fromName(%s) looking up...",
           hostName.toLatin1().constData());
#endif

    // Load res_init on demand.
    static volatile bool triedResolve = false;
    if (!triedResolve) {
#ifndef QT_NO_THREAD
        QMutexLocker locker(QMutexPool::globalInstanceGet(&local_res_init));
#endif
        if (!triedResolve) {
            resolveLibrary();
            triedResolve = true;
        }
    }

    // If res_init is available, poll it.
    if (local_res_init)
        local_res_init();

    QHostAddress address;
    if (address.setAddress(hostName)) {
        // Reverse lookup
// Reverse lookups using getnameinfo are broken on darwin, use gethostbyaddr instead.
#if !defined (QT_NO_GETADDRINFO) && !defined (Q_OS_DARWIN)
        sockaddr_in sa4;
#ifndef QT_NO_IPV6
        sockaddr_in6 sa6;
#endif
        sockaddr *sa = 0;
        QT_SOCKLEN_T saSize = 0;
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            sa = (sockaddr *)&sa4;
            saSize = sizeof(sa4);
            memset(&sa4, 0, sizeof(sa4));
            sa4.sin_family = AF_INET;
            sa4.sin_addr.s_addr = htonl(address.toIPv4Address());
        }
#ifndef QT_NO_IPV6
        else {
            sa = (sockaddr *)&sa6;
            saSize = sizeof(sa6);
            memset(&sa6, 0, sizeof(sa6));
            sa6.sin6_family = AF_INET6;
            memcpy(sa6.sin6_addr.s6_addr, address.toIPv6Address().c, sizeof(sa6.sin6_addr.s6_addr));
        }
#endif

        char hbuf[NI_MAXHOST];
        if (!sa || getnameinfo(sa, saSize, hbuf, sizeof(hbuf), 0, 0, 0) != 0) {
            results.setError(QHostInfo::HostNotFound);
            results.setErrorString(tr("Host not found"));
            return results;
        }
        results.setHostName(QString::fromLatin1(hbuf));
#else
        in_addr_t inetaddr = inet_addr(hostName.toLatin1().constData());
        struct hostent *ent = gethostbyaddr((const char *)&inetaddr, sizeof(inetaddr), AF_INET);
        if (!ent) {
            results.setError(QHostInfo::HostNotFound);
            results.setErrorString(tr("Host not found"));
            return results;
        }
        results.setHostName(QString::fromLatin1(ent->h_name));
#endif
    }

#if !defined (QT_NO_GETADDRINFO)
    // Call getaddrinfo, and place all IPv4 addresses at the start and
    // the IPv6 addresses at the end of the address list in results.
    addrinfo *res = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
#ifdef Q_ADDRCONFIG
    hints.ai_flags = Q_ADDRCONFIG;
#endif

    int result = getaddrinfo(hostName.toLatin1().constData(), 0, &hints, &res);
# ifdef Q_ADDRCONFIG
    if (result == EAI_BADFLAGS) {
        // if the lookup failed with AI_ADDRCONFIG set, try again without it
        hints.ai_flags = 0;
        result = getaddrinfo(hostName.toLatin1().constData(), 0, &hints, &res);
    }
# endif

    if (result == 0) {
        addrinfo *node = res;
        QList<QHostAddress> addresses;
        while (node) {
            if (node->ai_family == AF_INET) {
                QHostAddress addr;
                addr.setAddress(ntohl(((sockaddr_in *) node->ai_addr)->sin_addr.s_addr));
                if (!addresses.contains(addr))
                    addresses.append(addr);
            }
#ifndef QT_NO_IPV6
            else if (node->ai_family == AF_INET6) {
                QHostAddress addr;
                addr.setAddress(((sockaddr_in6 *) node->ai_addr)->sin6_addr.s6_addr);
                if (!addresses.contains(addr))
                    addresses.append(addr);
            }
#endif
            node = node->ai_next;
        }
        if (addresses.isEmpty() && node == 0) {
            // Reached the end of the list, but no addresses were found; this
            // means the list contains one or more unknown address types.
            results.setError(QHostInfo::UnknownError);
            results.setErrorString(tr("Unknown address type"));
        }

        results.setAddresses(addresses);
        freeaddrinfo(res);
    } else if (result == EAI_NONAME
               || result ==  EAI_FAIL
#ifdef EAI_NODATA
	       // EAI_NODATA is deprecated in RFC 3493
	       || result == EAI_NODATA
#endif
	       ) {
        results.setError(QHostInfo::HostNotFound);
        results.setErrorString(tr("Host not found"));
    } else {
        results.setError(QHostInfo::UnknownError);
        results.setErrorString(QString::fromLocal8Bit(gai_strerror(result)));
    }

#else
    // Fall back to gethostbyname for platforms that don't define
    // getaddrinfo. gethostbyname does not support IPv6, and it's not
    // reentrant on all platforms. For now this is okay since we only
    // use one QHostInfoAgent, but if more agents are introduced, locking
    // must be provided.
    QMutexLocker locker(::getHostByNameMutex());
    hostent *result = gethostbyname(hostName.toLatin1().constData());
    if (result) {
        if (result->h_addrtype == AF_INET) {
            QList<QHostAddress> addresses;
            for (char **p = result->h_addr_list; *p != 0; p++) {
                QHostAddress addr;
                addr.setAddress(ntohl(*((quint32 *)*p)));
                if (!addresses.contains(addr))
                    addresses.prepend(addr);
            }
            results.setAddresses(addresses);
        } else {
            results.setError(QHostInfo::UnknownError);
            results.setErrorString(tr("Unknown address type"));
        }
    } else if (h_errno == HOST_NOT_FOUND || h_errno == NO_DATA
               || h_errno == NO_ADDRESS) {
        results.setError(QHostInfo::HostNotFound);
        results.setErrorString(tr("Host not found"));
    } else {
        results.setError(QHostInfo::UnknownError);
        results.setErrorString(tr("Unknown error"));
    }
#endif //  !defined (QT_NO_GETADDRINFO)

#if defined(QHOSTINFO_DEBUG)
    if (results.error() != QHostInfo::NoError) {
        qDebug("QHostInfoAgent::fromName(): error #%d %s",
               h_errno, results.errorString().toLatin1().constData());
    } else {
        QString tmp;
        QList<QHostAddress> addresses = results.addresses();
        for (int i = 0; i < addresses.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += addresses.at(i).toString();
        }
        qDebug("QHostInfoAgent::fromName(): found %i entries for \"%s\": {%s}",
               addresses.count(), hostName.toLatin1().constData(),
               tmp.toLatin1().constData());
    }
#endif
    return results;
}

QString QHostInfo::localHostName()
{
    char hostName[512];
    if (gethostname(hostName, sizeof(hostName)) == -1)
        return QString();
    hostName[sizeof(hostName) - 1] = '\0';
    return QString::fromLocal8Bit(hostName);
}

QT_END_NAMESPACE
