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

#ifndef QCAMERAVIDEOSURFACE_H
#define QCAMERAVIDEOSURFACE_H

#include <QList>
#include <QHash>
#include <QVideoFrame>
#include <QVideoSurface>

class CameraVideoSurface : public QObject
{
    Q_OBJECT;
public:
    CameraVideoSurface(QList<unsigned int>& expectedFormats, int defaultRotation);
    ~CameraVideoSurface();

    bool isValid() const { return m_valid; }
    QList<QVideoFrame::PixelFormat> commonFormats() { return m_commonFormats; }


    void setScaleMode(QtopiaVideo::VideoScaleMode s);

    unsigned int formatFourcc() const;
    QVideoFrame::PixelFormat formatQVideoFrame() const;

    QVideoFrame::PixelFormat formatMap(unsigned int) const;

    unsigned int formatMap(QVideoFrame::PixelFormat p) const;
    QVideoSurface* surface() ;

    void setRotation(int);
    QtopiaVideo::VideoRotation rotationMap(int);

    bool isDefaultSurface() const;

signals:
    void videoSurfaceFormatsChanged();

protected slots:
    void formatsChanged();

protected:
    void init(QList<unsigned int>&, int);
    void updateCommonFormats();

private:
    bool m_valid;
    bool m_useNativeFormat;
    QtopiaVideo::VideoRotation m_rotation;
    QVideoFormatList m_commonFormats;
    QVideoFormatList m_cameraFormats;

    QHash<unsigned int ,QVideoFrame::PixelFormat> m_formatMap;

    QVideoFrame::PixelFormat m_formatQVideoFrame;
    QtopiaVideo::VideoScaleMode m_scale;
    QVideoSurface *m_surface;
};

typedef QSet<QVideoFrame::PixelFormat> FormatSet;

#endif
