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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef QTUITESTNAMESPACE_H
#define QTUITESTNAMESPACE_H

#include <qtuitestglobal.h>

#include <QEvent>
#include <QObject>
#include <Qt>

class QPoint;

// syncqtopia header QtUiTest
namespace QtUiTest
{
    enum InputOption {
        NoOptions      = 0x0,
        DemoMode       = 0x1,
        KeyRepeat      = 0x2
    };

    enum WidgetType {
        InputMethod,
        SoftMenu,
        OptionsMenu,
        TabBar,
        Launcher,
        HomeScreen,
        Focus,
        CallManager
    };

    enum Key {
#ifdef Q_WS_QWS
        Key_Activate = Qt::Key_Select,
#else
        Key_Activate = Qt::Key_Enter,
#endif

#ifdef Q_WS_QWS
        Key_Select   = Qt::Key_Select,
#else
        Key_Select   = Qt::Key_Space,
#endif

#ifdef Q_WS_QWS
        Key_ActivateButton   = Qt::Key_Select
#else
        Key_ActivateButton   = Qt::Key_Space
#endif
    };

    QTUITEST_EXPORT void setInputOption(InputOption,bool = true);
    QTUITEST_EXPORT bool testInputOption(InputOption);

    QTUITEST_EXPORT QString errorString();
    QTUITEST_EXPORT void setErrorString(QString const&);

    QTUITEST_EXPORT void mousePress  (QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT void mouseRelease(QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT void mouseClick  (QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseClick  (QObject*,QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseClick  (QObject*,QByteArray const&,QPoint const&,
            Qt::MouseButtons = Qt::LeftButton, InputOption = NoOptions);

    QTUITEST_EXPORT void keyPress  (int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT void keyRelease(int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT void keyClick  (int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool keyClick  (QObject*,int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool keyClick  (QObject*,QByteArray const&,int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);

    QTUITEST_EXPORT bool mousePreferred();

    QTUITEST_EXPORT int maximumUiTimeout();

    QTUITEST_EXPORT Qt::Key asciiToKey(char);
    QTUITEST_EXPORT Qt::KeyboardModifiers asciiToModifiers(char);

    QTUITEST_EXPORT QObject* findWidget(WidgetType);

    QTUITEST_EXPORT QObject* testWidget(QObject*,const char*);

    QTUITEST_EXPORT bool connectFirst   (const QObject*, const char*, const QObject*, const char*);
    QTUITEST_EXPORT bool disconnectFirst(const QObject*, const char*, const QObject*, const char*);

    template<class T> inline T qtuitest_cast_helper(QObject* object, T)
    {
        T ret;
        if ((ret = qobject_cast<T>(object))) {}
        else {
            ret = qobject_cast<T>(QtUiTest::testWidget(object,
                        static_cast<T>(0)->_q_interfaceName()));
        }
        return ret;
    }

    QTUITEST_EXPORT void wait(int);
    QTUITEST_EXPORT bool waitForSignal(QObject*, const char*, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);
    QTUITEST_EXPORT bool waitForEvent(QObject*, QEvent::Type, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);
    QTUITEST_EXPORT bool waitForEvent(QObject*, QList<QEvent::Type> const&, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);
};

template<class T> inline
T qtuitest_cast(const QObject* object)
{
    return QtUiTest::qtuitest_cast_helper<T>(const_cast<QObject*>(object),0);
}

#include <qtuitestwidgetinterface.h>

#endif

