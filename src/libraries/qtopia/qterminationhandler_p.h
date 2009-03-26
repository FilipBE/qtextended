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

#ifndef QTERMINATIONHANDLER_P_H
#define QTERMINATIONHANDLER_P_H

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

#include "qterminationhandler.h"

#include <QString>
#include <QtopiaServiceRequest>
#include <QDataStream>

struct QTerminationHandlerData
{
    QTerminationHandlerData()
    {
    }

    QTerminationHandlerData( const QString& n, const QString& t, const QString& bt, const QString& i, const QtopiaServiceRequest& a )
        : name(n), text(t), buttonText(bt), buttonIcon(i), action(a)
    {
    }

    QTerminationHandlerData( const QTerminationHandlerData& other ) {
        (*this) = other;
    }

    QTerminationHandlerData& operator=( const QTerminationHandlerData& other ) {
        name = other.name;
        text = other.text;
        buttonText = other.buttonText;
        buttonIcon = other.buttonIcon;
        action = other.action;
        return *this;
    }

    QString name;
    QString text;
    QString buttonText;
    QString buttonIcon;
    QtopiaServiceRequest action;
};

inline QDataStream& operator<<( QDataStream& ostream, const QTerminationHandlerData& data ) {
    ostream << data.name << data.text << data.buttonText << data.buttonIcon << data.action;
    return ostream;
}

inline QDataStream& operator>>( QDataStream& istream, QTerminationHandlerData& data ) {
    QString n, t, bt, bi;
    QtopiaServiceRequest a;
    /*
    istream >> n;
    istream >> t;
    istream >> bt;
    istream >> bi;
    istream >> a;
    */
    istream >> n >> t >> bt >> bi >> a;
    data.name = n;
    data.text = t;
    data.buttonText = bt;
    data.buttonIcon = bi;
    data.action = a;
    return istream;
}

#endif
