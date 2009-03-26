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

#ifndef QTOPIATIMER_H
#define QTOPIATIMER_H

#include <QTimer>
#include <qtopiaglobal.h>

class QtopiaTimerPrivate;
class QTOPIABASE_EXPORT  QtopiaTimer : public QObject
{
Q_OBJECT
public:
    QtopiaTimer(QObject *parent = 0);
    virtual ~QtopiaTimer();

    enum Type { Normal            = 0x0000,
                PauseWhenInactive = 0x0001 };

    Type type() const;

    int interval () const;
    bool isActive () const;
    void setInterval(int msec, QtopiaTimer::Type = Normal);

    bool isSingleShot() const;
    void setSingleShot(bool);

public slots:
    void start();
    void stop();
    void start(int msec, QtopiaTimer::Type = Normal);

signals:
    void timeout();

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void activeChanged();

private:
    void disable(bool);
    void enable();

    QtopiaTimerPrivate *d;
};

#endif
