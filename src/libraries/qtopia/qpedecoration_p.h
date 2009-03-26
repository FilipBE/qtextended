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

#ifndef QPEDECORATION_P_H
#define QPEDECORATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qconfig.h>
#include <qtopiaglobal.h>
#include <qdecorationdefault_qws.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qpointer.h>
#include <qwindowdecorationinterface.h>

#ifndef QT_NO_QWS_QPE_WM_STYLE

class QTimer;
#include <qwidget.h>

class QTOPIA_EXPORT QtopiaDecoration : public QDecorationDefault
{
public:
    QtopiaDecoration();
    explicit QtopiaDecoration( const QString &plugin );
    virtual ~QtopiaDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, int region);
    virtual int regionAt(const QWidget *w, const QPoint &point);
    virtual bool paint(QPainter *, const QWidget *, int region, DecorationState state);

    using QDecorationDefault::paintButton; // Don't hide these symbols!
    virtual void paintButton(QPainter *, const QWidget *, int region, int state);

    virtual void regionClicked(QWidget *widget, int region);
    virtual void buildSysMenu( QWidget *, QMenu *menu );

protected:
    void help(QWidget *);
    virtual int getTitleHeight(const QWidget *);

private:
    void windowData( const QWidget *w, QWindowDecorationInterface::WindowData &wd ) const;

    bool helpExists() const;

protected:
    QImage imageOk;
    QImage imageClose;
    QImage imageHelp;
    QString helpFile;
    bool helpexists;
    QRect desktopRect;
};

#endif // QT_NO_QWS_QPE_WM_STYLE

#endif
