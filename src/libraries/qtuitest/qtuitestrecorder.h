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

#ifndef QTUITESTRECORDER_H
#define QTUITESTRECORDER_H

#include <QObject>
#include <qtuitestglobal.h>

class QtUiTestRecorderPrivate;
class QtUiTestWidgets;

class QTUITEST_EXPORT QtUiTestRecorder : public QObject
{
    Q_OBJECT

public:
    QtUiTestRecorder(QObject* =0);
    ~QtUiTestRecorder();

    static void emitKeyEvent(int,int,bool,bool);
    static void emitMouseEvent(int,int,int);

protected:
    virtual void connectNotify(char const*);
    virtual void disconnectNotify(char const*);

signals:
    void gotFocus(QObject*);
    void activated(QObject*);
    void stateChanged(QObject*,int);
    void entered(QObject*,QVariant const&);
    void selected(QObject*,QString const&);

    void keyEvent(int,int,bool,bool);
    void mouseEvent(int,int,int);

private:
    static void connectToAll(QObject*);
    friend class QtUiTestWidgets;
    friend class QtUiTestRecorderPrivate;
};

#endif

