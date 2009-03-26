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

#ifndef ACCOUNTCONFIGURATION_H
#define ACCOUNTCONFIGURATION_H

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

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

#include <QMailAccount>
#include <QMailMessage>
#include <QMailMessageKey>


class QMailAccountId;
class QSettings;

class QTOPIAMAIL_EXPORT AccountConfiguration
{
public:
    AccountConfiguration();
    explicit AccountConfiguration(const QMailAccountId& id);
    ~AccountConfiguration();

    /* General */
    enum AuthType {
        Auth_NONE = 0,
#ifndef QT_NO_OPENSSL
        Auth_LOGIN = 1,
        Auth_PLAIN = 2,
#endif
        Auth_INCOMING = 3
    };

    enum EncryptType {
        Encrypt_NONE = 0,
#ifndef QT_NO_OPENSSL
        Encrypt_SSL = 1,
        Encrypt_TLS = 2
#endif
    };

    // Report properties maanged by the QMailAccount class
    QMailAccount::AccountType accountType() const;
    QMailAccountId id() const;
    QString accountName() const;

    bool canCollectMail() const;
    bool canSendMail() const;

    /* SMTP */
    QString userName() const;
    void setUserName(const QString &str);
    QString emailAddress() const;
    void setEmailAddress(const QString &str);
    QString smtpServer() const;
    void setSmtpServer(const QString &str);
    int smtpPort() const;
    void setSmtpPort(int i);
#ifndef QT_NO_OPENSSL
    QString smtpUsername() const;
    void setSmtpUsername(const QString& username);
    QString smtpPassword() const;
    void setSmtpPassword(const QString& password);
#endif
    AuthType smtpAuthentication() const;
#ifndef QT_NO_OPENSSL
    void setSmtpAuthentication(AuthType t);
#endif
    EncryptType smtpEncryption() const;
#ifndef QT_NO_OPENSSL
    void setSmtpEncryption(EncryptType t);
#endif
    bool useSignature() const;
    void setUseSignature(bool b);
    QString signature() const;
    void setSignature(const QString &str);

    /* POP/IMAP */
    QString mailUserName() const;
    void setMailUserName(const QString &str);
    QString mailPassword() const;
    void setMailPassword(const QString &str);
    QString mailServer() const;
    void setMailServer(const QString &str);
    int mailPort() const;
    void setMailPort(int i);
    EncryptType mailEncryption() const;
#ifndef QT_NO_OPENSSL
    void setMailEncryption(EncryptType t);
#endif

    bool canDeleteMail() const;
    void setDeleteMail(bool b);

    int maxMailSize() const;
    void setMaxMailSize(int i);

    int checkInterval() const;
    void setCheckInterval(int i);

    bool intervalCheckRoamingEnabled() const;
    void setIntervalCheckRoamingEnabled(bool enabled);

    bool pushEnabled() const;
    void setPushEnabled(bool enabled);

    /* IMAP Only */
    QString baseFolder() const;
    void setBaseFolder(const QString &s);

    /* MMS Only */
    void setNetworkConfig(const QString &c);
    QString networkConfig() const;
    void setAutoDownload(bool autodl);
    bool isAutoDownload() const;

    /* General management */
    void saveSettings(QSettings *conf) const;
    void readSettings(QSettings *conf);

private:
    friend class QMailStore;

    void setId(const QMailAccountId &id);

    QMailAccountId _id;
    QMailAccount::AccountType _accountType;
    int _mailPort;
    int _smtpPort;
    QString _smtpServer;
    QString _baseFolder;
    QString _accountName;
    QString _userName;
    QString _emailAddress;
    QString _mailUserName;
    QString _mailPassword;
    QString _mailServer;
#ifndef QT_NO_OPENSSL
    EncryptType _mailEncryption;
    QString _smtpUsername;
    QString _smtpPassword;
    AuthType _smtpAuthentication;
    EncryptType _smtpEncryption;
#endif
    bool _synchronize;
    bool _deleteMail;
    bool _useSig;
    QString _sig;
    int _maxMailSize;
    int _checkInterval;
    bool _intervalCheckRoamingEnabled;
    bool _pushEnabled;
    QString _networkCfg;
    bool _autoDL;
};

#endif
