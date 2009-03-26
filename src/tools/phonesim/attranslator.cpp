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

#include "attranslator.h"

AtTranslator::AtTranslator(const QString& specFile)
    : gsmSpec(specFile)
{
}

QString AtTranslator::translateCommand( const QString& data )
{
    QString command;
    QString parameters;

    if( data.contains("?") ){
        command = data.left( data.indexOf("?")+1 );
    }else if( data.contains("=") ){
        command = data.left( data.indexOf("=")+1 );
        parameters = data.right( data.indexOf("=")+1 );
    }else{
        command = data;
    }
    QString answer = gsmSpec.getProfile(command);
    if(!gsmSpec.validateCommand(command, parameters) )
        answer = answer.append("\n").append("Parameters not valid, expected: ").append(gsmSpec.getParameterFormat(command).join("||"));
    return answer;
}

QString AtTranslator::translateResponse( const QString& data )
{
    Q_UNUSED( data );
    return "todo";
}

void AtTranslator::resetSpecification(const QString& filePath)
{
    gsmSpec.resetDictionary(filePath);
}








