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

#ifndef INSTANTMESSAGESERVICE_H
#define INSTANTMESSAGESERVICE_H

#include <QtopiaAbstractService>

class QString;

class InstantMessageService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    InstantMessageService(QObject* parent);
    ~InstantMessageService();

signals:
    void write(const QString &);

public slots:
    void writeMessage(const QString &uri);
};

#endif
