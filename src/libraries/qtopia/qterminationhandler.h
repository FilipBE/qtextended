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

#ifndef QTERMINATIONHANDLER_H
#define QTERMINATIONHANDLER_H

#include <QObject>
#include <QtopiaServiceRequest>
#include <QString>

struct QTerminationHandlerPrivate;
struct QTerminationHandlerData;
class QTOPIA_EXPORT QTerminationHandler : public QObject
{
public:
    explicit QTerminationHandler(const QString &text,
                                 const QString &buttonText = QString(),
                                 const QString &icon = QString(),
                                 const QtopiaServiceRequest &action = QtopiaServiceRequest(),
                                 QObject *parent = 0);
    explicit QTerminationHandler(const QtopiaServiceRequest &action,
                                 QObject *parent = 0);
    ~QTerminationHandler();

private:
    void installHandler(const QTerminationHandlerData& data);
    static QTerminationHandlerPrivate* staticData();
};

#endif
