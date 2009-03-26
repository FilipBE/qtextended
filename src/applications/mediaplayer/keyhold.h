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

#ifndef KEYHOLD_H
#define KEYHOLD_H

#include <QtCore>
#include <qtopiaglobal.h>

class KeyHold : public QObject
{
    Q_OBJECT
public:
    KeyHold( int key, int keyHold, int threshold, QObject* target, QObject* parent = 0 );

    bool eventFilter( QObject* o, QEvent* e );

private slots:
    void generateKeyHoldPress();

private:
    int m_key;
    int m_keyHold;

    int m_threshold;
    QTimer *m_countdown;

    QObject *m_target;
};

#endif
