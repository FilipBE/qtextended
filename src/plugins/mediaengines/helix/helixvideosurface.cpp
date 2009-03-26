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

#include "helixvideosurface.h"

#include "helixutil.h"
#include "reporterror.h"

#include <qvideoframe.h>

#if defined (Q_WS_X11)
#include <QX11Info>
#elif defined (Q_WS_QWS)
#include <QtGui/qscreen_qws.h>
#endif

#include <colormap.h>


#define GETBITMAPCOLOR(x) GetBitmapColor( (HXBitmapInfo*)(x) )
#define GETBITMAPPITCH(x) GetBitmapPitch( (HXBitmapInfo*)(x) )

static QVideoFrame::PixelFormat hxCompressionToPixelFormat( const HX_COMPRESSION_TYPE t )
{
    static QMap<HX_COMPRESSION_TYPE, QVideoFrame::PixelFormat> typesMap;

    if ( typesMap.isEmpty() ) {
        typesMap[ HX_I420 ] = QVideoFrame::Format_YUV420P;
        typesMap[ HX_YV12 ] = QVideoFrame::Format_YV12;
        typesMap[ HX_YUY2 ] = QVideoFrame::Format_UYVY;
        typesMap[ HX_UYVY ] = QVideoFrame::Format_UYVY;
        typesMap[ HX_ARGB ] = QVideoFrame::Format_ARGB32;
        typesMap[ HXCOLOR_RGB565_ID ] = QVideoFrame::Format_RGB565;
        typesMap[ HX_RGB ] = QVideoFrame::Format_RGB32;
    }

    return typesMap.value( t, QVideoFrame::Format_Invalid );
}

static HX_COMPRESSION_TYPE pixelFormatToHxCompression( QVideoFrame::PixelFormat pixelFormat )
{
    static QMap<QVideoFrame::PixelFormat,HX_COMPRESSION_TYPE> typesMap;

    if ( typesMap.isEmpty() ) {
        typesMap[ QVideoFrame::Format_YUV420P ] = HX_I420;
        typesMap[ QVideoFrame::Format_YV12 ] = HX_YV12;
        typesMap[ QVideoFrame::Format_UYVY ] = HX_UYVY;
        typesMap[ QVideoFrame::Format_ARGB32 ] = HX_ARGB;
        typesMap[ QVideoFrame::Format_RGB565 ] = HXCOLOR_RGB565_ID;
        typesMap[ QVideoFrame::Format_RGB32 ] = HX_RGB;
    }

    return typesMap.value( pixelFormat, 0 );
}


static HelixColorLibrary load_color_library()
{
    QLibrary library( helix_library_path() + QLatin1String("/hxltcolor.so") );

    HelixColorLibrary symbols;
    symbols.GetColorConverter = (FPGETCOLORCONVERTER)library.resolve( "GetColorConverter" );
    symbols.InitColorConverter = (FPINITCOLORCONVERTER)library.resolve( "InitColorConverter" );

    return symbols;
}

static int NullConverter(unsigned char*, int, int, int, int, int, int, int,
                         unsigned char*, int, int, int, int, int, int, int)
{
    return 0;
}

GenericVideoSurface::GenericVideoSurface():
    m_refCount(0),
    m_aspectRatio(0),
    m_aspectRatioDefined(false),
    Converter(0),
    m_inputFormat( QVideoFrame::Format_Invalid ),
    m_outputFormat( QVideoFrame::Format_Invalid ),
    m_paintObserver(0)
{
    m_library = load_color_library();

    if (m_library.InitColorConverter)
    {
        m_library.InitColorConverter();
    }
    else
    {
        REPORT_ERROR(ERR_HELIX);
    }
}

STDMETHODIMP GenericVideoSurface::BeginOptimizedBlt(HXBitmapInfoHeader *pBitmapInfo)
{
    Q_UNUSED(pBitmapInfo);

    return HXR_NOTIMPL;
}

static inline bool is16Bit()
{
#if defined(Q_WS_QWS)
    return qt_screen->depth() == 16;
#elif defined(Q_WS_X11)
    return QX11Info::appDepth() == 16;
#else
    return false;
#endif
}

STDMETHODIMP GenericVideoSurface::Blt( UCHAR* pImageBits, HXBitmapInfoHeader* pBitmapInfo, REF(HXxRect) rDestRect, REF(HXxRect) rSrcRect )
{
    if ( m_videoSize.isEmpty() ) {
        m_bufferWidth = rSrcRect.right - rSrcRect.left;
        m_bufferHeight = rSrcRect.bottom - rSrcRect.top;
        m_videoSize = QSize(m_bufferWidth, m_bufferHeight);

        int dstWidth = rDestRect.right - rDestRect.left;
        int dstHeight = rDestRect.bottom - rDestRect.top;

        if ( qAbs( m_bufferWidth*1024/m_bufferHeight - dstWidth*1024/dstHeight ) > 100  ) {
            // more than ~ 0.01 difference in aspect ration between src and dst
            m_aspectRatio = double( dstWidth ) / dstHeight;
            m_aspectRatioDefined = true;
            //qWarning() << "detected custom aspect ratio" << m_aspectRatio;
        }
    }

    if ( !m_paintObserver )
        return HXR_OK;

    QVideoFrame::PixelFormat prev_inputFormat = m_inputFormat;
    m_inputFormat = hxCompressionToPixelFormat( pBitmapInfo->biCompression );

    if ( m_inputFormat != prev_inputFormat )
        m_outputFormat == QVideoFrame::Format_Invalid;

    if ( m_inputFormat == QVideoFrame::Format_Invalid )
        return HXR_OK;

    if ( m_outputFormat == QVideoFrame::Format_Invalid ) {
        m_inPitch = GETBITMAPPITCH( pBitmapInfo );
        int inCID = GETBITMAPCOLOR( pBitmapInfo );

        QVideoFormatList preferredFormats = m_paintObserver->preferredFormats();
        if ( preferredFormats.contains( m_inputFormat ) ) {
            // video output supports format directly, skip color convertor step
            m_outputFormat = m_inputFormat;
        } else {
            QVideoFormatList allFormats = preferredFormats + m_paintObserver->supportedFormats();
            if ( allFormats.isEmpty() )
                return HXR_OK;

            if (m_library.GetColorConverter) {
                foreach ( QVideoFrame::PixelFormat format, allFormats ) {
                    HXBitmapInfoHeader bufferInfo;
                    memset( &bufferInfo, 0, sizeof(HXBitmapInfoHeader) );

                    bufferInfo.biWidth = m_bufferWidth;
                    bufferInfo.biHeight = m_bufferHeight;

                    bufferInfo.biPlanes = 1; //QVideoFrame::planesCount( format );
                    bufferInfo.biBitCount = QVideoFrame::colorDepth( format, 0 );
                    if ( QVideoFrame::planesCount( format ) > 1 ) {
                        bufferInfo.biBitCount += QVideoFrame::colorDepth( format, 1 );
                        bufferInfo.biBitCount += QVideoFrame::colorDepth( format, 2 );
                    }

                    bufferInfo.biCompression = pixelFormatToHxCompression( format );
                    bufferInfo.biSizeImage = bufferInfo.biWidth * bufferInfo.biHeight * bufferInfo.biBitCount / 8;

                    m_bufferPitch = GETBITMAPPITCH( &bufferInfo );
                    int bufferCID = GETBITMAPCOLOR( &bufferInfo );

                    Converter = m_library.GetColorConverter( inCID, bufferCID );

                    if ( Converter ) {
                        // found converter from m_inputFormat to m_outputFormat
                        m_outputFormat = format;
                        break;
                    }
                }
            }

            if ( !Converter ) {
                REPORT_ERROR( ERR_UNSUPPORTED );
                // Assign null converter if no converter available
                Converter = &NullConverter;
            }
        }
    }

    if ( m_outputFormat == QVideoFrame::Format_Invalid ) {
        REPORT_ERROR( ERR_UNSUPPORTED );
        Converter = &NullConverter;
        return HXR_FAIL;
    }

    if ( m_inputFormat == m_outputFormat ) {
        if ( !m_outputFrame.isNull() )
            m_outputFrame = QVideoFrame();

        int src_height = pBitmapInfo->biHeight;
        int src_pitch = m_inPitch;

        //TODO: we assume YUV420 or Format_YV12 here, add support for other formats
        uchar *pY = pImageBits;
        uchar* pU = pY + src_height*src_pitch;
        uchar* pV = pU + src_height*src_pitch/4;

        pY += rSrcRect.left + rSrcRect.top*src_pitch;
        pU += rSrcRect.left/2 + rSrcRect.top*src_pitch/4;
        pV += rSrcRect.left/2 + rSrcRect.top*src_pitch/4;

        QVideoFrame frame( m_outputFormat,
                           m_videoSize,
                           pY,
                           pU,
                           pV,
                           src_pitch,
                           src_pitch/2,
                           src_pitch/2 );

        if ( m_aspectRatioDefined )
            frame.setAspectRatio( m_aspectRatio );

        m_paintObserver->paint( frame );

    } else {
        if ( m_outputFrame.format() != m_outputFormat || m_outputFrame.size() != m_videoSize ) {
            m_outputFrame = QVideoFrame( m_outputFormat, m_videoSize );
        }

        Converter(m_outputFrame.planeData(0),
                  m_bufferWidth,
                  m_bufferHeight,
                  m_outputFrame.bytesPerLine( 0 ),
                  0,
                  0,
                  m_bufferWidth,
                  m_bufferHeight,

                  pImageBits,
                  pBitmapInfo->biWidth,
                  pBitmapInfo->biHeight,
                  m_inPitch,
                  rSrcRect.left,
                  rSrcRect.top,
                  rSrcRect.right - rSrcRect.left,
                  rSrcRect.bottom - rSrcRect.top
                 );

        if ( m_aspectRatioDefined )
            m_outputFrame.setAspectRatio( m_aspectRatio );

        m_paintObserver->paint( m_outputFrame );

    }
    return HXR_OK;
}


void GenericVideoSurface::repaintLastFrame()
{
    if ( m_paintObserver && !m_outputFrame.isNull() ) {
        m_paintObserver->paint( m_outputFrame );
    }
}

void GenericVideoSurface::updateColorConverter()
{
    qWarning() << "helix::GenericVideoSurface::updateColorConverter";
    m_outputFormat == QVideoFrame::Format_Invalid;
}

STDMETHODIMP GenericVideoSurface::EndOptimizedBlt()
{
    return HXR_NOTIMPL;
}

STDMETHODIMP GenericVideoSurface::GetOptimizedFormat( REF(HX_COMPRESSION_TYPE) ulType )
{
    Q_UNUSED(ulType);

    return HXR_NOTIMPL;
}

STDMETHODIMP GenericVideoSurface::GetPreferredFormat( REF(HX_COMPRESSION_TYPE) ulType )
{
    ulType = HX_RGB;

    return HXR_OK;
}

STDMETHODIMP GenericVideoSurface::OptimizedBlt( UCHAR* pImageBits, REF(HXxRect) rDestRect, REF(HXxRect) rSrcRect )
{
    Q_UNUSED(pImageBits);
    Q_UNUSED(rDestRect);
    Q_UNUSED(rSrcRect);

    return HXR_NOTIMPL;
}

STDMETHODIMP_(ULONG32) GenericVideoSurface::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) GenericVideoSurface::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP GenericVideoSurface::QueryInterface(REFIID riid, void** object)
{
    if (IsEqualIID( riid, IID_IUnknown))
    {
        AddRef();
        *object = (IUnknown*)(IHXSite*)this;
        return HXR_OK;
    }
    else if (IsEqualIID(riid, IID_IHXVideoSurface))
    {
        REPORT_ERROR( ERR_TEST );
        AddRef();
        *object = (IHXVideoSurface*)this;
        return HXR_OK;
    }

    *object = NULL;

    return HXR_NOINTERFACE;
}

void GenericVideoSurface::setPaintObserver(PaintObserver* paintObserver)
{
    m_paintObserver = paintObserver;
}

