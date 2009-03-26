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

#ifndef HELIXVIDEOSURFACE_H
#define HELIXVIDEOSURFACE_H

#include <QtGui>

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxwin.h>
#include <hxwintyp.h> // ### hxcore && hxwin && hxwintyp ?
#include <hxvsurf.h>
#include <hxcolor.h>

#include <qvideoframe.h>

class QVideoSurface;

typedef LPHXCOLORCONVERTER (HXEXPORT_PTR FPGETCOLORCONVERTER) (INT32 cidIn, INT32 cidOut);
typedef void (HXEXPORT_PTR FPINITCOLORCONVERTER) (void);

struct HelixColorLibrary
{
    FPGETCOLORCONVERTER GetColorConverter;
    FPINITCOLORCONVERTER InitColorConverter;
};


class PaintObserver
{
public:
    virtual ~PaintObserver() {}
    virtual void paint( QVideoFrame const& frame) = 0;

    virtual QVideoFormatList preferredFormats() const = 0;
    virtual QVideoFormatList supportedFormats() const = 0;

    virtual QVideoSurface *videoSurface() = 0;
};


class GenericVideoSurface : public IHXVideoSurface
{
public:
    GenericVideoSurface();
    virtual  ~GenericVideoSurface() {}
    //QImage const& buffer() const { return m_buffer; }

    // IHXVideoSurface
    STDMETHOD(BeginOptimizedBlt) (THIS_ HXBitmapInfoHeader *pBitmapInfo);
    STDMETHOD(Blt) (THIS_
        UCHAR* pImageBits,
        HXBitmapInfoHeader* pBitmapInfo,
        REF(HXxRect) rDestRect,
        REF(HXxRect) rSrcRect);
    STDMETHOD(EndOptimizedBlt) (THIS);
    STDMETHOD(GetOptimizedFormat) (THIS_ REF(HX_COMPRESSION_TYPE) ulType);
    STDMETHOD(GetPreferredFormat) (THIS_ REF(HX_COMPRESSION_TYPE) ulType);
    STDMETHOD(OptimizedBlt) (THIS_
        UCHAR* pImageBits,
        REF(HXxRect) rDestRect,
        REF(HXxRect) rSrcRect);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

    void setPaintObserver(PaintObserver* paintObserver);

    void updateColorConverter();
    void repaintLastFrame();
private:

    INT32 m_refCount;

    HelixColorLibrary m_library;

    QSize               m_videoSize;
    double m_aspectRatio;
    bool m_aspectRatioDefined;
    QVideoFrame m_outputFrame;

    //QImage              m_buffer;
    LPHXCOLORCONVERTER  Converter;
    QVideoFrame::PixelFormat m_inputFormat;
    QVideoFrame::PixelFormat m_outputFormat;
    int                 m_bufferPitch;
    int                 m_inPitch;
    int                 m_bufferWidth;
    int                 m_bufferHeight;
    PaintObserver*      m_paintObserver;
};

#endif
