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

#ifndef QABSTRACTVIDEOOUTPUT_H
#define QABSTRACTVIDEOOUTPUT_H

#include <QObject>

#include "qtopiavideo.h"
#include "qvideoframe.h"


class QRect;
class QRegion;
class QScreen;

class QAbstractVideoOutputPrivate;

class QTOPIAVIDEO_EXPORT QAbstractVideoOutput : public QObject
{
    Q_OBJECT

public:
    QAbstractVideoOutput( QScreen *screen = 0, QObject *parent = 0 );
    virtual ~QAbstractVideoOutput();

    int winId() const;
    QScreen *screen() const;

    void renderFrame( const QVideoFrame& frame );

    QtopiaVideo::VideoRotation rotation() const;
    QtopiaVideo::VideoScaleMode scaleMode() const;

    QRect geometry() const;
    QRegion requestedRegion() const;
    QRegion clipRegion () const;

    virtual QVideoFormatList supportedFormats() = 0;
    virtual QVideoFormatList preferredFormats() = 0;

    virtual bool isAcceleratedScaleSupported() = 0;

public slots:
    void setRotation( QtopiaVideo::VideoRotation );
    void setScaleMode( QtopiaVideo::VideoScaleMode );

    void setGeometry( const QRect& );
    void setRegion( const QRegion& );
    void setClipRegion( const QRegion& );

protected:
    bool isModified() const;
    void setModified( bool );

    virtual void geometryChanged();
    virtual void clipRegionChanged();

    virtual void doRenderFrame( const QVideoFrame& frame ) = 0;

    QRegion deviceMappedClipRegion() const;
    QRect deviceMappedGeometry() const;

private:
    Q_DISABLE_COPY(QAbstractVideoOutput);
    friend class QAbstractVideoOutputPrivate;
    QAbstractVideoOutputPrivate *d;
};

class QNullVideoOutput : public QAbstractVideoOutput
{
    Q_OBJECT

public:
    QNullVideoOutput( QScreen *screen = 0, QObject *parent = 0 )
        :QAbstractVideoOutput(screen, parent) {};
    virtual ~QNullVideoOutput() {};

    virtual QVideoFormatList supportedFormats()
        {  return QVideoFormatList(); }
    virtual QVideoFormatList preferredFormats()
        {  return QVideoFormatList();  }
    virtual bool isAcceleratedScaleSupported()
        {  return false;  }
protected:
    virtual void doRenderFrame(const QVideoFrame &) {};
};

#endif //QABSTRACTVIDEOOUTPUT_H

