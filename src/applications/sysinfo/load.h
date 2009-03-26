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

#ifndef LOAD_H
#define LOAD_H

#include <QWidget>

/*
  Little load meter
*/
class Load : public QWidget {
    Q_OBJECT
public:
    Load( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~Load();

protected:
    void paintEvent( QPaintEvent *ev );

private slots:
    void timeout();

private:
    int points;
    double *userLoad;
    double *systemLoad;
    double maxLoad;
    struct timeval last;
    int lastUser;
    int lastUsernice;
    int lastSys;
    int lastIdle;
    bool first;
};

class LoadInfo : public QWidget
{
    Q_OBJECT
public:
    LoadInfo( QWidget *parent = 0, Qt::WFlags f = 0 );

private:
    QPixmap makeLabel( const QColor &col );
    QString getCpuInfo();
private slots:
    void init();
};

#endif
