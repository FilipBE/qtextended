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
#include "security.h"

#include <qtopiaapplication.h>
#include <qpassworddialog.h>
#include <qtopiaipcenvelope.h>
#include <qsoftmenubar.h>
#ifdef QTOPIA_CELL
#include "phonesecurity.h"
#endif

#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QHostAddress>
#include <QCloseEvent>
#include <QSettings>


class WaitScreen : public QLabel
{
    Q_OBJECT
public:
    WaitScreen( QWidget *parent ) : QLabel( parent )
    {
        setText(tr("Please wait ..."));
        QSoftMenuBar::clearLabel(this, Qt::Key_Back);
    }

public slots:
    void makeClosable()
    {
        canClose = true;
        close();
    }
protected:
    void show()
    {
        canClose = false;
        // should show error if it timesout.
        QTimer::singleShot(5000, this, SLOT(makeClosable()));
        QLabel::show();
    }

    void closeEvent(QCloseEvent *e){
        // great way to appear to freeze device
        if (canClose)
            e->accept();
        else
            e->ignore();
    }

    void keyPressEvent(QKeyEvent *e)
    {
        // great way to appear to freeze device
        e->accept();
    }

private:
    bool canClose;

};

Security::Security( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
#ifdef QTOPIA_CELL
    , mStatus(0)
#endif
{
#ifndef QTOPIA_CELL
    setModal( true );
#endif
    setupUi( this );
    valid=false;
    reentryCheck=false;
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Passcode");
    passcode = cfg.value("passcode").toString();
#ifdef QTOPIA_CELL
    connect(passcode_poweron, SIGNAL(toggled(bool)), SLOT(markProtected(bool)));
#else
    passcode_poweron->setChecked(cfg.value("passcode_poweron",false).toBool());
#endif

#ifdef QTOPIA_CELL
    phonesec = new PhoneSecurity(this);
    connect(phonesec,SIGNAL(changed(bool)),this,SLOT(phoneChanged(bool)));
    connect(phonesec,SIGNAL(locked(bool)),this,SLOT(phoneLocked(bool)));
    connect(phonesec,SIGNAL(lockDone(bool)),this,SLOT(phoneLockDone(bool)));
#endif

    (void)QSoftMenuBar::menuFor(this); // just to get help
#ifdef QTOPIA_CELL
    QSoftMenuBar::setCancelEnabled(this, false);
#endif

    connect(changepasscode,SIGNAL(clicked()), this, SLOT(changePassCode()));
    connect(clearpasscode,SIGNAL(clicked()), this, SLOT(clearPassCode()));
#ifdef QTOPIA_CELL
    connect(pinSelection, SIGNAL(activated(int)), this, SLOT(updateGUI()));
#endif

    updateGUI();
}

Security::~Security()
{
}

void Security::phoneChanged(bool success)
{
#ifdef QTOPIA_CELL
    mStatus->makeClosable();
    if (success)
        QMessageBox::information(this, tr("Success"), tr("<p>Successfully changed PIN."));
    else
        QMessageBox::warning(this, tr("Failure"), tr("<p>Could not change PIN."));
#else
    Q_UNUSED(success);
#endif
}

void Security::phoneLocked(bool success)
{
#ifdef QTOPIA_CELL
    if (!success)
        QMessageBox::warning(this, tr("Failure"), tr("<p>Could not change protection state."));
#else
    Q_UNUSED(success);
#endif
}

void Security::phoneLockDone(bool success)
{
    passcode_poweron->blockSignals(true);
    passcode_poweron->setChecked(success);
    passcode_poweron->blockSignals(false);
    passcode_poweron->setEnabled(true);
}

void Security::updateGUI()
{
#ifdef QTOPIA_CELL
    // phone passcode.  they are never 'cleared'
    clearpasscode->hide();
    passcode_poweron->setEnabled(false);
    phonesec->setLockType(pinSelection->currentIndex());
#else
    bool empty = passcode.isEmpty();

    changepasscode->setText( empty ? tr("Set code" )
                             : tr("Change code" )  );
    passcode_poweron->setEnabled( !empty );
    clearpasscode->setEnabled( !empty );
#endif
}

void Security::markProtected(bool b)
{
#ifdef QTOPIA_CELL
    if (reentryCheck)
        return;
    passcode_poweron->setEnabled(false);
    QString p;
    if (pinSelection->currentIndex() == 0) {
        p = enterPassCode(tr("Enter SIM PIN"), false);
    } else {
        p = enterPassCode(tr("Enter Phone PIN"), false);
    }
    if (!p.isNull()) {
        phonesec->markProtected(pinSelection->currentIndex(), b, p);
    } else {
        reentryCheck = true;
        passcode_poweron->setEnabled( true );
        passcode_poweron->setChecked( !b );
        reentryCheck = false;
    }

#else
    Q_UNUSED(b);
#endif
}

void Security::setVisible(bool vis)
{
#ifndef QTOPIA_CELL
    if (vis) {
        valid=false;
        setEnabled(false);
        QDialog::setVisible(vis);
        if ( passcode.isEmpty() ) {
            // could insist...
            //changePassCode();
            //if ( passcode.isEmpty() )
            //reject();
        } else if ( timeout.isNull() || timeout.elapsed() > 2000 ) {
            // Insist on re-entry of passcode if more than 2 seconds have elapsed.
            QString pc = enterPassCode(tr("Enter Security passcode"));
            if ( pc != passcode ) {
                QMessageBox::critical(this, tr("Passcode incorrect"),
                        tr("<qt>The passcode entered is incorrect. Access denied</qt>"));
                //          reject();
                qApp->quit();
                return;
            }
            timeout.start();
        }
        setEnabled(true);
    } else {
        QDialog::setVisible(vis);
    }
#else
    QWidget::setVisible(vis);
#endif
    valid=true;
}

void Security::accept()
{
    applySecurity();
    QDialog::accept();
}

void Security::done(int r)
{
    QDialog::done(r);
    close();
}

void Security::applySecurity()
{
#ifndef QTOPIA_CELL
    if ( valid ) {        
        QSettings cfg("Trolltech","Security");
        cfg.beginGroup("Passcode");
        cfg.setValue("passcode",passcode);
        cfg.setValue("passcode_poweron",passcode_poweron->isChecked());
    }
#endif

    QtopiaIpcEnvelope("QPE/System", "securityChanged()");
}

// for the phone, it requires: once to check old, (even though in
// security app already), and
// twice for new.  Once it has all three, you can attempt to set the sim pin,
// or security pin as required.
void Security::changePassCode()
{
#ifdef QTOPIA_CELL
    QString old;
#endif
    QString new1;
    QString new2;
    bool    mismatch = false;
    bool    valid = true;

    do {
#ifdef QTOPIA_CELL
        // should check current pin first, may not be set.
        if (old.isNull()) {
            old = enterPassCode(tr("Enter current PIN"), false, false);
            // indicates dialog was canceled.
            if ( old.isNull() )
                return;
        }
        if (mismatch) {
            new1 = enterPassCode(tr("Mismatch: Retry new PIN"), false, false);
        } else if (!valid) {
            new1 = enterPassCode(tr("Invalid: Retry new PIN"), false, false);
        } else {
            new1 = enterPassCode(tr("Enter new PIN"), false, false);
        }
#else
        if (mismatch) {
            new1 = enterPassCode(tr("Mismatch: Retry new code"), true);
        } else if (!valid) {
            new1 = enterPassCode(tr("Invalid: Retry new code"), true);
        } else {
            new1 = enterPassCode(tr("Enter new passcode"), true);
        }
#endif
        // indicates dialog was canceled.
        if ( new1.isNull() )
            return;

        if (new1.length() < 4 || new1.length() > 8) {
            // GSM 51.010, section 27.14.2 says that PIN's must be between
            // 4 and 8 digits in length.
            valid = false;
        } else {
#ifdef QTOPIA_CELL
            new2 = enterPassCode(tr("Re-enter new PIN"), false, false);
#else
            new2 = enterPassCode(tr("Re-enter new passcode"), true);
#endif
            if ( new2.isNull() )
                return;

            valid = !new2.isEmpty();
            mismatch = new1 != new2;
        }

    } while (mismatch || !valid);

#ifdef QTOPIA_CELL
    phonesec->changePassword(pinSelection->currentIndex(),old,new2);

    if (!mStatus)
        mStatus = new WaitScreen(0);
    mStatus->showMaximized();
#else
    passcode = new1;
    updateGUI();
#endif
}

void Security::clearPassCode()
{
    passcode = QString();
    updateGUI();
}

QString Security::enterPassCode(const QString& prompt, bool encrypt, bool last)
{
#ifdef QTOPIA_CELL
    return QPasswordDialog::getPassword(this, prompt, encrypt ? QPasswordDialog::Crypted : QPasswordDialog::Pin, last);
#else
    return QPasswordDialog::getPassword(this, prompt, encrypt ? QPasswordDialog::Crypted : QPasswordDialog::Plain, last);
#endif
}

#include "security.moc"
