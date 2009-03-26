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

#ifndef KEYFILTER_H
#define KEYFILTER_H

#include <QtCore>

namespace mediaplayer
{

class KeyFilter : public QObject
{
    Q_OBJECT

public:
    KeyFilter( QObject* subject, QObject* target, QObject* parent = 0 );

    void addKey( int key );

    bool eventFilter( QObject* o, QEvent* e );

private:
    QObject *m_target;
    QSet<int> m_keys;
};

}   // ns mediaplayer

#endif
