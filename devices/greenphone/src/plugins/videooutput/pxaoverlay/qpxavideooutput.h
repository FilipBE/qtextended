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

#ifndef QPXAVIDEOOUTPUT_H
#define QPXAVIDEOOUTPUT_H

#include "qabstractvideooutput.h"
#include "qvideooutputfactory.h"
#include <QObject>


class QDirectPainter;
class QPxaVideoOutputPrivate;
class QScreen;

class QTOPIAMEDIA_EXPORT QPxaVideoOutput :public QAbstractVideoOutput
{
    Q_OBJECT

public:
    QPxaVideoOutput( QScreen *screen = 0, QObject *parent = 0 );
    virtual ~QPxaVideoOutput();

    virtual QVideoFormatList supportedFormats();
    virtual QVideoFormatList preferredFormats();

    virtual bool isAcceleratedScaleSupported() { return false; }

    virtual void doRenderFrame( const QVideoFrame& frame );

protected:
    virtual void geometryChanged();

private:
    QtopiaVideo::VideoRotation effectiveRotation() const;
    QPxaVideoOutputPrivate *d;
};


class QTOPIAMEDIA_EXPORT QPxaVideoOutputFactory :
    public QObject,
    public QVideoOutputFactory
{
    Q_OBJECT
    Q_INTERFACES(QVideoOutputFactory)
public:
    QPxaVideoOutputFactory();
    virtual ~QPxaVideoOutputFactory();

    virtual QVideoFormatList supportedFormats();
    virtual QVideoFormatList preferredFormats();

    virtual QAbstractVideoOutput* create(  QScreen *screen, QObject *parent = 0 );
};

#endif

