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

#ifndef QTOPIASQL_P_H
#define QTOPIASQL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QThread>
#include <QMutex>
#include <QHash>
#include <QMap>
#include <QStorageMetaInfo>
#include <QtopiaSql>
#include <QHash>

class QtopiaSqlPrivate : public QObject {
    Q_OBJECT
    public:
        QtopiaSqlPrivate();
        ~QtopiaSqlPrivate() {}

        static QtopiaSqlPrivate *instance();

        QString databaseFile( const QString &path );
#ifndef QTOPIA_HOST
        void connectDiskChannel();
#endif
        void preSetupDefaultConnection();
        void postSetupDefaultConnection();

    private:
        QString type;
        QString name;
        QString user;
        QString password;
        QString hostname;
        QSqlDatabase *defaultConn;
        QHash<QtopiaDatabaseId, QSqlDatabase> masterAttachedConns;
        QHash<QtopiaDatabaseId, QString> dbPaths;
        QHash<QtopiaDatabaseId, QSqlDatabase> dbs;
        QHash<QtopiaDatabaseId, QString> connectionNames;
        QMutex guardMutex;
        quint32 conId;
        QSqlDatabase nullDatabase;
        void installSorting( QSqlDatabase &db);
        friend class QtopiaSql;
    private slots:
        void disksChanged (QtopiaSqlMigratePlugin *plugin = 0);
        void cardMessage(const QString &message,const QByteArray &data);
        void systemMessage(const QString &message,const QByteArray &data);
};

#endif
