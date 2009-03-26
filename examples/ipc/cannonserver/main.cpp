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

#include <QtopiaApplication>
#include <QDebug>
#include <QtopiaIpcAdaptor>
#include <QString>
#include <QtGlobal>
#include <cstdlib>

class CannonListener : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    CannonListener(QObject *parent = 0);

public slots:
    void shootCannon(int);

signals:
    void missed();
    void hit();
};

CannonListener::CannonListener(QObject *parent)
    : QtopiaIpcAdaptor("QPE/CannonExample", parent)
{
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

void CannonListener::shootCannon(int power)
{
    int pwr = power % 100;
    int roll = qrand() % 100;

    if (pwr >= roll)
        emit hit();
    else
        emit missed();

    deleteLater();
}

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);

    CannonListener *listener = new CannonListener;
    app.registerRunningTask("CannonListener", listener);

    app.exec();
}

#include "main.moc"
