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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QFrame>
#include <QMap>

class QFrame;
class QTimer;
class QHBoxLayout;
class QLabel;
class QString;

// QStatusBar sucks!
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    StatusBar( QWidget *parent = 0 );
    virtual ~StatusBar();

    void addWidget( QWidget *widget, int stretch = 0, bool permanent = false );
    void removeWidget( QWidget *widget );

public slots:
    void showMessage( const QString &message, int timeout = 0 );

signals:
    void clicked();

private:
    QMap<QWidget*,QFrame*> widgets;
    QLabel *statusLabel;
    QTimer *statusClearer;
    QHBoxLayout *layout;
};

#endif
