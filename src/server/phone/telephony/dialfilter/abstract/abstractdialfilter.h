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

#ifndef ABSTRACTDIALFILTER_H
#define ABSTRACTDIALFILTER_H

#include "qtopiaserverapplication.h"
#include <QObject>

class AbstractDialFilter : public QObject
{
    Q_OBJECT
public:
    AbstractDialFilter( QObject * parent = 0)
        : QObject(parent) {}
    virtual ~AbstractDialFilter() {};

    enum Action {
        Reject = 0,
        Continue,
        DialIfNoMoreDigits,
        Dialtone,
        DialImmediately,
        ActionTaken
    };

    virtual AbstractDialFilter::Action filterInput( const QString& input,
            bool sendPressed = false, bool takeNoAction = false ) = 0;

    virtual QByteArray type() const = 0;

    static AbstractDialFilter* defaultFilter();
};
QTOPIA_TASK_INTERFACE(AbstractDialFilter);

#endif
