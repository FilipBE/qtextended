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
#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>

class TrayIconPrivate;

class QIcon;
class QMenu;
class QPoint;
class QAction;
class QPixmap;

class TrayIcon : public QObject
{
    friend class TrayIconPrivate;
    Q_OBJECT
public:
    TrayIcon( QObject *parent = 0 );
    virtual ~TrayIcon();

    bool installed();
    void show();
    void hide();

signals:
    void clicked();
    void sync();
    void quit();

private slots:
    void setState( int state );
    void lastWindowClosed();
    void connectionMessage( const QString &message, const QByteArray &data );

private:
    void popup( const QPoint &p );
    void iconClicked();

    QMenu *popupMenu;
    QAction *openAction;
    QAction *syncAction;
    int mState;

    TrayIconPrivate *d;
    void setIcon( const QPixmap &pm );
};

#endif
