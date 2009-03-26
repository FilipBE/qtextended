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

class CannonResultListener : public QObject
{
    Q_OBJECT

public:
    CannonResultListener(QObject *parent = 0);

public slots:
    void missed();
    void hit();
};

CannonResultListener::CannonResultListener(QObject *parent)
    : QObject(parent)
{

}

void CannonResultListener::missed()
{
    qDebug() << "Cannon missed.";
    deleteLater();
}

void CannonResultListener::hit()
{
    qDebug() << "Cannon HIT!!!";
    deleteLater();
}

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);

    if (argc != 2) {
        qDebug() << "Please specify the cannonPower argument";
        return 0;
    }

    CannonResultListener *listener = new CannonResultListener;
    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor("QPE/CannonExample");
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(missed()), listener, SLOT(missed()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(hit()), listener, SLOT(hit()));

    app.registerRunningTask("CannonResultListener", listener);

    adaptor->send(MESSAGE(shootCannon(int)), QString(argv[1]).toInt());

    app.exec();
    delete adaptor;
}

#include "main.moc"
