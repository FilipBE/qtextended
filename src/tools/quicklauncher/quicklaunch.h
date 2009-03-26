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
#ifndef QUICKLAUNCH_H
#define QUICKLAUNCH_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QPointer>
#include <QWidget>

class QtopiaChannel;
#ifndef SINGLE_EXEC
class QPluginManager;
class QApplicationFactoryInterface;
#endif
class QtopiaApplication;
class QEventLoop;

class QuickLauncher : public QObject
{
    Q_OBJECT
public:
    QuickLauncher();

    static void exec( int argc, char **argv );

private slots:
    void message(const QString &msg, const QByteArray & data);

private:
    void doQuickLaunch( QStringList &argList );

private:
    QtopiaChannel *qlChannel;

public:
#ifndef SINGLE_EXEC
    static QPluginManager *loader;
    static QObject *appInstance;
    static QApplicationFactoryInterface *appIface;
#endif

    static QtopiaApplication *app;
    static QPointer<QWidget> mainWindow;
    static bool validExitLoop;
    static bool needsInit;
    static QEventLoop *eventLoop;

#ifdef QTOPIA_SETPROC_ARGV0
    static char **argv0;
    static int argv_lth;
#endif
};

#endif
