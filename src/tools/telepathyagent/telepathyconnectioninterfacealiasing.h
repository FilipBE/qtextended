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

#ifndef TELEPATHYCONNECTIONINTERFACEALIASING_H
#define TELEPATHYCONNECTIONINTERFACEALIASING_H

#include <QObject>

class TelepathyConnection;
class TelepathyConnectionInterfaceAliasingPrivate;
class QStringList;
class QDBusMessage;
class QDBusError;
template <class T> class QList;
template <class T, class U> class QMap;

class TelepathyConnectionInterfaceAliasing : public QObject
{
    Q_OBJECT

public:
    TelepathyConnectionInterfaceAliasing(TelepathyConnection *conn);
    ~TelepathyConnectionInterfaceAliasing();

    enum AliasFlag {
        UserSet = 1
    };

    Q_DECLARE_FLAGS(AliasFlags, AliasFlag)

    AliasFlags aliasFlags() const;
    bool requestAliases(const QList<uint> &contacts);
    bool setAliases(const QMap<uint, QString> &aliases);

signals:
    void aliasesChanged(const QMap<uint, QString> &aliases);
    void aliasesRetrieved(const QMap<uint, QString> &aliases);

private slots:
    void aliasesChanged(const QDBusMessage &msg);

private:
    Q_DISABLE_COPY(TelepathyConnectionInterfaceAliasing)
    TelepathyConnectionInterfaceAliasingPrivate *m_data;
    friend class AliasingMessageHelper;
};

#endif
