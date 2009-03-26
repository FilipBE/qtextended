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

#ifndef QNETWORKCOOKIE_H
#define QNETWORKCOOKIE_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

class QByteArray;
class QDateTime;
class QString;
class QUrl;

class QNetworkCookiePrivate;
class Q_NETWORK_EXPORT QNetworkCookie
{
public:
    enum RawForm {
        NameAndValueOnly,
        Full
    };

    QNetworkCookie(const QByteArray &name = QByteArray(), const QByteArray &value = QByteArray());
    QNetworkCookie(const QNetworkCookie &other);
    ~QNetworkCookie();
    QNetworkCookie &operator=(const QNetworkCookie &other);
    bool operator==(const QNetworkCookie &other) const;
    inline bool operator!=(const QNetworkCookie &other) const
    { return !(*this == other); }

    bool isSecure() const;
    void setSecure(bool enable);

    bool isSessionCookie() const;
    QDateTime expirationDate() const;
    void setExpirationDate(const QDateTime &date);

    QString domain() const;
    void setDomain(const QString &domain);

    QString path() const;
    void setPath(const QString &path);

    QByteArray name() const;
    void setName(const QByteArray &cookieName);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QByteArray toRawForm(RawForm form = Full) const;

    static QList<QNetworkCookie> parseCookies(const QByteArray &cookieString);

private:
    QSharedDataPointer<QNetworkCookiePrivate> d;
    friend class QNetworkCookiePrivate;
};
Q_DECLARE_TYPEINFO(QNetworkCookie, Q_MOVABLE_TYPE);

class QNetworkCookieJarPrivate;
class Q_NETWORK_EXPORT QNetworkCookieJar: public QObject
{
    Q_OBJECT
public:
    QNetworkCookieJar(QObject *parent = 0);
    virtual ~QNetworkCookieJar();

    virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

protected:
    QList<QNetworkCookie> allCookies() const;
    void setAllCookies(const QList<QNetworkCookie> &cookieList);

private:
    Q_DECLARE_PRIVATE(QNetworkCookieJar)
    Q_DISABLE_COPY(QNetworkCookieJar)
};

#ifndef QT_NO_DEBUG_STREAM
class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QNetworkCookie &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNetworkCookie)
Q_DECLARE_METATYPE(QList<QNetworkCookie>)

QT_END_HEADER

#endif
