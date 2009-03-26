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

#ifndef QATCHATSCRIPT_H
#define QATCHATSCRIPT_H

#include <qatchat.h>
#include <qatresult.h>

class QAtChatScriptPrivate;

class QTOPIACOMM_EXPORT QAtChatScript : public QObject
{
    Q_OBJECT
public:
    explicit QAtChatScript( QAtChat *atchat, QObject *parent = 0 );
    ~QAtChatScript();

    int totalCommands() const;
    int successfulCommands() const;

public slots:
    void runChatFile( const QString& filename );
    void runChat( const QString& chatScript );
    void runChat( const QStringList& commands );
    void stop();

signals:
    void done( bool ok, const QAtResult& result );

private slots:
    void commandDone( bool ok, const QAtResult& result );

private:
    QAtChatScriptPrivate *d;

    void sendNext();
    QString expandEscapes( const QString& value );
    void word( const QString& value );
};

#endif
