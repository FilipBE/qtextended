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

// !!! IMPORTANT !!!
// This interface is still experimental and subject to change.

#ifndef QWINDOWDECORATIONINTERFACE_H
#define QWINDOWDECORATIONINTERFACE_H

#include <QFactoryInterface>
#include <qdecoration_qws.h>

#include <qtopiaglobal.h>

class QWidget;

class QTOPIA_EXPORT QWindowDecorationInterface
{

        public:

    virtual ~QWindowDecorationInterface();

    struct WindowData {
        const QWidget *window;
        QRect rect;
        QPalette palette;
        QString caption;
        enum Flags { Maximized=0x01, Dialog=0x02, Active=0x04 };
        quint32 flags;
        quint32 reserved;
    };

    enum Metric { TitleHeight, LeftBorder, RightBorder, TopBorder, BottomBorder, OKWidth, CloseWidth, HelpWidth, MaximizeWidth, CornerGrabSize };
    virtual int metric( Metric m, const WindowData * ) const;

    enum Area { Border, Title, TitleText };
    virtual void drawArea( Area a, QPainter *, const WindowData * ) const;

    enum Button { OK, Close, Help, Maximize };
    virtual void drawButton( Button b, QPainter *, const WindowData *, int x, int y, int w, int h, QDecoration::DecorationState ) const;

    virtual QRegion mask( const WindowData * ) const;

    virtual QString name() const = 0;
    virtual QPixmap icon() const = 0;
};

struct QTOPIA_EXPORT QWindowDecorationFactoryInterface : public QFactoryInterface
{
    virtual QWindowDecorationInterface *decoration(const QString &key) = 0;
};

#define QWindowDecorationFactoryInterface_iid "com.trolltech.Qtopia.QWindowDecorationFactoryInterface"
Q_DECLARE_INTERFACE(QWindowDecorationFactoryInterface, QWindowDecorationFactoryInterface_iid)

class QTOPIA_EXPORT QWindowDecorationPlugin : public QObject, public QWindowDecorationFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWindowDecorationFactoryInterface:QFactoryInterface)
public:
    explicit QWindowDecorationPlugin(QObject *parent=0);
    virtual ~QWindowDecorationPlugin();

    virtual QStringList keys() const = 0;
    virtual QWindowDecorationInterface *decoration(const QString &key) = 0;
};

#endif
