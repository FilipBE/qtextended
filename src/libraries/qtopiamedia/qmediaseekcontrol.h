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

#ifndef QMEDIASEEKCONTROL_H
#define QMEDIASEEKCONTROL_H

#include <QObject>

#include <qtopiaglobal.h>

#include "media.h"


class QMediaContent;


class QMediaSeekControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaSeekControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaSeekControl(QMediaContent* mediaContent);
    ~QMediaSeekControl();

    static QString name();

public slots:
    void seek(quint32 position, QtopiaMedia::Offset offset = QtopiaMedia::Beginning);
    void jumpForward(quint32 ms);
    void jumpBackwards(quint32 ms);
    void seekToStart();
    void seekToEnd();
    void seekForward();
    void seekBackward();

signals:
    void valid();
    void invalid();

    void positionChanged(quint32 ms);

private:
    Q_DISABLE_COPY(QMediaSeekControl);

    QMediaSeekControlPrivate*    d;
};

#endif

