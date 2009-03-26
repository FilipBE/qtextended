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

#include "qannotator_p.h"
#include <qtopianamespace.h>

QAnnotator::QAnnotator()
    : mIdGen("5c58d519-fd18-4063-847b-578df0be333e")
{
}

QAnnotator::~QAnnotator()
{
}

QUniqueId QAnnotator::add(const QByteArray &data, const QString &mimetype)
{
    QUniqueId id = mIdGen.createUniqueId();
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (!file.exists() && file.open(QIODevice::WriteOnly)) {
        QDataStream ds(&file);
        ds << mimetype;
        ds << data;
        file.close();
        return id;
    }
    return QUniqueId();
}

bool QAnnotator::set(const QUniqueId &id, const QByteArray &data, const QString &mimetype)
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream ds(&file);
        ds << mimetype;
        ds << data;
        file.close();
        return true;
    }
    return false;
}

void QAnnotator::remove(const QUniqueId &id)
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists())
        file.remove();
}

bool QAnnotator::contains(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    return file.exists();
}

QString QAnnotator::mimetype(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream ds(&file);
        QString mimetype;
        //QByteArray data;
        ds >> mimetype;
        //ds >> data;
        file.close();
        return mimetype;
    }
    return QString();
}

QByteArray QAnnotator::blob(const QUniqueId &id) const
{
    QString path = Qtopia::applicationFileName("Annotator", id.toLocalContextString());
    QFile file(path);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream ds(&file);
        QString mimetype;
        QByteArray data;
        ds >> mimetype;
        ds >> data;
        file.close();
        return data;
    }
    return QByteArray();
}
