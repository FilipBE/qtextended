/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QDBUSBINDING_H
#define QDBUSBINDING_H

#include <QtDBus/QtDBus>
#include <QtScript/qscriptable.h>
#include <QtScript/qscriptengine.h>

class QDBusConnectionConstructor : public QObject,
                                   public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue sessionBus READ sessionBus)
    Q_PROPERTY(QScriptValue systemBus READ systemBus)

public:
    QDBusConnectionConstructor(QScriptEngine *engine, QScriptValue extensionObject);

    QScriptValue sessionBus() const;
    QScriptValue systemBus() const;

public Q_SLOTS:
    QObject *qscript_call(const QString &name);

    void disconnectFromBus(const QString &name);
    QDBusConnection connectToBus(const QString &address, const QString &name);
    QDBusConnection connectToBus(QDBusConnection::BusType type, const QString &name);
};

class QScriptDBusConnection : public QObject,
                              public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QString baseService READ baseService)
    Q_PROPERTY(bool isConnected READ isConnected)
    Q_PROPERTY(QScriptValue dbusInterface READ dbusInterface)
public:
    QScriptDBusConnection(const QDBusConnection &conn, QObject *parent);

    inline QString baseService() const { return connection.baseService(); }
    inline bool isConnected() const { return connection.isConnected(); }
    QScriptValue dbusInterface() const;

    inline QDBusConnection dbusConnection() const { return connection; }

public Q_SLOTS:
    inline bool send(const QDBusMessage &message) const
    { return connection.send(message); }
    inline QDBusMessage call(const QDBusMessage &message, int callMode = QDBus::Block, int timeout = -1) const
    { return connection.call(message, QDBus::CallMode(callMode), timeout); }

    inline bool registerService(const QString &serviceName)
    { return connection.registerService(serviceName); }
    inline bool unregisterService(const QString &serviceName)
    { return connection.unregisterService(serviceName); }

    inline QDBusError lastError() const
    { return connection.lastError(); }

    inline void unregisterObject(const QString &path, QDBusConnection::UnregisterMode mode = QDBusConnection::UnregisterNode)
    { return connection.unregisterObject(path, mode); }
    inline QObject *objectRegisteredAt(const QString &path) const
    { return connection.objectRegisteredAt(path); }

#if 0
    bool callWithCallback(const QDBusMessage &message, QObject *receiver,
                          const char *slot, int timeout = -1) const;

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, QObject *receiver, const char *slot);

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, const QString& signature,
                 QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, const QString& signature,
                    QObject *receiver, const char *slot);

    bool registerObject(const QString &path, QObject *object,
                        RegisterOptions options = ExportAdaptors);

#endif

private:
    QDBusConnection connection;
};

Q_DECLARE_METATYPE(QScriptDBusConnection*)

class QScriptDBusInterfaceConstructor : public QObject,
                                        public QScriptable
{
    Q_OBJECT
public:
    QScriptDBusInterfaceConstructor(QScriptEngine *engine, QScriptValue extensionObject);

public Q_SLOTS:
    QScriptValue qscript_call(const QString &service, const QString &path, const QString &interface = QString(),
                              const QScriptValue &conn = QScriptValue());
};

Q_DECLARE_METATYPE(QDBusMessage)

class QScriptDBusMessageConstructor : public QObject, public QScriptable
{
    Q_OBJECT
    Q_ENUMS(MessageType)
public:
    enum MessageType {
        InvalidMessage = QDBusMessage::InvalidMessage,
        MethodCallMessage = QDBusMessage::MethodCallMessage,
        ReplyMessage = QDBusMessage::ReplyMessage,
        ErrorMessage = QDBusMessage::ErrorMessage,
        SignalMessage = QDBusMessage::SignalMessage
    };

    QScriptDBusMessageConstructor(QScriptEngine *engine, QScriptValue extensionObject);

    QScriptValue protoType() const { return proto; }

public Q_SLOTS:
    QDBusMessage createSignal(const QString &path, const QString &interface, const QString &name);
    QDBusMessage createMethodCall(const QString &destination, const QString &path, const QString &interface, const QString &method);
    QDBusMessage createError(const QString &name, const QString &msg);

public:
    static QScriptValue createReply(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue createErrorReply(QScriptContext *context, QScriptEngine *engine);

private:
    QScriptValue proto;
};

#endif // QDBUSBINDING_H
