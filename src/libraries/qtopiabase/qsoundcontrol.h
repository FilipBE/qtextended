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

#ifndef QSOUNDCONTROL_H
#define QSOUNDCONTROL_H

#include <QtGui>

#ifndef QT_NO_COP
#include <qcopchannel_qws.h>
#endif

#include <qtopiaglobal.h>

class QTOPIABASE_EXPORT QSoundControl : public QObject
{
    Q_OBJECT
public:
    explicit QSoundControl( QSound* sound, QObject* parent = 0 );

    void setVolume( int volume );
    int volume() const { return m_volume; }

    enum Priority { Default, RingTone };

    void setPriority( Priority priority );
    Priority priority() const { return m_priority; }

    QSound* sound() const { return m_sound; }

signals:
    // Sound has finished playing
    void done();

#ifndef QT_NO_COP
private slots:
    void processMessage( const QString& msg, const QByteArray& data );
#endif

private:
    QSound *m_sound;
    QUuid m_id;

    int m_volume;
    Priority m_priority;

#ifndef QT_NO_COP
    QCopChannel *m_channel;
#endif
};

#endif
