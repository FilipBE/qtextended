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

#ifndef RDSPROGRAMINFO_H
#define RDSPROGRAMINFO_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include "radioband.h"

class RdsProgramInfoPrivate;
class RdsGroup;

class RdsProgramInfo : public QObject
{
    Q_OBJECT
public:
    RdsProgramInfo( QObject *parent = 0 );
    ~RdsProgramInfo();

    void clear();
    void addGroup( const RdsGroup& group );

    int piCode() const;
    int ptyCode() const;
    bool trafficProgram() const;
    bool trafficAnnouncement() const;
    bool isMusic() const;
    bool isSpeech() const;
    QString programName() const;
    QString programType() const;
    QString radioText() const;
    QList<RadioBand::Frequency> alternateFrequencies() const;

signals:
    void timeNotification( const QDateTime& time, int zone );

private:
    RdsProgramInfoPrivate *d;
};

#endif
