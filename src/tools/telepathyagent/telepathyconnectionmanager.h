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

#ifndef TELEPATHYCONNECTIONMANAGER_H
#define TELEPATHYCONNECTIONMANAGER_H

#include <QMap>
#include <QObject>
#include <QVariant>

class QVariant;
class QString;
class QDBusObjectPath;
class TelepathyConnectionManagerPrivate;
class TelepathyConnection;

class TelepathyConnectionManager : public QObject
{
    Q_OBJECT

public:

    enum ParamFlag {
        Required = 1,
        Register = 2,
        HasDefault = 4
    };

    Q_DECLARE_FLAGS(ParamFlags, ParamFlag)

    struct ParamSpec {
        QString name;
        ParamFlags flags;
        QString signature;
        QVariant defaultValue;
    };

    explicit TelepathyConnectionManager(const QString &cmname, QObject *parent = 0);
    ~TelepathyConnectionManager();

    static QStringList availableManagers();

    bool isValid() const;
    QStringList supportedProtocols() const;
    TelepathyConnection *requestConnection(const QString &protocol,
                                           const QMap<QString,QVariant> &parameters);
    QString name() const;
    QList<ParamSpec> parametersForProtocol(const QString &proto) const;

signals:
    void newConnection(const QString &busName, QDBusObjectPath &path, const QString &protocol);

private:
    Q_DISABLE_COPY(TelepathyConnectionManager)
    TelepathyConnectionManagerPrivate *m_data;
};

#endif
