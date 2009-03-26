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

#include "editaccount.h"

#include <QtopiaApplication>
#include <private/qtopiainputdialog_p.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qscrollarea.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvalidator.h>

#include <qmailaccount.h>
#include <private/accountconfiguration_p.h>

static const AccountConfiguration::AuthType authenticationType[] = {
    AccountConfiguration::Auth_NONE,
#ifndef QT_NO_OPENSSL
    AccountConfiguration::Auth_LOGIN,
    AccountConfiguration::Auth_PLAIN,
#endif
    AccountConfiguration::Auth_INCOMING
};

#ifndef QT_NO_OPENSSL
static int authenticationIndex(AccountConfiguration::AuthType type)
{
    const int numTypes = sizeof(authenticationType)/sizeof(AccountConfiguration::AuthType);
    for (int i = 0; i < numTypes; ++i)
        if (type == authenticationType[i])
            return i;

    return 0;
};
#endif


class PortValidator : public QValidator
{
public:
    PortValidator(QWidget *parent = 0, const char *name = 0)
        : QValidator(parent) {
      setObjectName(name);
    }

    State validate(QString &str, int &) const
    {
        // allow empty strings, as it's a bit awkward to edit otherwise
        if ( str.isEmpty() )
            return QValidator::Acceptable;

        bool ok = false;
        int i = str.toInt(&ok);
        if ( !ok )
            return QValidator::Invalid;

        if ( i <= 0 || i >= 65536 )
            return QValidator::Invalid;

        return QValidator::Acceptable;
    }
};

EditAccount::EditAccount( QWidget* parent, const char* name, Qt::WFlags fl )
    : QDialog(parent, fl)
    , account(0)
    , accountNameInput(new QLineEdit)
{
    setupUi(this);
    setObjectName(name);

    //connect custom slots
    connect(setSignatureButton,SIGNAL(clicked()),SLOT(sigPressed()));
    connect(accountType,SIGNAL(currentIndexChanged(int)),SLOT(typeChanged(int)));
    connect(authentication,SIGNAL(currentIndexChanged(int)),SLOT(authChanged(int)));
    connect(emailInput,SIGNAL(textChanged(QString)),SLOT(emailModified()));
    connect(pushCheckBox,SIGNAL(stateChanged(int)),SLOT(pushCheckChanged(int)));
    connect(intervalCheckBox,SIGNAL(stateChanged(int)),SLOT(intervalCheckChanged(int)));

    // this functionality is not currently used
    //connect(mailboxButton,SIGNAL(clicked()),SLOT(configureFolders()));
    mailboxButton->hide();

    emailTyped = false;

    QtopiaApplication::setInputMethodHint(mailPortInput, QtopiaApplication::Number);
    QtopiaApplication::setInputMethodHint(smtpPortInput, QtopiaApplication::Number);

    const QString uncapitalised("email noautocapitalization");

    // These fields should not be autocapitalised
    QtopiaApplication::setInputMethodHint(mailUserInput, QtopiaApplication::Named, uncapitalised);
    QtopiaApplication::setInputMethodHint(mailServerInput, QtopiaApplication::Named, uncapitalised);

    QtopiaApplication::setInputMethodHint(emailInput, QtopiaApplication::Named, uncapitalised);
    QtopiaApplication::setInputMethodHint(smtpUsernameInput, QtopiaApplication::Named, uncapitalised);
    QtopiaApplication::setInputMethodHint(smtpServerInput, QtopiaApplication::Named, uncapitalised);

    // Too easy to mistype numbers in phone mode
    mailPasswInput->installEventFilter( this );
    accountNameInput->installEventFilter( this );
    defaultMailCheckBox->installEventFilter( this );
    PortValidator *pv = new PortValidator(this);
    mailPortInput->setValidator(pv);
    smtpPortInput->setValidator(pv);
    mailPasswInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
#ifdef QT_NO_OPENSSL
    encryption->hide();
    lblEncryption->hide();
    authentication->hide();
    lblAuthentication->hide();
    smtpUsernameInput->hide();
    lblSmtpUsername->hide();
    smtpPasswordInput->hide();
    lblSmtpPassword->hide();
    encryptionIncoming->hide();
    lblEncryptionIncoming->hide();
#else
    authentication->addItem("INCOMING"); // notr
    smtpPasswordInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
#endif

    typeChanged(1);
    createTabbedView();
    setLayoutDirection( qApp->layoutDirection() );

    currentTabChanged(0);
}

void EditAccount::setAccount(QMailAccount *in, AccountConfiguration* conf, bool defaultServer)
{
    account = 0;

    if (!in->id().isValid()) {
        // New account
        accountNameInput->setText("");
        emailInput->setText("");
        mailUserInput->setText("");
        mailPasswInput->setText("");
        mailServerInput->setText("");
        smtpServerInput->setText("");
        mailPortInput->setText("110");
        smtpPortInput->setText("25");
#ifndef QT_NO_OPENSSL
        smtpUsernameInput->setText("");
        smtpPasswordInput->setText("");
        encryption->setCurrentIndex(0);
        authentication->setCurrentIndex(0);
        smtpUsernameInput->setEnabled(false);
        lblSmtpUsername->setEnabled(false);
        smtpPasswordInput->setEnabled(false);
        lblSmtpPassword->setEnabled(false);
        encryptionIncoming->setCurrentIndex(0);
#endif
        pushCheckBox->setChecked(false);
        intervalCheckBox->setChecked(false);
        roamingCheckBox->setEnabled(false);
#ifdef QTOPIA_HOMEUI
        roamingCheckBox->hide();
#endif
        setWindowTitle( tr("Create new account", "translation not longer than English") );

        account = in;
        config = conf;
        typeChanged( 0 );
    } else {
        account = in;
        config = conf;

        accountNameInput->setText( config->accountName() );
        nameInput->setText( config->userName() );
        emailInput->setText( config->emailAddress() );
        mailUserInput->setText( config->mailUserName() );
        mailPasswInput->setText( config->mailPassword() );
        mailServerInput->setText( config->mailServer() );
        smtpServerInput->setText( config->smtpServer() );
        deleteCheckBox->setChecked( config->canDeleteMail() );

        sigCheckBox->setChecked( config->useSignature() );
        sig = config->signature();

        maxSize->setValue(config->maxMailSize());
        thresholdCheckBox->setChecked( config->maxMailSize() != -1 );
        smtpPortInput->setText( QString::number( config->smtpPort() ) );
        defaultMailCheckBox->setChecked( defaultServer );
#ifndef QT_NO_OPENSSL
        smtpUsernameInput->setText(config->smtpUsername());
        smtpPasswordInput->setText(config->smtpPassword());
        authentication->setItemText(3, accountType->currentText());
        authentication->setCurrentIndex(authenticationIndex(config->smtpAuthentication()));
        encryption->setCurrentIndex(static_cast<int>(config->smtpEncryption()));
        AccountConfiguration::AuthType type = authenticationType[authentication->currentIndex()];
        const bool enableCredentials(type == AccountConfiguration::Auth_LOGIN || type == AccountConfiguration::Auth_PLAIN);
        smtpUsernameInput->setEnabled(enableCredentials);
        lblSmtpUsername->setEnabled(enableCredentials);
        smtpPasswordInput->setEnabled(enableCredentials);
        lblSmtpPassword->setEnabled(enableCredentials);
        encryptionIncoming->setCurrentIndex(static_cast<int>(config->mailEncryption()));
#endif
        pushCheckBox->setChecked( config->pushEnabled() );
        intervalCheckBox->setChecked( config->checkInterval() > 0 );
        intervalPeriod->setValue( qAbs( config->checkInterval() ) );
        roamingCheckBox->setChecked( !config->intervalCheckRoamingEnabled() );
        roamingCheckBox->setEnabled( intervalCheckBox->isChecked() );
#ifdef QTOPIA_HOMEUI
        roamingCheckBox->hide();
#endif

        if ( account->messageSources().contains("pop3", Qt::CaseInsensitive)) {
            accountType->setCurrentIndex(0);
            typeChanged(0);
        } else if ( account->messageSources().contains("imap4", Qt::CaseInsensitive)) {
            accountType->setCurrentIndex(1);
            typeChanged(1);
            imapBaseDir->setText( config->baseFolder() );
        } else {
            accountType->setCurrentIndex(2);
            typeChanged(2);
        }
        mailPortInput->setText( QString::number( config->mailPort() ) );
    }

    nameInput->setText( config->userName() );
}

void EditAccount::createTabbedView()
{
    delete layout();

    QVBoxLayout* thelayout = new QVBoxLayout(this);
    thelayout->setMargin(0);
    thelayout->setSpacing(4);

    QHBoxLayout* formLayout = new QHBoxLayout;
    formLayout->setMargin(6);
    formLayout->setSpacing(4);
    formLayout->addWidget(new QLabel(tr("Name")));
    formLayout->addWidget(accountNameInput);
    thelayout->addLayout(formLayout);

    QFrame* separator = new QFrame;
    separator->setFrameStyle(QFrame::HLine);
    thelayout->addWidget(separator);

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    thelayout->addWidget(tabWidget);

    updateGeometry();

    accountNameInput->setFocus();
}

void EditAccount::currentTabChanged(int index)
{
    // Change the name to select the relevant help page
    setObjectName(index == 0 ? "email-account-in" : "email-account-out");
}

bool EditAccount::eventFilter( QObject* o, QEvent *e )
{
    if ((o == defaultMailCheckBox) && (e->type() == QEvent::FocusIn)) {
        if (tabWidget && tabWidget->currentIndex() != 1)
            tabWidget->setCurrentIndex( 1 );
    }

    return QDialog::eventFilter(o,e);
}

//TODO fix, unused code, slot inaccessible since button not visible on form

// void EditAccount::configureFolders()
// {
//     MailboxSelector sel(this, "sel", account);
//     QtopiaApplication::execDialog(&sel);
// }

void EditAccount::sigPressed()
{
    if (sigCheckBox->isChecked()) {
        QString sigText;

        if (sig.isEmpty())
            sigText = QLatin1String("~~\n") + nameInput->text();
        else
            sigText = sig;

#ifdef QTOPIA_HOMEUI
        bool ok = false;
        sigText = QtopiaInputDialog::getMultiLineText(this, tr("Signature"), QString(),
                QtopiaApplication::Words, QString(), sigText, &ok);
        if (ok)
            sig = sigText;
#else
        SigEntry sigEntry(this, "sigEntry", static_cast<Qt::WFlags>(1));
        sigEntry.setEntry(sigText);
        if ( QtopiaApplication::execDialog(&sigEntry) == QDialog::Accepted)
            sig = sigEntry.entry();
#endif
    }
}

void EditAccount::emailModified()
{
    emailTyped = true;
}

void EditAccount::authChanged(int index)
{
#ifndef QT_NO_OPENSSL
    AccountConfiguration::AuthType type = authenticationType[index];
    bool enableCredentials = (type == AccountConfiguration::Auth_LOGIN || type == AccountConfiguration::Auth_PLAIN);

    smtpUsernameInput->setEnabled(enableCredentials);
    lblSmtpUsername->setEnabled(enableCredentials);
    smtpPasswordInput->setEnabled(enableCredentials);
    lblSmtpPassword->setEnabled(enableCredentials);

    if (!enableCredentials) {
        smtpUsernameInput->clear();
        smtpPasswordInput->clear();
    }
#else
    Q_UNUSED(index);
#endif
}

void EditAccount::typeChanged(int)
{
#ifndef QT_NO_OPENSSL
    // Keep the authentication option in sync with the selected account type
    authentication->setItemText(3, accountType->currentText());
#endif

    // this functionality is not currently used
    //mailboxButton->setEnabled( true );
    mailPortInput->setEnabled( true );

    smtpPortInput->setEnabled( true );
    smtpServerInput->setEnabled( true );
    deleteCheckBox->setEnabled( true );
    defaultMailCheckBox->setEnabled( true );
    thresholdCheckBox->setEnabled( true );

    if (accountType->currentText() == "POP") {
        mailPortInput->setText("110");

        baseFolderLabel->hide();
        imapBaseDir->hide();
        //mailboxButton->hide();
        pushCheckBox->setEnabled( false );
        pushCheckBox->hide();
        
        if (encryptionIncoming->count() > 2)
            encryptionIncoming->removeItem(2);

        if (!account || !account->id().isValid()) {
            // Default to non-deleting, if not previously configured
            deleteCheckBox->setChecked(false);
        }
    } else if (accountType->currentText() == "IMAP") {
        mailPortInput->setText("143");

        baseFolderLabel->show();
        imapBaseDir->show();
        //mailboxButton->show();
        //mailboxButton->setEnabled( account->mailboxes().count() > 0 );
        pushCheckBox->setEnabled( true );
        pushCheckBox->show();
        
        if (encryptionIncoming->count() < 3)
            encryptionIncoming->addItem("TLS");

        if (!account || !account->id().isValid()) {
            // Default to deleting, if not previously configured
            deleteCheckBox->setChecked(true);
        }
    }
}

void EditAccount::intervalCheckChanged(int enabled)
{
#ifndef QTOPIA_HOMEUI
    if (enabled && account) {
        QMessageBox::warning(0,
                             tr("Interval checking"),
                             tr("<qt>Interval checking may generate a "
                                "significant amount of data traffic.</qt>"),
                             tr("OK"));
    }
    roamingCheckBox->setEnabled(enabled && account);
#endif
    pushCheckBox->setEnabled(!enabled);
}

void EditAccount::pushCheckChanged(int enabled)
{
    intervalCheckBox->setEnabled(!enabled);
}

void EditAccount::deleteAccount()
{
    done(2);
}

bool EditAccount::isDefaultAccount()
{
    return defaultMailCheckBox->isChecked();
}

void EditAccount::accept()
{
    QString name = accountNameInput->text();
    if ( name.trimmed().isEmpty() ) {
        name = mailServerInput->text();
        if ( name.trimmed().isEmpty() )
            name = smtpServerInput->text();
    }

    if (name.trimmed().isEmpty()) {
        int ret = QMessageBox::warning(this, tr("Empty account name"),
                tr("<qt>Do you want to continue and discard any changes?</qt>"),
                QMessageBox::Yes, QMessageBox::No|QMessageBox::Default|QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            reject();
        return;
    }

    QMailAccountId accountId(account->id());

    if ( accountType->currentText() == "POP" )  {
        *account = QMailAccount::accountFromSource("pop3");
    } else if ( accountType->currentText() == "IMAP")  {
        *account = QMailAccount::accountFromSource("imap4");
    }

    account->setId( accountId );
    account->setAccountName( accountNameInput->text() );

    config->setEmailAddress( emailInput->text() );
    config->setUserName( nameInput->text() );
    config->setMailUserName( mailUserInput->text() );
    config->setMailPassword( mailPasswInput->text() );
    config->setMailServer( mailServerInput->text() );
    config->setSmtpServer( smtpServerInput->text() );
    config->setDeleteMail( deleteCheckBox->isChecked() );

    config->setUseSignature( sigCheckBox->isChecked() );
    config->setSignature( sig );

    if ( thresholdCheckBox->isChecked() ) {
        config->setMaxMailSize( maxSize->value() );
    } else {
        config->setMaxMailSize( -1 );
    }
    config->setPushEnabled( pushCheckBox->isChecked() );
    if ( intervalCheckBox->isChecked() )
        config->setCheckInterval( intervalPeriod->value() );
    else
        config->setCheckInterval( -intervalPeriod->value() );
    config->setIntervalCheckRoamingEnabled( !roamingCheckBox->isChecked() );
    if ( accountType->currentText() == "IMAP") 
        config->setBaseFolder( imapBaseDir->text() );

    QString temp;
    bool result;
    temp = mailPortInput->text();
    config->setMailPort( temp.toInt(&result) );
    if ( (!result) ) {
        // should only happen when the string is empty, since we use a validator.
        if (accountType->currentText() == "POP")
            config->setMailPort( 110 );
        else
            config->setMailPort( 143 );
    }

    temp = smtpPortInput->text();
    config->setSmtpPort( temp.toInt(&result) );
    // should only happen when the string is empty, since we use a validator.
    if ( !result )
        config->setSmtpPort( 25 );

    //try to guess email address
    if ( (!emailTyped) && (config->emailAddress().isEmpty()) ) {
        QString address = config->smtpServer();

        if ( address.count('.')) {
            config->setEmailAddress( config->mailUserName() + "@" +
            address.mid( address.indexOf('.') + 1, address.length() ) );
        } else if (address.count('.') == 1) {
            config->setEmailAddress( config->mailUserName() + "@" + address );
        }
    }

    //set an accountname
    if ( account->accountName().isEmpty() ) {
        int pos = name.indexOf('.');
        if ( pos != -1) {
            name = name.mid( pos + 1, name.length() );

            pos = name.indexOf('.', pos);
            if (pos != -1)
                name = name.mid(0, pos);
        }

        account->setAccountName( name );
    }

#ifndef QT_NO_OPENSSL
    config->setSmtpUsername(smtpUsernameInput->text());
    config->setSmtpPassword(smtpPasswordInput->text());
    config->setSmtpAuthentication(authenticationType[authentication->currentIndex()]);
    config->setSmtpEncryption(static_cast<AccountConfiguration::EncryptType>(encryption->currentIndex()));
    config->setMailEncryption(static_cast<AccountConfiguration::EncryptType>(encryptionIncoming->currentIndex()));
#endif

    QDialog::accept();
}

SigEntry::SigEntry(QWidget *parent, const char *name, Qt::WFlags fl )
    : QDialog(parent,fl)
{
    setObjectName(name);
    setWindowTitle( tr("Signature") );

    QGridLayout *grid = new QGridLayout(this);
    input = new QTextEdit(this);
    grid->addWidget(input, 0, 0);
}

