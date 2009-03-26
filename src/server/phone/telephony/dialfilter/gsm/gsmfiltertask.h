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

#ifndef GSMDIALFILTER_H
#define GSMDIALFILTER_H

#include "abstractdialfilter.h"
class GSMDialFilterPrivate;
class GSMDialFilter : public AbstractDialFilter
{
    Q_OBJECT
public:
    explicit GSMDialFilter( QObject *parent = 0 );
    virtual ~GSMDialFilter();

    AbstractDialFilter::Action filterInput( const QString &input,
            bool sendPressed = false, bool takeNoAction = false );

    QByteArray type() const;
private:
    GSMDialFilterPrivate *d;
};

#endif
