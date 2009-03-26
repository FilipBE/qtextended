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

#ifndef MEDIACONTENTSERVER_H
#define MEDIACONTENTSERVER_H

#include <QMediaAbstractControlServer>

class QMediaServerSession;

namespace mediaserver
{

class MediaContentServerPrivate;

class MediaContentServer : public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    MediaContentServer(QMediaServerSession* session,
                       QMediaHandle const& handle);


    ~MediaContentServer();

signals:
    void controlAvailable(QString const& name);
    void controlUnavailable(QString const& name);

private slots:
    void interfaceAvailable(const QString& name);
    void interfaceUnavailable(const QString& name);

private:
    MediaContentServerPrivate*  d;
};

}   // ns mediaserver

#endif
