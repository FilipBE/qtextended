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

#ifndef ATTRANSLATOR_H
#define ATTRANSLATOR_H

#include <QString>
#include <QMap>
#include <QStringList>
#include "gsmspec.h"

class AtTranslator
{

public:
    AtTranslator(const QString& specFile);
    QString translateCommand( const QString& );
    QString translateResponse( const QString& );
    void resetSpecification(const QString& );

private:
    GSMSpec gsmSpec;
};

#endif
