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

#ifndef E2_TELEPHONYBAR_H
#define E2_TELEPHONYBAR_H

#include <QWidget>
#include <QBrush>
#include <QPixmap>
#include <QValueSpaceItem>
#include <QString>

class E2TelephonyBar : public QWidget
{
Q_OBJECT
public:
    E2TelephonyBar(QWidget *parent = 0, Qt::WFlags = 0);

public slots:
    void batteryChanged();
    void signalChanged();
    void timeChanged();

private:
    virtual void paintEvent(QPaintEvent *);

    QPixmap fillBrush;
    QPixmap batPix;
    QPixmap signalPix;
    QPixmap cBatPix;
    QPixmap cSignalPix;
    QString cTime;
    QValueSpaceItem time;
    QValueSpaceItem battery;
    QValueSpaceItem signal;
};

#endif
