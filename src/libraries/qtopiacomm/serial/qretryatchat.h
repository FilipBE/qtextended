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

#ifndef QRETRYATCHAT_H
#define QRETRYATCHAT_H

#include <qatchat.h>

class QRetryAtChatPrivate;

class QTOPIACOMM_EXPORT QRetryAtChat : public QObject
{
    Q_OBJECT
public:
    QRetryAtChat( QAtChat *atchat, const QString& command, int numRetries,
                  int timeout = 1000, bool deleteAfterEmit = true,
                  QObject *parent = 0 );
    ~QRetryAtChat();

signals:
    void done( bool ok, const QAtResult& result );

private slots:
    void doneInternal( bool ok, const QAtResult& result );
    void timeout();

private:
    QRetryAtChatPrivate *d;
};

#endif
