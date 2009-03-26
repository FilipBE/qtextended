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

#include "accountconfiguration_p.h"
#include "qmailstore_p.h"

#include <QApplication>
#include <QDir>
#include <QMailBase64Codec>
#include <QMailFolder>
#include <QSettings>
#include <qtopianamespace.h>


/*!
    \enum AccountConfiguration::AuthType

    This enum type is used to describe the type of authentication used to communicate with a server.

    \value Auth_NONE       No authentication is used for this account.
    \value Auth_LOGIN      A two stage simple cleartext password mechanism.
    \value Auth_PLAIN      A one stage simple cleartext password mechanism. PLAIN supercedes the LOGIN mechanism.
    \value Auth_INCOMING   Indicates mail check must be done before sending SMTP mail. Only applies to accounts with both incoming and outgoing details defined.
*/

/*!
    \enum AccountConfiguration::EncryptType

    This enum type is used to describe the type of encryption used to
    communicate with a server. Transport Layer Security (TLS) and its
    predecessor Secure Sockets Layer (SSL) enable encrypted communication
    with an e-mail server.

    \value Encrypt_NONE    No encryption is used.
    \value Encrypt_SSL     SSL (Secure Sockets Layer) encryption is used.
    \value Encrypt_TLS     TLS (Transport Layer Security) encryption is used.
*/

/*!
    \class AccountConfiguration
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \ingroup messaginglibrary

    \brief The AccountConfiguration class stores attributes associated with an external messaging account.

    The AccountConfiguration class stores various attributes associated with either an 
    incoming source of messages such as an SMS, MMS, POP, or IMAP account, or an outgoing 
    sink for messages such as an SMTP account.

    Note: the AccountConfiguration class will be replaced by a more flexible solution in 
    a future release of Qt Extended.

    \section1 SMTP specific methods

    AccountConfiguration includes a number of SMTP specific methods. emailAddress() is
    used as the from email address when composing email messages. smtpServer()
    is the domain name or DNS address of the SMTP server used to send email
    messages, and smtpPort() is the port address of the server.

    If smtpAuthentication() or smtpEncryption() is true then smtpUsername() and
    smtpPassword() will be used to authenticate and/or encrypt communication
    to the server.

    If useSignature() is true the signature as given by signature() will be
    appended to emails sent.

    Each of these methods has a corresponding setter method.

    \section1 POP/IMAP specific methods

    A number of POP/IMAP specific methods are included in AccountConfiguration.
    mailUserName() and mailPassword() are used when logging into an
    a POP or IMAP server. mailServer() is the domain name or DNS address of
    the server, and mailPort() is the port number of the server. If
    mailEncryption() is true encrypted communication with the server
    will be attempted.

    If canCollectMail() is true then the account can be used to collect
    email. If canDeleteMail() is true then mail will be deleted from
    the server after it is downloaded to the client.

    maxMailSize() is the size of the largest mail to download in full.
    For mails larger than this size only header (summary) information
    such as the title, author, recipients and date will be downloaded.

    markMessageForDeletion() can be used to mark a message for deletion and
    serverUids() returns a list of unique server identifiers for
    messages in the account.

    Each of these methods has a corresponding setter method.

    \section1 IMAP specific methods

    A few IMAP specific methods are included. baseFolder() defines the
    folder that will be used as the root folder when communicating with
    the server, only messages in the base folder and subfolders of the
    base folder will be retrieved. The base folder can be set with
    setBaseFolder().

    mailboxes() returns a list of folders owned by the account, and getMailbox() 
    can be used to find the folder with a specified name.

    \section1 MMS specific methods

    A few MMS specific methods are also included. networkConfig()
    returns the network configuration used for the account, and
    if isAutoDownload() is true then when a preliminary MMS
    notification arrives an attempt at retrieving the full MMS
    message will be made.
*/

/*! 
    Creates an empty and invalid AccountConfiguration.
*/
AccountConfiguration::AccountConfiguration()
{
}

/*!
    Creates an AccountConfiguration by loading the data from the mail store as
    specified by the QMailAccountId \a id. If the account does not exist in the store, 
    then an empty and invalid AccountConfiguration will result.

    \sa QMailStore::account()
*/

AccountConfiguration::AccountConfiguration(const QMailAccountId& id)
    : _id(id),
      _accountType(QMailAccount::POP),
      _mailPort(110),
      _smtpPort(25),
#ifndef QT_NO_OPENSSL
      _mailEncryption(AccountConfiguration::Encrypt_NONE),
      _smtpAuthentication(AccountConfiguration::Auth_NONE),
      _smtpEncryption(AccountConfiguration::Encrypt_NONE),
#endif
      _synchronize(true),
      _deleteMail(false),
      _useSig(false),
      _maxMailSize(-1),
      _checkInterval(-1),
      _intervalCheckRoamingEnabled(false),
      _pushEnabled(false)
{
    QMailStorePrivate::loadAccountSettings(this);   
}

/*!
  Destroys the configuration object.
*/
AccountConfiguration::~AccountConfiguration()
{
}

/*!
  Returns the storage id for this account.
 */
QMailAccountId AccountConfiguration::id() const
{
    return _id;
}

/*!
  Returns the name of the account.
*/
QString AccountConfiguration::accountName() const
{
    return qApp->translate( "AccountList", _accountName.toLatin1() );
}

/*!
  Sets the user name used to login to the outgoing mail server for the account to \a str.

  \sa userName()
*/
void AccountConfiguration::setUserName(const QString &str)
{
    _userName = str;
}

/*!
  Returns the user name used to login to the outgoing mail server for the account.

  \sa setUserName()
*/
QString AccountConfiguration::userName() const
{
    return _userName;
}

/*!
  Sets the email address used when sending mail to \a str.

  \sa emailAddress()
*/
void AccountConfiguration::setEmailAddress(const QString &str)
{
    _emailAddress = str;
}

/*!
  Returns the email address used when sending mail.

  \sa setEmailAddress()
*/
QString AccountConfiguration::emailAddress() const
{
    return _emailAddress;
}

/*!
  Sets the SMTP server used to send mail to \a str.

  \sa smtpServer()
*/
void AccountConfiguration::setSmtpServer(const QString &str)
{
    _smtpServer = str;
}

/*!
  Returns the SMTP server used to send mail.

  \sa setSmtpServer()
*/
QString AccountConfiguration::smtpServer() const
{
    return _smtpServer;
}

/*!
  Returns the SMTP port number used when sending mail.

  \sa setSmtpPort()
*/
int AccountConfiguration::smtpPort() const
{
    return _smtpPort;
}

/*!
  Sets the SMTP port number used when sending mail to \a i.

  \sa smtpPort()
*/
void AccountConfiguration::setSmtpPort(int i)
{
    _smtpPort = i;
}

#ifndef QT_NO_OPENSSL

/*!
  Returns the user name used to login to the outgoing SMTP mail server for the account.

  \sa setSmtpUsername()
*/
QString AccountConfiguration::smtpUsername() const
{
    return _smtpUsername;
}

/*!
  Sets the user name used to login to the outgoing SMTP mail server for the account to \a username.

  \sa smtpUsername()
*/
void AccountConfiguration::setSmtpUsername(const QString& username)
{
    _smtpUsername = username;
}

/*!
  Returns the password used to login to the outgoing SMTP mail server for the account.

  \sa setSmtpPassword()
*/
QString AccountConfiguration::smtpPassword() const
{
    return _smtpPassword;
}

/*!
  Sets the password used to login to the outgoing SMTP mail server for the
  account to \a password.

  \sa smtpPassword()
*/
void AccountConfiguration::setSmtpPassword(const QString& password)
{
    _smtpPassword = password;
}

#endif

/*!
  Returns the authentication type used for the outgoing SMTP mail server for the account.

  \sa setSmtpAuthentication()
*/
AccountConfiguration::AuthType AccountConfiguration::smtpAuthentication() const
{
#ifndef QT_NO_OPENSSL
    return _smtpAuthentication;
#else
    return Auth_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

/*!
  Sets the authentication type used for the outgoing SMTP mail server for the account to \a t.

  \sa smtpAuthentication()
*/
void AccountConfiguration::setSmtpAuthentication(AuthType t)
{
    _smtpAuthentication = t;
}

#endif

/*!
  Returns the encryption type used for the outgoing SMTP mail server for the account.

  \sa setSmtpEncryption()
*/
AccountConfiguration::EncryptType AccountConfiguration::smtpEncryption() const
{
#ifndef QT_NO_OPENSSL
    return _smtpEncryption;
#else
    return Encrypt_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

/*!
  Sets the encryption type used for the outgoing SMTP mail server for the account to \a t.

  \sa smtpEncryption()
*/
void AccountConfiguration::setSmtpEncryption(EncryptType t)
{
    _smtpEncryption = t;
}

#endif

/*!
  Returns the encryption type used for the incoming mail server for the account.

  \sa setMailEncryption()
*/
AccountConfiguration::EncryptType AccountConfiguration::mailEncryption() const
{
#ifndef QT_NO_OPENSSL
    return _mailEncryption;
#else
    return Encrypt_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

/*!
  Sets the encryption type used for the incoming mail server for the account to \a t.

  \sa mailEncryption()
*/
void AccountConfiguration::setMailEncryption(EncryptType t)
{
    _mailEncryption = t;
}

#endif

/*!
  Returns the account type.
*/
QMailAccount::AccountType AccountConfiguration::accountType() const
{
    return _accountType;
};

/*!
  Returns true if signature text should be appended to outgoing mails for the account; otherwise returns false.

  \sa setUseSignature()
*/
bool AccountConfiguration::useSignature() const
{
    return _useSig;
}

/*!
  If \a b is true append signature text to outgoing mails for the account; otherwise do not.

  \sa useSignature()
*/
void AccountConfiguration::setUseSignature(bool b)
{
    _useSig = b;
}

/*!
  Returns the signature text for outgoing mails for the account.

  \sa setSignature()
*/
QString AccountConfiguration::signature() const
{
    return _sig;
}

/*!
  Sets the signature text for outgoing mails for the account to \a str.

  \sa signature()
*/
void AccountConfiguration::setSignature(const QString &str)
{
    _sig = str;
}

/*!
  Returns the user name used to login to the incoming mail server for the account.

  \sa setMailUserName()
*/
QString AccountConfiguration::mailUserName() const
{
    return _mailUserName;
}

/*!
  Sets the user name used to login to the incoming mail server for the account to \a str.

  \sa mailUserName()
*/
void AccountConfiguration::setMailUserName(const QString &str)
{
    _mailUserName = str;
}

/*!
  Returns the password used to login to the incoming mail server for the account.

  \sa setMailPassword()
*/
QString AccountConfiguration::mailPassword() const
{
    return _mailPassword;
}

/*!
  Sets the password used to login to the incoming mail server for the account to \a str.

  \sa mailPassword()
*/
void AccountConfiguration::setMailPassword(const QString &str)
{
    _mailPassword = str;
}

/*!
  Returns the incoming mail server for the account.

  \sa setMailServer()
*/
QString AccountConfiguration::mailServer() const
{
    return _mailServer;
}

/*!
  Sets the incoming mail server for the account to \a str.

  \sa mailServer()
*/
void AccountConfiguration::setMailServer(const QString &str)
{
    _mailServer = str;
}

/*!
  Returns the incoming mail server port for the account.

  \sa setMailPort()
*/
int AccountConfiguration::mailPort() const
{
    return _mailPort;
}

/*!
  Sets the incoming mail server port for the account to \a i.

  \sa mailPort()
*/
void AccountConfiguration::setMailPort(int i)
{
    _mailPort = i;
}

/*!
  Returns true if mail should be deleted from the server after being deleted
  and expunged locally; otherwise returns false.

  \sa setDeleteMail()
*/
bool AccountConfiguration::canDeleteMail() const
{
    return _deleteMail;
}

/*!
  If \a b is true the mail should be deleted from the server after being
  deleted and expunged locally; otherwise do not delete mail from the server.
  When enabled mail is deleted on the server the next time mail is downloaded.

  \sa canDeleteMail()
*/
void AccountConfiguration::setDeleteMail(bool b)
{
    _deleteMail = b;
}

/*!
  Returns the maximum size of mails to be fully downloaded.

  \sa setMaxMailSize()
*/
int AccountConfiguration::maxMailSize() const
{
    return _maxMailSize;
}

/*!
  Sets the maximum size of mails to download in bytes to \a i. For mails
  larger than this size only the mail header will be downloaded.

  \sa maxMailSize()
*/
void AccountConfiguration::setMaxMailSize(int i)
{
    _maxMailSize = i;
}

/*!
  Returns the time in minutes between automatic mail retrieval attempts for
  the account.

  \sa
  setCheckInterval()
*/
int AccountConfiguration::checkInterval() const
{
    return _checkInterval;
}

/*!
  Sets the time in minutes between automatic mail retrieval attempts for the
  account to \a i.

  \sa checkInterval()
*/
void AccountConfiguration::setCheckInterval(int i)
{
    _checkInterval = i;
}

/*!
  Returns true if interval mail checking is enabled even when
  the device is in roaming mode.

  \sa setIntervalCheckRoamingEnabled()
*/
bool AccountConfiguration::intervalCheckRoamingEnabled() const
{
    return _intervalCheckRoamingEnabled;
}

/*!
  Sets whether interval checking is enabled during device roaming to \a enabled.

  \sa intervalCheckRoamingEnabled()
*/
void AccountConfiguration::setIntervalCheckRoamingEnabled(bool enabled)
{
    _intervalCheckRoamingEnabled = enabled;
}

/*!
  Returns true if the account is push email enabled otherwise returns false. 
  
  If an account is push email enabled then the connection to the server for this account 
  should be kept open so that new mail can be discovered as soon as it arrives. This may 
  require less bandwith than repeatedly polling the account for new mail using interval checking. 
  
  Currently only supported for IMAP accounts using the IDLE extension documented in RFC2177.
  
  \sa setPushEnabled()
*/
bool AccountConfiguration::pushEnabled() const
{
    return _pushEnabled;
}

/*!
  Sets whether the account is push email enabled to \a enabled.

  \sa pushEnabled()
*/
void AccountConfiguration::setPushEnabled(bool enabled)
{
    _pushEnabled = enabled;
}

/*!
  Sets the base folder for this account to \a s. This folder is shown in the
  user interface as the root folder for this account.

  \sa baseFolder()
*/
void AccountConfiguration::setBaseFolder(const QString &s)
{
    _baseFolder = s;
}

/*!
  Returns the base folder for this account.

  \sa setBaseFolder()
*/
QString AccountConfiguration::baseFolder() const
{
    return _baseFolder;
}

/*!
  Serialize the account information to the current group of
  the given QSettings file \a conf.

  \sa readSettings()
*/
void AccountConfiguration::saveSettings(QSettings *conf) const
{
    conf->setValue("name", _userName );
    conf->setValue("email", _emailAddress );
    conf->setValue("mailuser", _mailUserName );
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        QByteArray encoded = codec.encode(_mailPassword, "ISO-8859-1");
        QString plain;
        plain = QString::fromLatin1(encoded.constData(), encoded.length());
        conf->setValue("mailpasswordobs", plain);
    }
    conf->setValue("mailserver", _mailServer );
    conf->setValue("mailport", _mailPort);
    conf->setValue("basefolder", _baseFolder);

    conf->setValue("smtpserver", _smtpServer );
    conf->setValue("smtpport", _smtpPort);
#ifndef QT_NO_OPENSSL
    conf->setValue("smtpUsername",_smtpUsername);
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        QByteArray encoded = codec.encode(_smtpPassword, "ISO-8859-1");
        QString plain;
        plain = QString::fromLatin1(encoded.constData(), encoded.length());
        conf->setValue("smtpPasswordObs", plain);
    }
    conf->setValue("smtpAuthentication",_smtpAuthentication);
    conf->setValue("smtpEncryption",_smtpEncryption);
    conf->setValue("mailEncryption",_mailEncryption);
#endif
    conf->setValue("usesig", _useSig);
    conf->setValue("maxmailsize", _maxMailSize);
    conf->setValue("checkinterval", _checkInterval);
    conf->setValue("intervalCheckRoamingEnabled", _intervalCheckRoamingEnabled);
    conf->setValue("pushEnabled", _pushEnabled);
    conf->remove("defaultmailserver");

    if (_useSig) {
        QString path = Qtopia::applicationFileName("qtopiamail", "") + "sig_" + QString::number(_id.toULongLong());
        QFile file(path);
        if ( file.open(QIODevice::WriteOnly) ) {    // file opened successfully
            QTextStream t( &file );        // use a text stream
            t << _sig;
            file.close();
        }
    }

    conf->setValue("synchronize", _synchronize);
    conf->setValue("deletemail", _deleteMail);

    conf->setValue("networkConfig", _networkCfg);
    conf->setValue("autoDownload", _autoDL);
}

/*!
    Deserialize the account information from the current group of
    the given QSettings file \a conf.

  \sa saveSettings()
*/
void AccountConfiguration::readSettings(QSettings *conf)
{
    _accountName = conf->value("accountname").toString();
    _userName = conf->value("name").toString();
    _emailAddress = conf->value("email").toString();
    _mailUserName = conf->value("mailuser").toString();
    //backwards compatible for passwords
    _mailPassword = conf->value("mailpassword").toString();
    if (_mailPassword.isEmpty()) {
        QString obsfucated = conf->value("mailpasswordobs").toString();
        QByteArray encoded(obsfucated.toAscii());
        QMailBase64Codec codec(QMailBase64Codec::Text);
        _mailPassword = codec.decode(encoded, "ISO-8859-1");
    }

    _mailServer = conf->value("mailserver").toString();
    _mailPort = conf->value("mailport", 110).toInt();
    _accountType = static_cast<QMailAccount::AccountType>( conf->value("type", QMailAccount::POP ).toInt() );
    _baseFolder = conf->value("basefolder").toString();

    _useSig = conf->value("usesig").toBool();
    _maxMailSize = conf->value("maxmailsize", 100*1024).toInt();
    setCheckInterval( conf->value("checkinterval", -1).toInt() );
    setIntervalCheckRoamingEnabled(conf->value("intervalCheckRoamingEnabled").toBool());
    setPushEnabled(conf->value("pushEnabled").toBool());

    // write signature to a file (to resolve problem with config.h)
    if (_useSig) {
        QString path = Qtopia::applicationFileName("qtopiamail", "") + "sig_" + QString::number(_id.toULongLong());
        QFile file(path);
        if ( file.open(QIODevice::ReadOnly) ) {    // file opened successfully
            QTextStream t( &file );        // use a text stream
            _sig = t.readAll();
            file.close();
        }
    }

    _smtpServer = conf->value("smtpserver").toString();
    _smtpPort = conf->value("smtpport", 25).toInt();
#ifndef QT_NO_OPENSSL
    _smtpUsername = conf->value("smtpUsername").toString();

    //backwards compatible for passwords
    _smtpPassword = conf->value("smtpPassword").toString();
    if (_smtpPassword.isEmpty()) {
        QString obsfucated = conf->value("smtpPasswordObs").toString();
        QByteArray encoded(obsfucated.toAscii());
        QMailBase64Codec codec(QMailBase64Codec::Text);
        _smtpPassword = codec.decode(encoded, "ISO-8859-1");
    }

    int index = conf->value("smtpAuthentication",0).toInt();
    _smtpAuthentication = static_cast<AuthType>(index);
    index = conf->value("smtpEncryption",0).toInt();
    _smtpEncryption = static_cast<EncryptType>(index);
    index = conf->value("mailEncryption",0).toInt();
    _mailEncryption = static_cast<EncryptType>(index);
#endif
    _synchronize = conf->value("synchronize").toBool();
    _deleteMail = conf->value("deletemail").toBool();

    _networkCfg = conf->value("networkConfig").toString();
    _autoDL = conf->value("autoDownload").toBool();
}

/*!
  Sets the network configuration for the account to \a c.

  \sa networkConfig()
*/
void AccountConfiguration::setNetworkConfig(const QString &c)
{
    _networkCfg = c;
}

/*!
  Returns the network configuration for the account if any; otherwise returns
  the default WAP account from the general Qt Extended network settings.

  \sa setNetworkConfig()
*/
QString AccountConfiguration::networkConfig() const
{
    if (_networkCfg.isEmpty()) {
        QSettings dfltConfig("Trolltech", "Network");
        dfltConfig.beginGroup("WAP");
        return dfltConfig.value("DefaultAccount").toString();
    }
    return _networkCfg;
}

/*!
  Only applicable for MMS accounts. If \a autodl is true sets the account to
  automatically download the full MMS message once the preliminary MMS
  notification arrives. Otherwise full MMS messages will not be automatically
  fetched.

  \sa isAutoDownload()
*/
void AccountConfiguration::setAutoDownload(bool autodl)
{
    _autoDL = autodl;
}

/*!
  Only applicable for MMS accounts. Returns true if the full MMS will be
  retrieved when the preliminary MMS notification arrives; otherwise returns
  false.

  \sa setAutoDownload()
*/
bool AccountConfiguration::isAutoDownload() const
{
    return _autoDL;
}

/*!
  Returns true if the account can be used to collect mail.
*/
bool AccountConfiguration::canCollectMail() const
{
    if (_accountType == QMailAccount::POP || _accountType == QMailAccount::IMAP)
        return true;

    return false;
}

/*!
  Returns true if the account can be used to send mail.
*/
bool AccountConfiguration::canSendMail() const
{
    //email accounts can send only if smtp servers and email addresses have been defined.
    if (_accountType == QMailAccount::POP || _accountType == QMailAccount::IMAP) {
        return !_smtpServer.isEmpty() && !_emailAddress.isEmpty();
    }

#ifndef ENABLE_UNCONDITIONAL_MMS_SEND
    // MMS can only send if a network configuration is specified
    if (_accountType == QMailAccount::MMS) {
        return !_networkCfg.isEmpty();
    }
#endif

    if (_accountType == QMailAccount::Collective) {
        return !_emailAddress.isEmpty();
    }

    return (_accountType != QMailAccount::System);
}

void AccountConfiguration::setId(const QMailAccountId &id)
{
    _id = id;
}

