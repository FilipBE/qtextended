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

#ifndef QMEDIAVIDEOCONTROL_H
#define QMEDIAVIDEOCONTROL_H

#include <QObject>

#include <qtopiaglobal.h>
#include <qtopiavideo.h>


class QWidget;
class QMediaContent;

class QMediaVideoControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaVideoControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaVideoControl(QMediaContent* mediaContent);
    ~QMediaVideoControl();

    QWidget* createVideoWidget(QWidget* parent = 0) const;

    static QString name();

public slots:
    void setVideoRotation(QtopiaVideo::VideoRotation);
    void setVideoScaleMode(QtopiaVideo::VideoScaleMode);

signals:
    void valid();
    void invalid();

    void videoTargetAvailable();
    void videoTargetRemoved();

private:
    Q_DISABLE_COPY(QMediaVideoControl);

    QMediaVideoControlPrivate*    d;
};

#endif
