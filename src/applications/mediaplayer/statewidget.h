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

#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "playercontrol.h"

#include <qmediawidgets.h>

#include <QtGui>

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    StateWidget( PlayerControl* control, QWidget* parent = 0 );

signals:
    void playing();
    void paused();
    void stopped();

private slots:
    void setState( PlayerControl::State state );
    void setStopped();

protected:
    void keyPressEvent( QKeyEvent* e );
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

private:
    void togglePlaying();

    PlayerControl *m_control;
    QMediaStateLabel *m_label;

    QTimer *m_holdtimer;
};

#endif
