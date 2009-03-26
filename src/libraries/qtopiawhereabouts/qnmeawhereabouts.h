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

#ifndef QNMEAWHEREABOUTS_H
#define QNMEAWHEREABOUTS_H

#include "qwhereabouts.h"
#include <qtopiaglobal.h>

class QNmeaWhereaboutsPrivate;
class QIODevice;

class QTOPIAWHEREABOUTS_EXPORT QNmeaWhereabouts : public QWhereabouts
{
    Q_OBJECT
public:
    enum UpdateMode {
        InvalidMode,
        RealTimeMode,
        SimulationMode
    };

    explicit QNmeaWhereabouts(QObject *parent = 0);
    explicit QNmeaWhereabouts(UpdateMode updateMode, QObject *parent = 0);
    ~QNmeaWhereabouts();

    void setSourceDevice(QIODevice *source);
    QIODevice *sourceDevice() const;

    void setUpdateMode(UpdateMode mode);
    UpdateMode updateMode() const;

public slots:
    virtual void startUpdates();
    virtual void stopUpdates();
    virtual void requestUpdate();

    void newDataAvailable();

private:
    friend class QNmeaWhereaboutsPrivate;
    QNmeaWhereaboutsPrivate *d;
    Q_DISABLE_COPY(QNmeaWhereabouts)
};

#endif
