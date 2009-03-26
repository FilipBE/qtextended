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
#ifndef QMEDIARTPSESSIONENGINE_H
#define QMEDIARTPSESSIONENGINE_H

#include <qtopiaglobal.h>
#include <qmediartpsession.h>


class QTOPIAMEDIA_EXPORT QMediaRtpEngine : public QObject
{
    Q_OBJECT
public:
    virtual ~QMediaRtpEngine();

    virtual QList<QMediaRtpPayload> supportedInboundPayloads(QMediaRtpStream::Type type) = 0;
    virtual QList<QMediaRtpPayload> supportedOutboundPayloads(QMediaRtpStream::Type type) = 0;

    virtual int streamCount() const = 0;
    virtual QMediaRtpStream *stream(int index) const = 0;
    virtual QMediaRtpStream *addStream(
            QMediaRtpStream::Type type, QMediaRtpStream::Direction direction) = 0;
    virtual void removeStream(QMediaRtpStream *stream) = 0;
};

class QTOPIAMEDIA_EXPORT QMediaRtpEngineFactory
{
public:
    virtual ~QMediaRtpEngineFactory();

    virtual QMediaRtpEngine *createRtpEngine() = 0;
};

Q_DECLARE_INTERFACE(QMediaRtpEngineFactory, "com.trolltech.qtopia.QMediaRtpEngineFactory/1.0");


#endif
