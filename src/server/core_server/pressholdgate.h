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

#ifndef PRESSHOLDGATE_H
#define PRESSHOLDGATE_H

#include <QObject>
class QDeviceButton;

class PressHoldGate : public QObject {
    Q_OBJECT
public:
    PressHoldGate(QObject* parent)
        : QObject( parent ), held_key(0), held_tid(0), hardfilter(false) { }
    PressHoldGate(const QString& contxt, QObject* parent)
        : QObject( parent ), held_key(0), held_tid(0), context(contxt), hardfilter(false) { }
    bool filterKey(int keycode, bool pressed, bool pressable, bool holdable, bool releasable);
    bool filterDeviceButton(int keycode, bool pressed, bool isautorepeat);
    void setHardFilter(bool y) { hardfilter=y; }
signals:
    void activate(int keycode, bool held, bool pressed);
private:
    void timerEvent(QTimerEvent*);
    int held_key;
    int held_tid;
    QString context;
    bool hardfilter;
};

#endif
