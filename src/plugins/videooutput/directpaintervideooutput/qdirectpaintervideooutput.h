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

#ifndef QDIRECTPAINTERVIDEOOUTPUT_H
#define QDIRECTPAINTERVIDEOOUTPUT_H

#include <QObject>

#include "qabstractvideooutput.h"
#include "qvideooutputfactory.h"

class QDirectPainter;
class QDirectPainterVideoOutputPrivate;

class QTOPIAMEDIA_EXPORT QDirectPainterVideoOutput : public QAbstractVideoOutput
{
    Q_OBJECT

public:
    QDirectPainterVideoOutput( QScreen *screen, QObject *parent = 0 );
    virtual ~QDirectPainterVideoOutput();

    virtual QVideoFormatList supportedFormats();
    virtual QVideoFormatList preferredFormats();

    virtual bool isAcceleratedScaleSupported() { return false; }

    virtual void doRenderFrame( const QVideoFrame& frame );

private:
    QtopiaVideo::VideoRotation effectiveRotation() const;
    QVideoFrame::PixelFormat nativeScreenPixelFormat();
    QDirectPainterVideoOutputPrivate *d;
};


class QTOPIAMEDIA_EXPORT QDirectPainterVideoOutputFactory :
    public QObject,
    public QVideoOutputFactory
{
    Q_OBJECT
    Q_INTERFACES(QVideoOutputFactory)
public:
    QDirectPainterVideoOutputFactory();
    virtual ~QDirectPainterVideoOutputFactory();

    virtual QVideoFormatList supportedFormats();
    virtual QVideoFormatList preferredFormats();

    virtual QAbstractVideoOutput* create( QScreen *screen, QObject *parent = 0 );
};

#endif
