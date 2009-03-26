/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

/*
  command.cpp
*/

#include <QProcess>

#include "command.h"

QT_BEGIN_NAMESPACE

void executeCommand( const Location& location, const QString& format,
		     const QStringList& args )
{
    QString actualCommand;
    for ( int i = 0; i < (int) format.length(); i++ ) {
	int ch = format[i].unicode();
	if ( ch > 0 && ch < 8 ) {
	    actualCommand += args[ch - 1];
	} else {
	    actualCommand += format[i];
	}
    }

    QString toolName = actualCommand;
    int space = toolName.indexOf( QLatin1Char(' ') );
    if ( space != -1 )
	toolName.truncate( space );

    QProcess process;
    process.start(QLatin1String("sh"),
        QStringList() << QLatin1String("-c") << actualCommand );
    process.waitForFinished();

    if (process.exitCode() == 127)
	location.fatal( tr("Couldn't launch the '%1' tool")
			.arg(toolName),
			tr("Make sure the tool is installed and in the"
			   " path.") );

    QString errors = QString::fromLocal8Bit(process.readAllStandardError());
    while ( errors.endsWith(QLatin1Char('\n')) )
        errors.truncate( errors.length() - 1 );
    if ( !errors.isEmpty() )
	location.fatal( tr("The '%1' tool encountered some problems")
			.arg(toolName),
			tr("The tool was invoked like this:\n%1\n"
			   "It emitted these errors:\n%2")
			.arg(actualCommand).arg(errors) );
}

QT_END_NAMESPACE
