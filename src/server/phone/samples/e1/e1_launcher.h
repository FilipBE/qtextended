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

#ifndef E1_LAUNCHER_H
#define E1_LAUNCHER_H

#include <QWidget>
#include <QPixmap>
#include "qabstractserverinterface.h"
#include <qvaluespace.h>
#include <custom.h>

class QExportedBackground;
class E1Header;
class E1PhoneBrowser;
class E1Dialer;

class E1ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E1ServerInterface(QWidget *parent = 0, Qt::WFlags flags = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void showEvent(QShowEvent *);

private slots:
    void wallpaperChanged();
    void showDialer( const QString& msg, const QByteArray& );
    void messageCountChanged();

private:
    QPixmap m_wallpaper;
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QExportedBackground *m_eb;
#endif
    E1Header *m_header;
    E1PhoneBrowser *m_browser;
    E1Dialer* m_dialer;
    QValueSpaceItem m_newMessages;
};

#endif
