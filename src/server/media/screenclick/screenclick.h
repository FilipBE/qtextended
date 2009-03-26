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

#ifndef SCREENCLICK_H
#define SCREENCLICK_H

#include "qtopiaserverapplication.h"

class ScreenClick : public QObject,
                    public QtopiaServerApplication::QWSEventFilter
{
Q_OBJECT
public:
    ScreenClick();
    virtual ~ScreenClick();

protected:
    virtual bool qwsEventFilter( QWSEvent * );
    virtual void screenClick(bool) = 0;

private slots:
    void sysMessage(const QString& message, const QByteArray &data);

private:
    void readSettings();

    bool m_clickenabled;
    bool m_up;
};

#endif
