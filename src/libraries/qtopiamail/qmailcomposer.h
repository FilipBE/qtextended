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

#ifndef QMAILCOMPOSER_H
#define QMAILCOMPOSER_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QIconSet>
#include <qtopiaglobal.h>
#include <qmailmessage.h>

class QContent;
class QMenu;
class QMailAccount;

class QTOPIAMAIL_EXPORT QMailComposerInterface : public QWidget
{
    Q_OBJECT

public:
    enum ComposeContext { Create = 0, Reply = 1, ReplyToAll = 2, Forward = 3 };

public:
    QMailComposerInterface( QWidget *parent = 0 );
    virtual ~QMailComposerInterface();

    //these non virtual functions query data from the explict composer plugin descriptors
    QString key() const;
    QList<QMailMessage::MessageType> messageTypes() const;
    QList<QMailMessage::ContentType> contentTypes() const;
    QString name(QMailMessage::MessageType type) const;
    QString displayName(QMailMessage::MessageType type) const;
    QIcon displayIcon(QMailMessage::MessageType type) const;

    virtual bool isEmpty() const = 0;
    virtual bool isReadyToSend() const = 0;

    virtual QMailMessage message() const = 0;

    virtual void setTo(const QString& toAddress) = 0;
    virtual QString to() const = 0;

    virtual void setFrom(const QString& fromAddress) = 0;
    virtual QString from() const = 0;

    virtual void setSubject(const QString& subject) = 0;

    virtual void setMessageType(QMailMessage::MessageType type);

    virtual bool isDetailsOnlyMode() const = 0;
    virtual void setDetailsOnlyMode(bool val) = 0;

    virtual QString contextTitle() const = 0;

    virtual QMailAccount fromAccount() const = 0;

    virtual void setDefaultAccount(const QMailAccountId& id) = 0;

public slots:
    virtual void setMessage( const QMailMessage& mail ) = 0;

    virtual void clear() = 0;

    virtual void setBody( const QString &text, const QString &type );

    virtual void attach( const QContent &lnk, QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments );

    virtual void setSignature( const QString &sig );

    virtual void reply(const QMailMessage& source, int type) = 0;

signals:
    void sendMessage();
    void cancel();
    void changed();
    void contextChanged();
};

class QTOPIAMAIL_EXPORT QMailComposerFactory
{
public:
    // Yield the key for each interface supporting the supplied type
    static QStringList keys(QMailMessage::MessageType type = QMailMessage::AnyType,
                            QMailMessage::ContentType contentType = QMailMessage::NoContent);

    // Yield the default key for the supplied type
    static QString defaultKey( QMailMessage::MessageType type = QMailMessage::AnyType );

    // Properties available for each interface
    static QList<QMailMessage::MessageType> messageTypes(const QString &key);
    //static QList<QMailMessage::ContentType> contentTypes(const QString& key);
    static QString name(const QString &key, QMailMessage::MessageType type);
    static QString displayName(const QString &key, QMailMessage::MessageType type);
    static QIcon displayIcon(const QString &key, QMailMessage::MessageType type);

    // Use the interface identified by the supplied key to create a composer
    static QMailComposerInterface *create( const QString& key, QWidget *parent = 0 );
};

#endif
