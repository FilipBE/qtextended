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

#include "util.h"

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtDebug>

QString DesktopServices::storageLocation()
{
#if defined(Q_WS_MAC)
   QString path = QDir::homePath() + QLatin1String("/Library/") + QCoreApplication::applicationName();
#else
   QString path = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
#endif
    QDir dir(path);
    if (!dir.exists())
        dir.mkdir(path);
    return path;
}

#define AUTOSAVE_IN  1000 * 3  // seconds
#define MAXWAIT      1000 * 15 // seconds

AutoSave::AutoSave(QObject *parent) : QObject(parent)
{
    Q_ASSERT(parent);
}

AutoSave::~AutoSave()
{
    if (m_timer.isActive())
        qWarning() << "AutoSave: still active when destroyed, changes not saved.";
}

void AutoSave::changed()
{
    if (m_firstChange.isNull())
        m_firstChange.start();

    if (m_firstChange.elapsed() > MAXWAIT) {
        saveNow();
    } else {
        m_timer.start(AUTOSAVE_IN, this);
    }
}

void AutoSave::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        saveNow();
    } else {
        QObject::timerEvent(event);
    }
}

void AutoSave::saveNow()
{
    if (!m_timer.isActive())
        return;
    m_timer.stop();
    m_firstChange = QTime();
    if (!QMetaObject::invokeMethod(parent(), "save", Qt::DirectConnection)) {
        qWarning() << "AutoSave: error invoking slot save() on parent";
    }
}

