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

/*!
    \class QControlInterface
    \inpublicgroup QtMediaModule
    \brief The  QControlInterface class provides an interface for camera device modes
    that have controls
*/

/*!
    \fn QControlInterface::~QControlInterface()
    Destructor
    \internal
*/

/*!
    \fn  QList<QCameraControl*> QControlInterface::controls() const
    Returns a list of available controls
    Note: These controls should affect the current capture mode only
    \sa QCameraControl
*/

/*!
    \fn  void QControlInterface::setValue(quint32 id, int value)
    Sets control \a id to \a value
*/

/*!
    \class QCameraStreamInterface
    \inpublicgroup QtMediaModule
    \brief The QCameraStreamInterface class is an interface class for camera device modes that stream data
*/

/*!
    \fn QCameraStreamInterface::~QCameraStreamInterface()
    \internal
*/

/*!
    \fn void QCameraStreamInterface::start(unsigned int fmt, QSize res, int fps)
    Prompts the camera device to provide data in format \a fmt with size \a res and in \a fps
    frames per second (this value may be ignored by the device).
    Note: format is given as a fourcc code value.
*/

/*!
    \fn  void QCameraStreamInterface::stop()
    Prompts the camera device to stop producing data
*/

/*!
    \fn  QList<unsigned int> QCameraStreamInterface::framerates()
    Returns a list of supported framerates
*/

/*!
    \fn  unsigned int QCameraStreamInterface::framerate()
    Returns the current framerate
*/

/*!
    \class QCameraZoomInterface
    \inpublicgroup QtMediaModule
    \brief The QCameraZoomInterface class is an Interface class for zooming
    When implementing this functionality, Zooming should affect preview, still and video capture.
*/

/*!
    \fn QCameraZoomInterface::~QCameraZoomInterface()
    \internal
*/

/*!
    \fn  bool QCameraZoomInterface::hasZoom() const
    Returns true if the camera device has zoom capabilities otherwise returns false
*/

/*!
    \fn  QPair<unsigned int,unsigned int> QCameraZoomInterface::zoomRange()
    Returns the minimum and maximum supported zoom.
    A value of QPair<0,0>() indicates zoom not supported.
*/

/*!
    \fn  void QCameraZoomInterface::zoomIn()
    Zoom in by one step.
    Note: zoom steps begin from the minimum zoom value given by the function
*/

/*!
    \fn  void QCameraZoomInterface::zoomOut()
    Zoom out by one step
*/


/*!
    \class QCameraPreviewCapture
    \inpublicgroup QtMediaModule
    \brief The QCameraPreviewCapture class allows the user to stream view finder data from the device
*/

/*!
    \fn QCameraPreviewCapture::~QCameraPreviewCapture()
    \internal
*/

/*!
    \fn  QtopiaCamera::FormatResolutionMap QCameraPreviewCapture::formats() const
    Returns a QMap of formats and supported resolutions in that format
*/

/*!
    \fn  unsigned int QCameraPreviewCapture::format()
    Returns the current format

    Note to implementor:
    Should always be valid
*/

/*!
    \fn  QSize QCameraPreviewCapture::resolution()
    Returns the current resolution

    Note to implementor:
    Should always be valid
*/

/*!
    \fn void QCameraPreviewCapture::frameReady(QVideoFrame const& frame)
    Signals that a frame of data given by \a frame is ready
*/

/*!
    \class QCameraStillCapture
    \inpublicgroup QtMediaModule
    \brief The QCameraStillCapture class handles capturing of still images
*/

/*!
    \fn  QtopiaCamera::FormatResolutionMap QCameraStillCapture::formats() const
    Returns a QMap of formats and supported resolutions of that format
*/

/*!
    \fn  unsigned int QCameraStillCapture::format()
    Returns the current format
*/

/*!
    \fn  QSize QCameraStillCapture::resolution()
    Returns the current resolution

    Note to implementor:
    Should always be valid
*/

/*!
    \fn void QCameraStillCapture::autoFocus()
    If supported, causes the device to autofocus otherwise does nothing
*/

/*!
    \fn  void QCameraStillCapture::takeStillImage(unsigned int format, QSize resolution, int count, unsigned int msecs)
    Takes a still image in pixel format \a format and size \a resolution
    \a count is the number of multi-shots (only if capable, otherwise it may be ignored) and \a msecs is the delay in milliseconds between each shot
*/

/*!
    \fn void QCameraStillCapture::notifyPreSnap()
    Signals that the image in the preview data buffer is the actuall image that will be taken.
    Note:The procesing of this signal should take absolute minimal time.

    Note to implementor:
    This signal should be emitted just before the instance in which the photo is taken.
    It is optional.
*/


/*!
    \fn void QCameraStillCapture::imageReady(QByteArray & buffer, QSize resolution, bool complete)
    Signals that a the image contained in \a buffer with image \a resolution is ready
    If \a complete is true the \a buffer contains the full image, otherwise
    only a partial image was given. Subsequent data must be appended. This can
    happen for example with jpeg images.
*/

/*!
    \class QCameraVideoCapture
    \inpublicgroup QtMediaModule
    \brief The QCameraVideoCapture class allows the user to capture video data from a camera device.

    The QCameraVideoCapture class provides the interface neccesary to capture raw bitstreams of video data.
    Usage is through the provided QIODevice interfaces in read only mode.
    \sa QIODevice
*/

/*!
    \fn  QtopiaCamera::FormatResolutionMap QCameraVideoCapture::formats() const
    Returns a QMap of formats and supported resolutions of that format
*/

/*!
    \fn  unsigned int QCameraVideoCapture::format()
    Returns the current format

    Note to implementor:
    Should always be valid.
*/

/*!
    \fn  QSize QCameraVideoCapture::resolution()
    Returns the current  resolution

    Note to implementor:
    Should always be valid.
*/


/*!
    \class QCameraDevice
    \inpublicgroup QtMediaModule
    \brief The QCameraDevice class  represents a simple camera device.
    For the user it is a  thin interface for accessing camera functionality.

    The QCameraDevice has three modes of operation.
    \list
        \o Still:  Used to capture single or if supported multiple still images.
        \o Video:  Used to stream raw video data.
        \o Preview: Used to stream viewfinder data which can then be display on a device screen.
    \endlist
    When implementing this class it is a requirement that  previewing of data should always be available to the client
    whereas Still and Video capture are optional.
*/

/*!
    \enum QCameraDevice::CaptureMode

    This enum specifies the possible modes the device supports

    \value  StillMode   Indicates support for Still mode
    \value  VideoMode   Indicates support for Video mode
*/

/*!
    \enum QCameraDevice::Orientation

    This enum specifies the Orientation that the camera is in

    \value    FrontFacing   The camera  faces the user e.g For video conferencing
    \value    BackFacing    The camera  faces away from the user. e.g Most standard cameras
    \value    Changing      Has no fixed orientation. e.g Positionable / Swivel Cameras
*/

/*!
    \fn unsigned int  QCameraDevice::captureModes() const
    Returns a bit-wise OR of supported capture modes
    \sa QCameraDevice::CaptureMode
*/

/*!
    \fn  QCameraPreviewCapture* QCameraDevice::previewCapture() const
    Returns a class that implements the functionality to capture  preview data from the camera device
    \sa QCameraPreviewCapture
*/

/*!
    \fn  QCameraStillCapture* QCameraDevice::stillCapture() const
    Returns a class that implements the functionality to capture still data from the camera device
    \sa QCameraStillCapture
*/

/*!
    \fn  QCameraVideoCapture* QCameraDevice::videoCapture() const
    Returns a class that implements the functionality to capture video data from the camera device
    \sa QCameraVideoCapture
*/

/*!
    \fn  QCameraDevice::Orientation QCameraDevice::orientation() const
    Returns the camera device orientation
    \sa QCameraDevice::Orientation
*/

/*!
    \fn  QString QCameraDevice::description() const
    Returns a description string of the camera device
*/

/*!
    \fn  QString QCameraDevice::name() const
    Returns an indentifier for the camera device
*/

/*!
    \fn void QCameraDevice::cameraError(QtopiaCamera::CameraError err, QString errorMessage)
    Signals that an error \a err has occured. Clients should connect to this signal to receive debugging/error information
    contained in \a errorMessage

    \sa QtopiaCamera::CameraError
*/


