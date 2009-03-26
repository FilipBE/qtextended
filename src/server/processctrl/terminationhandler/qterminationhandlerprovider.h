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

#ifndef QTERMINATIONHANDLERPROVIDER_H
#define QTERMINATIONHANDLERPROVIDER_H

#include <QObject>

class QString;
class QPixmap;
class QtopiaServiceRequest;

class QTerminationHandlerProvider : public QObject
{
    Q_OBJECT
public:
    QTerminationHandlerProvider(QObject *parent = 0);
    virtual ~QTerminationHandlerProvider();

signals:
    void applicationTerminated(const QString &name, const QString &text,
                               const QPixmap &icon, const QString &buttonText,
                               QtopiaServiceRequest &buttonAction);
};

#endif
