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

#ifndef QGOOGLECONTEXT_P_H
#define QGOOGLECONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QList>
#include <QDateTime>
#include <QUrl>
#include <QHttp>
#include <QSslSocket>

#ifndef QT_NO_OPENSSL

#include <qappointment.h>
#include <qtopiasql.h>

#include "qappointmentsqlio_p.h"

class QXmlSimpleReader;
class QXmlInputSource;
class GoogleCalHandler;
class QtopiaHttp;

class QGoogleCalendarFetcher;
// not a public class, but needs to be used by datebook.
// later need to make a plugin or some other optional context.
class QTOPIAPIM_EXPORT QGoogleCalendarContext : public QAppointmentContext
{
    Q_OBJECT
public:
    QGoogleCalendarContext(QObject *parent, QObject *access);

    QIcon icon() const;
    QString description() const;
    QString title() const;
    QString title(const QPimSource &) const;

    using QAppointmentContext::editable;
    bool editable() const;

    // do available and set, same as outer model?
    // do as contexts later.
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    QPimSource defaultSource() const;

    using QAppointmentContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateAppointment(const QAppointment &);
    bool removeAppointment(const QUniqueId &);
    QUniqueId addAppointment(const QAppointment &, const QPimSource &);

    bool removeOccurrence(const QUniqueId &original, const QDate &);
    bool restoreOccurrence(const QUniqueId &original, const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original, const QOccurrence &, const QDate& = QDate());
    QUniqueId replaceRemaining(const QUniqueId &original, const QAppointment &, const QDate& = QDate());

    // Google functions
    //void addUser(const QString &user);
    //QStringList users() const;

    QString addAccount(const QString &user, const QString &password);
    void removeAccount(const QString &);

    QStringList accounts() const;

    enum FeedType {
        FullPrivate,
        FullPublic,
        FreeBusyPublic
    };

    QString password(const QString &account) const;
    FeedType feedType(const QString &account) const;
    QString name(const QString &account) const;
    QString email(const QString &account) const;
    QString accountHolder(const QString &account) const; // From the server

    void setEmail(const QString &account, const QString &email);
    void setPassword(const QString &account, const QString &password);
    void setFeedType(const QString &account, FeedType);
    void setName(const QString& account, const QString& name);

    void syncAccount(const QString &account);
    void syncAllAccounts();

    bool syncing() const;
    void syncProgress(int &amount, int &total) const;

    // need error reporting....
    // enum + string

    enum Status {
        NotStarted,
        InProgress,
        Completed,
        BadAuthentication,
        NotVerified,
        TermsNotAgreed,
        CaptchaRequired,
        AccountDeleted,
        AccountDisabled,
        ServiceUnavailable,
        CertificateError,
        ParseError,
        DataAccessError,
        UnknownError
    };

    static QString statusMessage(int status);
signals:
    void syncProgressChanged(int, int);
    void syncStatusChanged(const QString &account, int);
    void finishedSyncing();

private slots:
    void updateFetchingProgress(int, int);
    void updateFetchingState(QGoogleCalendarContext::Status);
    void syncAccountList();
private:
    QList<QGoogleCalendarFetcher *> mFetchers;
    void saveAccount(const QString &);

    struct Account {
        Account() : type(FullPrivate) {}
        FeedType type;
        QString password;
        QString name;
        QString email;
        QString accountHolder;
    };

    QMap<QString, Account> mAccounts;

    QAppointmentSqlIO *mAccess;
};

/*
   Split off as separate class to enable multiple concurrent downloads.

   the database itself already handles locking db for the adds, this gives us a measure of
    protection in that it will be more difficult to confuse multiple http downloads
    of different accounts if they are done with different QHttp objects

    Later, turn into QThread so can also avoid halting while adding items to DB
*/
class QGoogleCalendarFetcher : public QObject // : public QThread
{
    Q_OBJECT
public:
    QGoogleCalendarFetcher(const QPimSource & sqlContext, const QString& account, const QString &email, const QString &password, const QString &url, QObject *parent = 0);

    QGoogleCalendarContext::Status status() const;
    QString statusMessage() const;

    int lastValue() const;
    int lastMaximum() const;

    // async call.
    void fetch();

    QString account() const;
    QString email() const;
    QString name() const;

    void fetchProgress(int &, int &) const;
signals:
    void fetchProgressChanged(int, int);
    void completed(QGoogleCalendarContext::Status);

    void certificateError(const QMap<QString, QString> &, const QString &);

private slots:
    void parseRemaining();
    void abortParsing();
    void parsePartial(const QHttpResponseHeader &);
    void httpFetchProgress(int, int);

    //void sendCertificateError(QtSslSocket::VerifyResult result, bool hostNameWrong, const QString &str);
private:
    void fetchAuthentication();
    void fetchAppointments();

    enum State
    {
        IdleState,
        AuthenticationState,
        AppointmentDownloadState,
        ErrorState,
        CompletedState
    };

    /* shared */
    int lastProgress;
    int lastTotal;
    State mState;
    QGoogleCalendarContext::Status mStatus;
    QUrl mUrl;
    QPimSource mContext;
    QString mAccount;
    QString mEmail;
    QString mPassword;
    QString mAuth;

    QDateTime syncTime;
    QDateTime lastSyncTime;

    /* worker side */
    QXmlSimpleReader *xmlReader;
    QXmlInputSource *mSource;
    GoogleCalHandler *mHandler;

    QtopiaHttp *mDownloader;
    QSslSocket *mSslSocket;

    QAppointmentSqlIO *mAccess;
};

#endif //QT_NO_OPENSSL
#endif
