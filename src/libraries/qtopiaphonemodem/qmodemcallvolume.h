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

#ifndef QMODEMCALLVOLUME_H
#define QMODEMCALLVOLUME_H

#include <qcallvolume.h>

class QModemService;
class QAtResult;
class QModemCallVolumePrivate;

class QTOPIAPHONEMODEM_EXPORT QModemCallVolume : public QCallVolume
{
    Q_OBJECT
public:
    explicit QModemCallVolume( QModemService *service );
    ~QModemCallVolume();

protected:
    virtual bool hasDelayedInit() const;

public slots:
    virtual void setSpeakerVolume( int volume );
    virtual void setMicrophoneVolume( int volume );

protected slots:
    void callVolumesReady();

private slots:
    void initialize();
    void vgrRangeQuery(bool ok, const QAtResult &result);
    void vgrQuery(bool ok, const QAtResult &result);

    void vgtRangeQuery(bool ok, const QAtResult &result);
    void vgtQuery(bool ok, const QAtResult &result);

private:
    QModemCallVolumePrivate *d;
};

#endif
