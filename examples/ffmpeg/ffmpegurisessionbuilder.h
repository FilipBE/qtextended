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


#ifndef __FFMPEG_URISESSIONBUILDER_H
#define __FFMPEG_URISESSIONBUILDER_H

#include <qobject.h>
#include <qmediasessionbuilder.h>

class QMediaCodecPlugin;

namespace ffmpeg
{

class Engine;
class UriSessionBuilderPrivate;

class UriSessionBuilder :
    public QObject,
    public QMediaSessionBuilder
{
    Q_OBJECT

public:
    UriSessionBuilder(Engine* engine);
    ~UriSessionBuilder();

    QString type() const;
    Attributes const& attributes() const;

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* serverSession);

private:
    UriSessionBuilderPrivate* d;
};

} // ns ffmpeg

#endif  // __FFMPEG_URISESSIONBUILDER_H


