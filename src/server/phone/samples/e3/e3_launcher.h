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

#ifndef E3_LAUNCHER_H
#define E3_LAUNCHER_H

#include "qabstractserverinterface.h"
#include <QContent>
#include <QContentSet>
#include <QPair>
#include <QtopiaServiceRequest>
#include <QHash>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include "themebackground_p.h"

class LauncherView;
class ContextLabel;
class QExportedBackground;
class QSettings;
class QAbstractBrowserScreen;
class E3Today;
class ThemedView;
class QAbstractDialerScreen;
class QPhoneCall;
class QAbstractCallScreen;
class QUniqueId;
class QAbstractCallPolicyManager;
class ThemeControl;
class QAbstractHeader;
class QAbstractContextLabel;

class E3ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E3ServerInterface(QWidget *parent, Qt::WFlags flags);
signals:
    void themeLoaded();
protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

private slots:
    void acceptIncoming();
    void loadTheme();
    void launch(QContent);
    void received(const QString &, const QByteArray &);
    void showCallscreen();
    void requestDial(const QString &);
    void showDialer(const QString &);
#ifdef Q_WS_QWS
    void windowEvent(QWSWindow*,QWSServer::WindowEvent);
#endif

private:
    void header();
    void context();
    void quickApps();
    void idleContext();
    void showApps();

    typedef QPair<QString, QtopiaServiceRequest> ButtonBinding;
    ButtonBinding buttonBinding(QSettings &) const;

    LauncherView *m_view;
    QAbstractHeader *m_header;
    QAbstractContextLabel *m_context;
    QContentSet m_idleApps;
    QAbstractBrowserScreen *m_browser;
    QAbstractDialerScreen *dialer();
    QAbstractDialerScreen *m_dialer;
    QAbstractCallScreen *callscreen();
    QAbstractCallScreen *m_callscreen;
    E3Today *m_today;
    ThemedView *m_theme;
    ThemeBackground *m_tbackground;
    QWidget *m_titleSpacer;
    QAbstractCallPolicyManager *m_cell;

    typedef QHash<int, QtopiaServiceRequest> IdleKeys;
    IdleKeys m_idleKeys;
    ThemeControl *themeCtrl;
};

#endif
