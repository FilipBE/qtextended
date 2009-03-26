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

#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QtopiaAbstractService>

class QDate;
class QDSActionRequest;
class QMailMessageId;
class QString;
class QStringList;

class EmailService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    EmailService(QObject* parent);
    ~EmailService();

signals:
    void viewInbox();
    void message(const QMailMessageId&);
    void write(const QString&, const QString&);
    void write(const QString&, const QString&, const QStringList&, const QStringList&);
    void vcard(const QString& filename, const QString& description);
    void vcard(const QDSActionRequest& request);
    void cleanup(const QDate&, int);

public slots:
    void writeMail();
    void writeMail( const QString& name, const QString& email );
    void writeMessage( const QString& name, const QString& email,
                       const QStringList& docAttachments,
                       const QStringList& fileAttachments );
    void viewMail();
    void viewMail( const QMailMessageId& id );
    void emailVCard( const QString& filename, const QString& description );
    void emailVCard( const QString& channel,
                     const QMailMessageId& id,
                     const QString& filename,
                     const QString& description );
    void cleanupMessages( const QDate& date, int size );

    // QDS services
    void emailVCard( const QDSActionRequest& request );
};

#endif
