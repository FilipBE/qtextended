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

#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include <qtopiaglobal.h>
#include <QList>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QWaitCondition>

class QTOPIACOMM_EXPORT ScriptThread : public QThread {
    Q_OBJECT
public:
    explicit ScriptThread( QObject * parent = 0 );
    ~ScriptThread();

    void addScriptToRun( const QString& scriptPath, const QStringList& parameter );
    int remainingTasks() const;
    void run();
signals:
    void scriptDone();

private:
    QList<QString> scripts;
    QList<QStringList> params;
    bool quit;
    QWaitCondition waitCond;
    mutable QMutex mutex;
};

#endif
