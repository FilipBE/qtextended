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

#ifndef QMAILSERVICEACTION_H
#define QMAILSERVICEACTION_H

#include "qprivateimplementation.h"

#include <qtopiaglobal.h>

#include <QMailAccountId>
#include <QMailFolderId>
#include <QMailMessageId>
#include <QMailMessageKey>
#include <QString>
#include <QStringList>


class QMailServiceActionPrivate;

class QTOPIAMAIL_EXPORT QMailServiceAction 
    : public QObject, 
      public QPrivatelyNoncopyable<QMailServiceActionPrivate>
{
    Q_OBJECT

    friend class QMailServiceActionPrivate;
    friend class QMailRetrievalActionPrivate;
    friend class QMailTransmitActionPrivate;
    friend class QMailSearchActionPrivate;

public:
    typedef QMailServiceActionPrivate ImplementationType;

    enum Connectivity {
        Offline = 0,
        Connecting,
        Connected,
        Disconnected
    };

    enum Activity {
        Pending = 0,
        InProgress,
        Successful,
        Failed
    };

    class Status 
    {
    public: 
        Status(const Status &other);

        enum ErrorCode {
            ErrNoError = 0,
            ErrUnknownResponse,
            ErrLoginFailed,
            ErrCancel,
            ErrFileSystemFull,
            ErrNonexistentMessage,
            ErrEnqueueFailed,
            ErrNoConnection,
            ErrConnectionInUse,
            ErrConnectionNotReady,
            ErrConfiguration,
            ErrInvalidAddress
        };

        ErrorCode errorCode;
        QString text;

        QMailAccountId accountId;
        QMailFolderId folderId;
        QMailMessageId messageId;

    private:
        friend class QMailServiceActionPrivate;

        Status(ErrorCode code, 
               const QString &text,
               const QMailAccountId &accountId, 
               const QMailFolderId &folderId,
               const QMailMessageId &messageId);
    };

    ~QMailServiceAction();

    Connectivity connectivity() const;
    Activity activity() const;
    const Status status() const;
    QPair<uint, uint> progress() const;

public slots:
    virtual void cancelOperation();

signals:
    void connectivityChanged(QMailServiceAction::Connectivity c);
    void activityChanged(QMailServiceAction::Activity a);
    void statusChanged(const QMailServiceAction::Status &s);
    void progressChanged(uint value, uint total);

protected:
    // Only allow creation by sub-types
    template<typename Subclass>
    QMailServiceAction(Subclass *p, QObject *parent);

protected:
    void setStatus(Status::ErrorCode code, const QString &text = QString());
    void setStatus(Status::ErrorCode code, const QString &text, const QMailAccountId &accountId, const QMailFolderId &folderId = QMailFolderId(), const QMailMessageId &messageId = QMailMessageId());
};


class QMailRetrievalActionPrivate;

class QTOPIAMAIL_EXPORT QMailRetrievalAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailRetrievalActionPrivate ImplementationType;

    /*
    enum RetrievalSpecification {
        Unspecified = 0,
        MetaData,
        Content
    };
    */

    QMailRetrievalAction(QObject *parent = 0);
    ~QMailRetrievalAction();

public slots:
    /* Expected command set:
    void retrieveFolderList(const QMailFolderId &folderId);
    void retrieveMessageList(const QMailFolderId &folderId);

    void retrieveMessages(const QMailMessageIdList &messageIds, RetrievalSpecification spec = Unspecified);
    void retrieveMessageElement(const QMailMessageId &messageId, const QString &elementIdentifier);

    void retrieveAll(const QMailAccountId &accountId, RetrievalSpecification spec = Unspecified);
    void exportUpdates(const QMailAccountId &accountId);

    void synchronize(const QMailAccountId &accountId, RetrievalSpecification spec = Unspecified);
    */
    /* Existing command set: */
    void retrieve(const QMailAccountId &accountId);
    void completeRetrieval(const QMailMessageIdList &ids);

    void retrieveFolders(const QMailAccountId &accountId);
};


class QMailTransmitActionPrivate;

class QTOPIAMAIL_EXPORT QMailTransmitAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailTransmitActionPrivate ImplementationType;

    QMailTransmitAction(QObject *parent = 0);
    ~QMailTransmitAction();

public slots:
    /* Expected command set:
    void transmitMessages(const QMailAccountId &accountId);
    */
    /* Existing command set: */
    void send(const QMailMessageIdList &);
};


class QMailSearchActionPrivate;

class QTOPIAMAIL_EXPORT QMailSearchAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailSearchActionPrivate ImplementationType;

    QMailSearchAction(QObject *parent = 0);
    ~QMailSearchAction();

    QMailMessageIdList matchingMessageIds() const;

public slots:
    void searchMessages(const QMailMessageKey &filter, const QString& bodyText);
    void cancelOperation();
};


Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Connectivity)
Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Activity)
Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Status::ErrorCode)

#endif
