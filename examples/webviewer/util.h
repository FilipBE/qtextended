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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore/QObject>
#include <QtCore/QBasicTimer>
#include <QtCore/QTime>

class DesktopServices {

public:
    static QString storageLocation();

};

/*
    This class will call the save() slot on the parent object when the parent changes.
    It will wait several seconds after changed() to combining multiple changes and
    prevent continuous writing to disk.
  */
class AutoSave : public QObject {

Q_OBJECT

public:
    AutoSave(QObject *parent);
    ~AutoSave();
    void changed();
    void saveNow();

protected:
    void timerEvent(QTimerEvent *event);

private:
    QBasicTimer m_timer;
    QTime m_firstChange;

};

#endif
