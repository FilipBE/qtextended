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
#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>

class QPushButton;

class ConnectionStatusWidget : public QWidget
{
    Q_OBJECT
public:
    ConnectionStatusWidget( QWidget *parent = 0 );
    ~ConnectionStatusWidget();

private slots:
    void setState( int state );
    void connectionMessage( const QString &message, const QByteArray &data );
    void statusClicked();
    void hintClicked();

private:
    void showEvent( QShowEvent *e );

    QString section;
    QPushButton *statusBtn;
    QPushButton *hintBtn;
    int mState;
};

#endif
