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

#include "qwindowdecorationinterface.h"

/*! \class QWindowDecorationInterface
    \inpublicgroup QtBaseModule

  \brief The QWindowDecorationInterface class provides an interface for Qt Extended window decoration styles.

  Window decoration styles may be added to Qt Extended via plug-ins. In order to
  write a style plug-in you must 
  derive from the QWindowDecorationInterface class and implement
  the virtual functions.

  The window being decorated is defined by the
  QWindowDecorationPlugin::WindowData struct:
\code
struct WindowData {
    const QWidget *window;
    QRect rect;
    QPalette palette;
    QString caption;
    enum Flags { Maximized=0x01, Dialog=0x02, Active=0x04 };
    Q_UINT32 flags;
    Q_UINT32 reserved;
};
\endcode

    Window decorations are loaded by setting the \c Decoration setting
    in the \c Appearance group of \c {Trolltech/qpe}, for example:

\code
    [Appearance]
    Decoration=mydecoration
\endcode

    To allow a QWindowDecorationInterface class to be created a plug-in class
    derived from QWindowDecorationPlugin should be implemented.

    Note that Qt Extended includes a decoration plugin that
    allows basic \l {Tutorial: Window Decoration Theme}{theming}, and this
    should be used in preference to custom decoration plugins.

    \sa QWindowDecorationPlugin

    \ingroup appearance
*/

/*! \internal
    \fn QWindowDecorationInterface::~QWindowDecorationInterface()
*/

/*! \enum QWindowDecorationInterface::WindowData::Flags

    \value Maximized The window is maximized.
    \value Dialog The window is a dialog.
    \value Active The window has keyboard focus.

*/

/*! \enum QWindowDecorationInterface::Metric

  \value TitleHeight the height of the title.
  \value LeftBorder the width of the border on the left of the window.
  \value RightBorder the width of the border on the right of the window.
  \value TopBorder the width of the border on the top of the window, above
    the title bar.
  \value BottomBorder the width of the border on the bottom of the window.
  \value OKWidth the width of the OK button.
  \value CloseWidth the width of the Close (X) button.
  \value HelpWidth the width of the Help (?) button.
  \value MaximizeWidth the width of the maximize/restore button.
  \value CornerGrabSize the size of the area allowing diagonal resize at
    each corner of the window.
*/

/*! \enum QWindowDecorationInterface::Button

  \value OK the OK button.
  \value Close the close button.
  \value Help the help button.
  \value Maximize the maximize/restore button.
*/

/*! \enum QWindowDecorationInterface::Area

  \value Border defines the entire decoration area, excluding the title bar.
  \value Title defines the area at the top of the window that contains the
    buttons and captions.  It must extend the full width of the window.
  \value TitleText defines the window caption.
*/

/*! \fn int QWindowDecorationInterface::metric( Metric metric, const WindowData *wd ) const

  Returns the \a metric for the window defined by \a wd.
*/

/*! \fn void QWindowDecorationInterface::drawArea( Area area, QPainter *p, const WindowData *wd ) const

  Draw the specified \a area using QPainter \a p for the window defined by \a wd.
*/

/*! \fn void QWindowDecorationInterface::drawButton( Button button, QPainter *painter, const WindowData *wd, int x, int y, int w, int h, QDecoration::DecorationState state ) const

  Draw \a button with QPainter \a painter for the window defined by \a wd
  within the bounds supplied by \a x, \a y, \a w, \a h in the
  specified \a state.
*/

/*! \fn QRegion QWindowDecorationInterface::mask( const WindowData *wd ) const

  Returns the mask of the decoration including all borders and the title
  for the window defined by \a wd as a QRegion.  The window decorations do
  not necessarily need to be rectangular, however the title bar must be
  rectangular and must be the width of the window.  This ensures the title
  is drawn correctly for maximized windows.
*/

/*! \fn QString QWindowDecorationInterface::name() const

  Returns the name of the decoration. This will
  be displayed in the appearance settings dialog.
*/

/*! \fn QPixmap QWindowDecorationInterface::icon() const

  Returns the icon of the decoration. This may
  be displayed in the appearance settings dialog.
*/



/*!
    \class QWindowDecorationPlugin
    \inpublicgroup QtBaseModule
    \brief The QWindowDecorationPlugin class defines a base class for
    implementing window decoration plugins.

    \ingroup plugins
    \ingroup appearance

    Classes that inherit QWindowDecorationPlugin must provide
    implementations of the keys() and decoration() functions.

    \sa QWindowDecorationInterface
*/

/*!
    \fn QStringList QWindowDecorationPlugin::keys() const

    Returns the list of interfaces implemented by this plug-in.
*/

/*!
    \fn QWindowDecorationInterface *QWindowDecorationPlugin::decoration(const QString &key)

    Returns an instance of the QWindowDecorationInterface matching \a key.
*/

/*!
    \fn QWindowDecorationPlugin::QWindowDecorationPlugin(QObject* parent)

    Constructs a QWindowDecorationPlugin with the given \a parent.
*/
QWindowDecorationPlugin::QWindowDecorationPlugin(QObject*)
{
}


/*! \fn QWindowDecorationPlugin::~QWindowDecorationPlugin()

  Destroys a QWindowDecorationPlugin.
*/
QWindowDecorationPlugin::~QWindowDecorationPlugin()
{
}
