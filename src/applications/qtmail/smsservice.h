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

#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <QtopiaAbstractService>

#ifndef QTOPIA_NO_SMS

class QDSActionRequest;
class QString;
class QStringList;

class SMSService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    SMSService(QObject* parent);
    ~SMSService();

signals:
    void newMessages(bool);
    void viewInbox();
    void write(const QString& name, const QString& number, const QString& filename);
    void vcard(const QString& filename, const QString& description);
    void vcard(const QDSActionRequest& request);

public slots:
    void writeSms();
    void writeSms( const QString& name, const QString& number );
    void writeSms( const QString& name, const QString& number,
                   const QString& filename );
    void viewSms();
    void viewSmsList();
    void smsVCard( const QString& filename, const QString& description );
    void smsVCard( const QString& channel,
                   const QString& id,
                   const QString& filename,
                   const QString& description );

    // QDS services
    void smsVCard( const QDSActionRequest& request );
};

#endif

#endif
