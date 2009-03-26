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
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>

#include <trace.h>
QD_LOG_OPTION(SyncAuthentication)

#include "syncauthentication.h"
#include "log.h"

#include <qtopianamespace.h>
#include <QTextBrowser>

QByteArray SyncAuthentication::serverId()
{
    return Qtopia::deviceId().toLocal8Bit();
}

QByteArray SyncAuthentication::ownerName()
{
    return Qtopia::ownerName().toLocal8Bit();
}

QByteArray SyncAuthentication::loginName()
{
    struct passwd *pw = 0;
    pw = ::getpwuid( ::geteuid() );
    Q_ASSERT(pw);
    QByteArray ret( pw->pw_name );
    return ret;
}

bool SyncAuthentication::isAuthorized( const QHostAddress & /*peeraddress*/ )
{
    // You can always connect from any address
    // Having a stupid default for this check was one of the most annoying things in Qtopia 2.
    return true;
}

bool SyncAuthentication::checkUser( const QByteArray &user )
{
    TRACE(SyncAuthentication) << "SyncAuthentication::checkUser" << user << "vs" << loginName();
    return ( user == loginName() );
}

// This is never recursively called.
// The calling function cleans up the message pointer.
static QMessageBox *message = 0;
static bool _checkPassword( const QByteArray &password )
{
    TRACE(SyncAuthentication) << "::_checkPassword";
    static int lastdenial = 0;
    static int denials = 0;
    int now = ::time(0);

    // You get several prompts (depending on the type) before the system locks out
    // new connections for 10 minutes. Actually, you can just restart the app but
    // I guess that wasn't as easy when this code ran in the server.
    if ( now < lastdenial + 600 )
        denials = 0;

    // Detect old Qtopia Desktop (no password)
    if ( password.isEmpty() ) {
        // You only get this message once per 10 minutes (since it's so annoying).
        if ( denials < 1 ) {
            message = new QMessageBox(
                    "Sync Connection",
                    "<qt>An unauthorized system is requesting access to this device. "
                    "If you are using a version of Qtopia Desktop older than 1.5.1, "
                    "please upgrade.",
                    QMessageBox::Warning,
                    QMessageBox::Cancel, QMessageBox::NoButton, QMessageBox::NoButton,
                    0, Qt::Dialog|Qt::WindowStaysOnTopHint);
            message->setModal(true);
            message->setButtonText(QMessageBox::Cancel, "Deny");
            message->exec();

            denials++;
            lastdenial = now;
        }
        return false;
    }

    // Second, check sync password...
    if ( password.left(6) == "Qtopia" ) {
        QSettings cfg("Trolltech","Security");
        cfg.beginGroup("Sync");
        QStringList pwds = cfg.value("Passwords").toStringList();
        foreach( const QString &pwd, pwds ) {
            if ( pwd.isEmpty() ) continue;
            char *tmp = crypt( password.mid(8).constData(), pwd.left(2).toLocal8Bit().constData() );
            QByteArray cpassword( tmp );
            LOG() << "Comparing crypted passwords" << pwd << cpassword;
            if ( pwd == cpassword ) {
                return true;
            }
        }

        // Unrecognized system. Be careful...
        message = new QMessageBox(
                "Sync Connection",
                "<qt>An unrecognized system is requesting access to this device. "
                "If you have just initiated a Sync for the first time, this is normal.",
                QMessageBox::Warning,
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton,
                0, Qt::Dialog|Qt::WindowStaysOnTopHint);
        message->setModal(true);
        message->setButtonText(QMessageBox::Yes, "Allow");
        message->setButtonText(QMessageBox::No, "Deny");

        bool die = false;
        // You get 3 of these prompts per 10 minutes.
        if ( denials >= 3 ) {
            USERLOG("Too many denials... try again in 10 minutes");
            die = true;
        }
        int result;
        if ( !die ) {
            result = message->exec();
            if ( result == QMessageBox::No || result == QDialog::Rejected ) {
                USERLOG("User rejected connection");
                die = true;
            }
        }
        if ( die ) {
            denials++;
            lastdenial = now;
            return false;
        } else {
            USERLOG("User accepted connection");
            const char salty[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/.";
            char salt[2];
            salt[0]= salty[rand() % (sizeof(salty)-1)];
            salt[1]= salty[rand() % (sizeof(salty)-1)];
            char *tmp = crypt( password.mid(8).constData(), salt );
            QByteArray cpassword( tmp );
            LOG() << "New (crypted) password is" << cpassword;
            denials = 0;
            pwds.prepend(QString(cpassword));
            cfg.setValue("Passwords", pwds);
            return true;
        }
    }

    return false;
}

bool SyncAuthentication::checkPassword( const QByteArray &password )
{
    TRACE(SyncAuthentication) << "SyncAuthentication::checkPassword" << password;

    static bool recursive = false;
    if ( recursive ) {
        WARNING() << "***** Recursive call to checkPassword!" << __FILE__ << __LINE__;
        return false;
    }
    recursive = true;

    Q_ASSERT(!message);

    bool ret = _checkPassword(password);

    if ( message ) {
        delete message;
        message = 0;
    }

    recursive = false;

    return ret;
}

void SyncAuthentication::clearDialogs()
{
    TRACE(SyncAuthentication) << "SyncAuthentication::clearDialogs";
    if ( message ) {
        message->reject();
        message->deleteLater();
    }
    message = 0;
}

