/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
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

#ifndef QDBUSMESSAGE_H
#define QDBUSMESSAGE_H

#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbuserror.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(DBus)

class QDBusMessagePrivate;
class QDBUS_EXPORT QDBusMessage
{
public:
    enum MessageType {
        InvalidMessage,
        MethodCallMessage,
        ReplyMessage,
        ErrorMessage,
        SignalMessage
    };

    QDBusMessage();
    QDBusMessage(const QDBusMessage &other);
    QDBusMessage &operator=(const QDBusMessage &other);
    ~QDBusMessage();

    static QDBusMessage createSignal(const QString &path, const QString &interface,
                                     const QString &name);
    static QDBusMessage createMethodCall(const QString &destination, const QString &path,
                                         const QString &interface, const QString &method);
    static QDBusMessage createError(const QString &name, const QString &msg);
    static inline QDBusMessage createError(const QDBusError &err)
    { return createError(err.name(), err.message()); }
    static inline QDBusMessage createError(QDBusError::ErrorType type, const QString &msg)
    { return createError(QDBusError::errorString(type), msg); }

    QDBusMessage createReply(const QList<QVariant> &arguments = QList<QVariant>()) const;
    inline QDBusMessage createReply(const QVariant &argument) const
    { return createReply(QList<QVariant>() << argument); }

    QDBusMessage createErrorReply(const QString name, const QString &msg) const;
    inline QDBusMessage createErrorReply(const QDBusError &err) const
    { return createErrorReply(err.name(), err.message()); }
    inline QDBusMessage createErrorReply(QDBusError::ErrorType type, const QString &msg) const;

    QString service() const;
    QString path() const;
    QString interface() const;
    QString member() const;
    QString errorName() const;
    QString errorMessage() const;
    MessageType type() const;
    QString signature() const;

    bool isReplyRequired() const;

    void setDelayedReply(bool enable) const;
    bool isDelayedReply() const;

    void setArguments(const QList<QVariant> &arguments);
    QList<QVariant> arguments() const;

    QDBusMessage &operator<<(const QVariant &arg);

private:
    friend class QDBusMessagePrivate;
    QDBusMessagePrivate *d_ptr;
};

inline QDBusMessage QDBusMessage::createErrorReply(QDBusError::ErrorType atype, const QString &amsg) const
{ return createErrorReply(QDBusError::errorString(atype), amsg); }

#ifndef QT_NO_DEBUG_STREAM
QDBUS_EXPORT QDebug operator<<(QDebug, const QDBusMessage &);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif

