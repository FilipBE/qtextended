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

#ifndef HARDWAREMANIPULATOR_H
#define HARDWAREMANIPULATOR_H

#include <QObject>

#include "qsmsmessagelist.h"

class QSMSMessage;
class HardwareManipulator : public QObject
{
Q_OBJECT

public:
    HardwareManipulator(QObject *parent=0);
    QSMSMessageList & getSMSList();

public slots:
    virtual void handleFromData( const QString& );
    virtual void handleToData( const QString& );
    virtual void setPhoneNumber( const QString& );
    virtual void constructSMSMessage(const int type, const QString &sender, const QString &serviceCenter, const QString &text);
    virtual void sendSMS( const QSMSMessage& m );

signals:
    void unsolicitedCommand(const QString &cmd);
    void command(const QString &cmd);
    void variableChanged(const QString &n, const QString &v);
    void switchTo(const QString &cmd);
    void startIncomingCall(const QString &number);

protected:
    virtual QString constructCBMessage(const QString &messageCode, int geographicalScope, const QString &updateNumber, const QString &channel,
    const QString &scheme, int language, const QString &numPages, const QString &page, const QString &content);
    virtual void constructSMSDatagram(int port, const QString &sender,  const QByteArray &data, const QByteArray &contentType);

    virtual void warning(const QString &title, const QString &message);

    virtual int convertString(const QString &number, int maxValue, int numChar, int base, bool *ok);

private:
    QSMSMessageList SMSList;
};

class HardwareManipulatorFactory
{
public:
    virtual ~HardwareManipulatorFactory() {};
    inline virtual HardwareManipulator *create(QObject *p) { Q_UNUSED(p); return 0; }

    QString ruleFile() const { return ruleFilename; }
    void setRuleFile(const QString& filename) { ruleFilename = filename; }

private:
    QString ruleFilename;
};


#endif
