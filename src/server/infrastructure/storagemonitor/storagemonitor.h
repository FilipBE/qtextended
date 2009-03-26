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

#ifndef STORAGEMONITOR_H
#define STORAGEMONITOR_H

#include <QObject>
#include <QTime>
class QFileSystem;
class QStorageMetaInfo;
class QtopiaTimer;
class OutOfSpace;
class QtopiaChannel;

class StorageMonitor : public QObject
{
    Q_OBJECT
public:
    StorageMonitor(QObject *o = 0);
    ~StorageMonitor();

public slots:
    void checkAvailStorage();
    void systemMsg(const QString &msg, const QByteArray &data);

private slots:
    void showCleanupWizard();

    void availableDisksChanged();

private:
    void outOfSpace(QString& text);
    void fileSystemMetrics(const QFileSystem* fs, long &available, long &total);

    void setEnabled(bool enabled);

    QStorageMetaInfo *sinfo;
    OutOfSpace *box;
    QtopiaTimer *storageTimer;

    int minimalStorageLimit;
    int minAbsolute;
    int maxAbsolute;
    int pollFrequency;
    QTime metaInfoUpdate;

    QtopiaChannel *channel;
};

#endif
