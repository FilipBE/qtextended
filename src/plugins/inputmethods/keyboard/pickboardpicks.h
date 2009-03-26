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
#ifndef PICKBOARDPICKS_H
#define PICKBOARDPICKS_H

#include <qtopiaglobal.h>
#include <qframe.h>
#include <qlist.h>

// Internal stuff...
#include "pickboardcfg.h"

class PickboardPicks : public QFrame {
    Q_OBJECT
public:
    PickboardPicks(QWidget* parent=0, Qt::WFlags f=0);
    ~PickboardPicks();
    QSize sizeHint() const;
    void initialise();
    void setMode(int);
    int currentMode() const { return mode; }

    void mousePressEvent(QMouseEvent*);

    void resetState();

public slots:
    void doMenu();

protected:
    void drawContents( QPainter * );
    void mouseDoubleClickEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

protected:
    int mode;
    QList<PickboardConfig*> configs;

private:
    PickboardConfig* config() { return configs.at(mode); }
};


#endif
