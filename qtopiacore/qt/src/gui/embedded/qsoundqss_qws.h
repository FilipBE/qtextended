/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QSOUNDQSS_QWS_H
#define QSOUNDQSS_QWS_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SOUND

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtGui/qwssocket_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if defined(QT_NO_NETWORK) || defined(QT_NO_DNS)
#define QT_NO_QWS_SOUNDSERVER
#endif

#ifndef Q_OS_MAC

class QWSSoundServerPrivate;

class Q_GUI_EXPORT QWSSoundServer : public QObject {
    Q_OBJECT
public:
    explicit QWSSoundServer(QObject *parent=0);
    ~QWSSoundServer();
    void playFile( int id, const QString& filename );
    void stopFile( int id );
    void pauseFile( int id );
    void resumeFile( int id );
    
Q_SIGNALS:
    void soundCompleted( int );
    
private Q_SLOTS:
    void translateSoundCompleted( int, int );

private:
    QWSSoundServerPrivate* d;
};

#ifndef QT_NO_QWS_SOUNDSERVER
class Q_GUI_EXPORT QWSSoundClient : public QWSSocket {
    Q_OBJECT
public:

    enum SoundFlags {
	Priority = 0x01,
	Streaming = 0x02  // currently ignored, but but could set up so both Raw and non raw can be done streaming or not.
    };
    enum DeviceErrors {
	ErrOpeningAudioDevice = 0x01,
	ErrOpeningFile = 0x02,
	ErrReadingFile = 0x04
    };
    explicit QWSSoundClient(QObject* parent=0);
    ~QWSSoundClient( );
    void reconnect();
    void play( int id, const QString& filename );
    void play( int id, const QString& filename, int volume, int flags = 0 );
    void playRaw( int id, const QString&, int, int, int, int flags = 0 );

    void pause( int id );
    void stop( int id );
    void resume( int id );
    void setVolume( int id, int left, int right );
    void setMute( int id, bool m );
    
    // to be used by server only, to protect phone conversation/rings.
    void playPriorityOnly(bool);

    // If silent, tell sound server to release audio device
    // Otherwise, allow sound server to regain audio device
    void setSilent(bool);
    
Q_SIGNALS:
    void soundCompleted(int);
    void deviceReady(int id);
    void deviceError(int id, QWSSoundClient::DeviceErrors);

private Q_SLOTS:
    void tryReadCommand();
    void emitConnectionRefused();
    
private:
    void sendServerMessage(QString msg);
};

class QWSSoundServerSocket : public QWSServerSocket {
    Q_OBJECT

public:
    explicit QWSSoundServerSocket(QObject *parent=0);
public Q_SLOTS:    
    void newConnection();

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QWSSoundServerSocket(QObject *parent, const char *name);
#endif

Q_SIGNALS:
    void playFile(int, int, const QString&);
    void playFile(int, int, const QString&, int, int);
    void playRawFile(int, int, const QString&, int, int, int, int);
    void pauseFile(int, int);
    void stopFile(int, int);
    void resumeFile(int, int);
    void setVolume(int, int, int, int);
    void setMute(int, int, bool);

    void stopAll(int);

    void playPriorityOnly(bool);

    void setSilent(bool);

    void soundFileCompleted(int, int);
    void deviceReady(int, int);
    void deviceError(int, int, int);
};
#endif

#endif // Q_OS_MAC

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_SOUND

#endif // QSOUNDQSS_QWS_H
