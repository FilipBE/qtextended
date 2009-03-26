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

#include <qtopiaglobal.h>
#include <qtopia4sync.h>

class MySyncPlugin : public Qtopia4SyncPlugin
{
    Q_OBJECT
public:
    MySyncPlugin( QObject *parent = 0 )
        : Qtopia4SyncPlugin( parent )
    {
    }

    ~MySyncPlugin()
    {
    }

    QString dataset() { return "mydataset"; }

    void fetchChangesSince( const QDateTime & /*timestamp*/ )
    {
        // emit createClientRecord(QByteArray)
        // emit replaceClientRecord(QByteArray)
        // emit deleteClientRecord(QString)

        // emit clientChangesCompleted()
    }

    void beginTransaction( const QDateTime & /*timestamp*/ )
    {
        // set a point for roll back
    }

    void createServerRecord( const QByteArray & /*record*/ )
    {
        // store record
        // emit mappedId(serverId,localId)
    }

    void replaceServerRecord( const QByteArray & /*record*/ )
    {
        // store record
    }

    void removeServerRecord( const QString & /*localId*/ )
    {
        // remove record
    }

    void abortTransaction()
    {
        // revert any create/replace/delete events since beginTransaction()
    }

    void commitTransaction()
    {
        // commit any create/replace/delete events since beginTransaction()
    }
};

QTOPIA_EXPORT_PLUGIN(MySyncPlugin)

#include "main.moc"
