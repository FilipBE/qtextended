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

#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QWidget>
#include <QHash>
#include <QFrame>

class QLabel;
class QFileSystem;

class QScrollArea;
class QNetworkDevice;
class QNetworkInterface;


class NetworkInfoView : public QWidget
{
    Q_OBJECT
public:
    NetworkInfoView( QWidget *parent=0);
    QSize sizeHint() const;

protected:
    void timerEvent(QTimerEvent*);

signals:
    void updated();

private slots:
    void updateNetInfo();
    void init();

private:
    QScrollArea *area;
};


class NetworkDeviceInfo : public QFrame
{
    Q_OBJECT
public:
    NetworkDeviceInfo( const QNetworkDevice*, QWidget *parent=0 );
    ~NetworkDeviceInfo();

public slots:

private:
    void getSizeString( double &size, QString &string );

;

    const QNetworkDevice *networkDevice;

};

#endif
