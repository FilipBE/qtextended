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

#ifndef CELLBROADCASTCONTROL_H
#define CELLBROADCASTCONTROL_H

#include <QObject>
#include <qtelephonynamespace.h>
#include "qtopiaserverapplication.h"

class QString;
class QCellBroadcast;
class QCBSMessage;

class CellBroadcastControl : public QObject
{
Q_OBJECT
public:
    CellBroadcastControl(QObject *parent = 0);

    enum Type { Popup, Background };

signals:
    void broadcast(CellBroadcastControl::Type, const QString &channel, const QString &text);

private slots:
    void cellBroadcast(const QCBSMessage &);
    void cellBroadcastResult(QTelephony::Result);
    void registrationChanged(QTelephony::RegistrationState);
    void hideCBSMessage();

private:
    void subscribe();
    void showPopup( const QString &channel, const QString &text );

    QCellBroadcast *cb;
    bool firstSubscribe;
    int infoMsgId;
    QString cellLocation;
};

QTOPIA_TASK_INTERFACE(CellBroadcastControl)

#endif
