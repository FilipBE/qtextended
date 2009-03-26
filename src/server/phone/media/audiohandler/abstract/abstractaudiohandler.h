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

#ifndef ABSTRACTAUDIOHANDLER_H
#define ABSTRACTAUDIOHANDLER_H

#include <QObject>
#include <QByteArray>
#include "qtopiaserverapplication.h"

class AbstractAudioHandlerPrivate;
class QAudioStateConfiguration;
class QtopiaIpcAdaptor;

class AbstractAudioHandler : public QObject
{
    Q_OBJECT
public:
    AbstractAudioHandler( QObject* parent = 0 );
    virtual ~AbstractAudioHandler();

    virtual void activateAudio( bool enabled ) = 0;
    virtual void transferAudio( const QByteArray& profile ) = 0;
    virtual QByteArray audioType() = 0;

    QByteArray audioProfile() const;
    void setAudioProfile( const QByteArray& newAudioProfile );

    bool isInitialized() const;

    static QByteArray profileForKey( int key );
    static AbstractAudioHandler* audioHandler(const QByteArray& audioType);

signals:
    bool initialized();

protected:
    virtual void initialize() = 0;

    QtopiaIpcAdaptor* ipcAdaptor;
    QAudioStateConfiguration* audioConf;

private:
    AbstractAudioHandlerPrivate *d;
    Q_PRIVATE_SLOT( d, void _q_audioConfInitialized() );
    friend class AbstractAudioHandlerPrivate;
};

QTOPIA_TASK_INTERFACE(AbstractAudioHandler);

#endif
