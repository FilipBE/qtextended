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

#ifndef MESSAGEBOARD_H
#define MESSAGEBOARD_H

#include <QObject>
#include <QString>
#include "qtopiaserverapplication.h"

class MessageBoardPrivate;
class MessageBoard : public QObject
{
    Q_OBJECT
public:
    struct Note {
        QString text;
        QString pixmap;
        int id;
    };

    MessageBoard( QObject *parent = 0 );
    ~MessageBoard();

    int postMessage( const QString& pix, const QString& message, int priority = 10 );
    void clearMessage( int ident );
    bool isEmpty() const;
    Note message( int identifier  ) const;
    Note message() const;

signals:
    void boardUpdated();

private:
    MessageBoardPrivate *d;
};

QTOPIA_TASK_INTERFACE( MessageBoard );

#endif
