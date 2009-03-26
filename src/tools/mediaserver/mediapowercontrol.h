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

#ifndef MEDIAPOWERCONTROL_H
#define MEDIAPOWERCONTROL_H

#include <QObject>
#include <QtopiaApplication>

namespace mediaserver
{

class MediaPowerControl : public QObject
{
    Q_OBJECT

public:
    MediaPowerControl(QObject* parent = 0);
    ~MediaPowerControl();

private slots:
    void activeSessionCount(int);

private:
    int m_activePlayingSessions;
    QtopiaApplication::PowerConstraint  m_powerConstraint;
};

}   // ns mediaserver

#endif
