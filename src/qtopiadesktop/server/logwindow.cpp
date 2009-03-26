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
#include "logwindow.h"

#include <qcopchannel_qd.h>
#include <desktopsettings.h>

#include <qdebug.h>
#include <QTextEdit>
#include <QHBoxLayout>

LogWindow::LogWindow()
    : QWidget( 0, Qt::Tool|Qt::WindowStaysOnTopHint )
{
    setWindowTitle( tr("Qtopia Sync Agent Messages") );

    DesktopSettings settings( "logwindow" );
    QRect r = settings.value( "geometry" ).toRect();
    if ( r.isValid() )
        setGeometry( r );

    textView = new QTextEdit;
    textView->setReadOnly( true );
    textView->setLineWrapMode( QTextEdit::NoWrap );
    textView->clear();

    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );

    hbox->addWidget( textView );

    QCopChannel *status = new QCopChannel( "QD/Status", this );
    connect( status, SIGNAL(received(QString,QByteArray)),
            this, SLOT(statusMessage(QString,QByteArray)) );
    QCopChannel *sys = new QCopChannel( "QD/Connection", this );
    connect( sys, SIGNAL(received(QString,QByteArray)), this, SLOT(connectionMessage(QString,QByteArray)) );
}

LogWindow::~LogWindow()
{
}

LogWindow *LogWindow::getInstance()
{
    static LogWindow *instance = 0;
    if ( !instance )
        instance = new LogWindow;
    return instance;
}

void LogWindow::statusMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "message(QString)" ) {
        QString msg;
        stream >> msg;
        addText( msg );
    }
}

void LogWindow::connectionMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "setHint(QString,QString)" ) {
        QString hint;
        QString section;
        stream >> hint >> section;
        addText( tr("Hint: %1 (see manual: %2)").arg(hint).arg(section) );
    }
}

void LogWindow::addText( const QString &msg )
{
    static bool first = true;
    if ( first ) {
        first = false;
        textView->setHtml( msg );
    } else {
        textView->append( msg );
    }
}

void LogWindow::closeEvent( QCloseEvent * /*e*/ )
{
    DesktopSettings settings( "logwindow" );
    settings.setValue( "geometry", geometry() );
}

