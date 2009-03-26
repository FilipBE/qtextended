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

#ifndef QWHEREABOUTS_H
#define QWHEREABOUTS_H

#include <qtopiaglobal.h>
#include <QObject>

#include <QWhereaboutsUpdate>

class QWhereaboutsPrivate;
class QWhereaboutsUpdate;

class QTOPIAWHEREABOUTS_EXPORT QWhereabouts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval);
public:
    enum UpdateMethod {
        AssistedUpdate = 0x01,
        NetworkBasedUpdate = 0x02,
        TerminalBasedUpdate = 0x04
    };
    Q_DECLARE_FLAGS(UpdateMethods, UpdateMethod)

    enum State {
        NotAvailable,
        Initializing,
        Available,
        PositionFixAcquired
    };

    explicit QWhereabouts(UpdateMethods updateMethods = 0, QObject *parent = 0);
    virtual ~QWhereabouts();

    UpdateMethods updateMethods() const;
    State state() const;

    void setUpdateInterval(int interval);
    int updateInterval() const;

    QWhereaboutsUpdate lastUpdate() const;

public slots:
    void startUpdates(int msec);
    virtual void startUpdates() = 0;
    virtual void stopUpdates() = 0;

    virtual void requestUpdate() = 0;

protected:
    void setState(State state);
    void emitUpdated(const QWhereaboutsUpdate &update);

signals:
    void updated(const QWhereaboutsUpdate &update);
    void stateChanged(QWhereabouts::State state);

private:
    QWhereaboutsPrivate *d;
    Q_DISABLE_COPY(QWhereabouts)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWhereabouts::UpdateMethods);

#endif
