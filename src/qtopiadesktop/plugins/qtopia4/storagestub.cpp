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
#include <qdplugin.h>

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QDebug>

class StorageTestStub : public QDServerSyncPlugin
{
    QD_CONSTRUCT_PLUGIN(StorageTestStub,QDServerSyncPlugin);
public:
    QString id() { return "com.trolltech.sync.storagestub"; }
    QString displayName() { return "Storage Test Stub"; }
    void init() { syncIteration = 0; }

    QString dataset() { return "contacts"; }

    QByteArray referenceSchema()
    {
        char *text="<?xml version=\"1.0\" encoding=\"UTF-8\"?><Contact><Identifier/> <FirstName pronunciation=\"\"/> <LastName pronunciation=\"\"/> <Addresses/> <PhoneNumbers default=\"\"/> <EmailAddresses maxItems=\"3\" default=\"\"/> <Categories/> </Contact>";
        QByteArray bytes(text);
        return bytes;
    }

    void fetchChangesSince(const QDateTime &);
    void createClientRecord(const QByteArray &);
    void replaceClientRecord(const QByteArray &);
    void removeClientRecord(const QString &);

    void beginTransaction(const QDateTime &) {}
    void abortTransaction() {}
    void commitTransaction() {}

private:
    int syncIteration;
};

QD_REGISTER_PLUGIN(StorageTestStub)

void StorageTestStub::createClientRecord(const QByteArray & /*record*/)
{
    // do nothing
    //qDebug() << "create record" << QString::fromUtf8(record);
}

void StorageTestStub::replaceClientRecord(const QByteArray & /*record*/)
{
    // do nothing
    //qDebug() << "update record" << QString::fromUtf8(record);
}

void StorageTestStub::removeClientRecord(const QString & /*identifier*/)
{
    // do nothing
    //qDebug() << "remove record" << identifier;
}

void StorageTestStub::fetchChangesSince(const QDateTime &timestamp)
{
    int item;
    if (timestamp.isNull()) {
        syncIteration = 0;
    }

    static const QString createMask(":/storagestub/%1/create/%2");
    static const QString replaceMask(":/storagestub/%1/replace/%2");
    static const QString removeMask(":/storagestub/%1/remove/%2");

    // creates..
    item = 0;
    while (QFile::exists(createMask.arg(syncIteration).arg(item))) {
        QFile f(createMask.arg(syncIteration).arg(item++));
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray bytes = f.readAll();
            //qDebug() << "Server create:" << QString::fromUtf8(bytes);
            emit createServerRecord(bytes);
            f.close();
        }
    }
    // replaces..
    item = 0;
    while (QFile::exists(replaceMask.arg(syncIteration).arg(item))) {
        QFile f(replaceMask.arg(syncIteration).arg(item++));
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray bytes = f.readAll();
            //qDebug() << "Server replace:" << QString::fromUtf8(bytes);
            emit replaceServerRecord(bytes);
            f.close();
        }
    }
    // removes..
    item = 0;
    while (QFile::exists(removeMask.arg(syncIteration).arg(item))) {
        QFile f(removeMask.arg(syncIteration).arg(item++));
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray bytes = f.readAll();
            //qDebug() << "Server remove:" << QString::fromUtf8(bytes).trimmed();
            emit removeServerRecord(QString::fromUtf8(bytes).trimmed());
            f.close();
        }
    }

    syncIteration = (syncIteration+1)%3;

    emit serverChangesCompleted();
}

#include "storagestub.moc"
