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

#ifndef MESSAGECONTROL_H
#define MESSAGECONTROL_H

#include <QObject>
#include <qvaluespace.h>
#include <qtopiaipcenvelope.h>
#include <QtopiaIpcAdaptor>
class QString;

class QCommServiceManager;

class MessageControl : public QObject
{
Q_OBJECT
public:
    MessageControl( QObject *parent = 0);
    int messageCount() const;

#ifdef QTOPIA_CELL
signals:
    void smsMemoryFull(bool);
    void messageRejected();
#endif

private slots:
#ifdef QTOPIA_CELL
    void smsMemoryFullChanged();
#endif
    void controlMessage(const QString& message, const QByteArray&);
    void messageCountChanged();

private:
    void doNewCount(bool write = true);
    void updateMessageCount(int &messageCount, QDataStream &stream);

    QValueSpaceObject phoneValueSpace;
#ifdef QTOPIA_CELL
    QValueSpaceItem smsMemFull;
    int prevSmsMemoryFull;
#endif
    QtopiaIpcAdaptor *messageCountUpdate;
    QtopiaChannel channel;

    int smsCount;
    int mmsCount;
    int systemCount;
    int instantCount;
    int emailCount;
};

#endif
