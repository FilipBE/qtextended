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

#include "qscreeninformation.h"
#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <QApplication>
#include <QDesktopWidget>
#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>

/*!
    \class QScreenInformation
    \inpublicgroup QtBaseModule


    \brief The QScreenInformation class provides additional information about screens beyond that from QDesktopWidget.

    Some devices have multiple output screens.  For example, a device might
    have a primary LCD screen plus a TV composite video output socket.  The TV
    output can display what is on the LCD screen (known as "cloning"), or it can
    provide a separate framebuffer for displaying a second image that is different
    from that on the LCD.

    The following code finds the first screen of type Television and then directs
    that it be a clone of the primary screen:

    \code
    QScreenInformation tv(QScreenInformation::Television);
    tv.setClonedScreen(QApplication::desktop()->primaryScreen());
    \endcode

    The television screen can be separated from the LCD for separate display
    as follows:

    \code
    tv.setClonedScreen(-1);
    \endcode

    When the \c tv object is deleted, or the application exits, the clone state
    will revert to its default setting.

    Some devices have multiple screens that can be visible at the same
    time, overlaid on each other.  Transparency colors are used to "poke holes"
    in one layer to allow lower layers to be partially or completely
    revealed.

    Overlay screens are marked with the type QScreenInformation::Overlay.
    Such screens will typically return a list of layers from
    supportedLayers() that indicates the layer numbers that can be
    set for the overlay.

    Layer numbers are always relative to zero, which indicates the main
    GUI layer.  Layer numbers less than zero indicate a layer that is
    below the main GUI layer and numbers greater than zero indicate a
    layer that is above the main GUI layer.

    When a client application calls setLayer(), a request is sent to
    the screen's QScreenInformationProvider object to change the layer
    number and make the screen visible.  When the QScreenInformation object
    is deleted, or the application exits, the overlay is made invisible
    and reverted to its default layer ordering.

    \sa QScreenInformationProvider

    \ingroup hardware
*/

/*!
    \enum QScreenInformation::Type
    This enum defines the type of a screen.

    \value Normal The screen is a normal display; e.g. an LCD or CRT.
    \value Television The screen outputs a television signal.
    \value Overlay The screen is a normal display overlaid over one of the other screens.
*/

class QScreenInformationPrivate
{
public:
    int screenNumber;
    QValueSpaceItem *item;
    QValueSpaceObject *cloneRequested;
    QValueSpaceObject *layerRequested;
};

/*!
    Constructs a screen information object for \a screenNumber and
    attaches it to \a parent.  If \a screenNumber is -1, then the
    primary screen number will be used.  Otherwise, if the \a screenNumber
    is invalid, then screenNumber() on the constructed object will be -1.

    \sa screenNumber()
*/
QScreenInformation::QScreenInformation(int screenNumber, QObject *parent)
    : QObject(parent)
{
    if (screenNumber == -1)
        screenNumber = QApplication::desktop()->primaryScreen();
    else if (screenNumber < 0 || screenNumber >= QApplication::desktop()->numScreens())
        screenNumber = -1;
    init(screenNumber);
}

/*!
    Constructs a screen information object for the first screen
    with the specified \a type and attaches it to \a parent.
    If there is no such screen, then screenNumber() on the
    constructed object will return -1.

    \sa screenNumber()
*/
QScreenInformation::QScreenInformation(QScreenInformation::Type type, QObject *parent)
    : QObject(parent)
{
    int found = -1;
    QDesktopWidget *desktop = QApplication::desktop();
    for (int screen = 0; screen < desktop->numScreens(); ++screen) {
        QValueSpaceItem item
            (QLatin1String("/Hardware/Screens/") + QString::number(screen), this);
        if (item.value("type", -1).toInt() == (int)type) {
            found = screen;
            break;
        }
    }
    init(found);
}

void QScreenInformation::init(int screenNumber)
{
    d = new QScreenInformationPrivate();
    d->screenNumber = screenNumber;
    d->item = new QValueSpaceItem
        (QLatin1String("/Hardware/Screens/") + QString::number(screenNumber), this);
    connect(d->item, SIGNAL(contentsChanged()), this, SIGNAL(changed()));
    d->cloneRequested = 0;
    d->layerRequested = 0;
}

/*!
    Destroys this screen information object.  If the clone state was
    changed with setClonedScreen(), then the system will revert the
    screen to its previous clone mode.

    \sa setClonedScreen()
*/
QScreenInformation::~QScreenInformation()
{
    delete d;
}

/*!
    Returns the screen number that this object pertains to.
    Returns -1 if this object was constructed using an invalid
    screen number, or it was constructed using a QScreenInformation::Type
    and there was no screen with that type.
*/
int QScreenInformation::screenNumber() const
{
    return d->screenNumber;
}

/*!
    Returns true if the screen is expected to be visible to the user
    at the present time.  In the case of Television screens, this may
    be false if the video cable is not connected.

    \sa changed()
*/
bool QScreenInformation::isVisible() const
{
    return d->item->value("isVisible", true).toBool();
}

/*!
    Returns the type of screen.
*/
QScreenInformation::Type QScreenInformation::type() const
{
    return (Type)(d->item->value("type", (int)Normal).toInt());
}

/*!
    Returns the number of the screen that this screen is cloning.
    The output on the other screen will also be sent to this screen.
    If the value is -1, then this screen is separated from all
    the others.

    \sa setClonedScreen(), changed()
*/
int QScreenInformation::clonedScreen() const
{
    return d->item->value("clonedScreen", -1).toInt();
}

/*!
    Sets the number of the screen that this screen is cloning to \a value.
    The output on screen \a value will also be sent to this screen.
    If \a value is -1, then this screen should be separated from all others.

    When this QScreenInformation object is deleted, the requested clone
    operation will be reverted.  This permits the system to revert the
    clone state to the default when an application crashes.

    The value of clonedScreen() will remain at its previous value until
    the changed() signal is emitted.  The system is free to ignore the
    request if the requested clone operation is not supported.

    \sa clonedScreen(), changed()
*/
void QScreenInformation::setClonedScreen(int value)
{
    if (!d->cloneRequested) {
        // Post a temporary value into the value space that will revert
        // to false when this application exits so that the system can
        // automatically revert the clone mode on the screen to the default.
        d->cloneRequested = new QValueSpaceObject
            (QLatin1String("/Hardware/Screens/Cloned/") +
             QString::number(d->screenNumber), this);
        d->cloneRequested->setAttribute("", true);
        QValueSpaceObject::sync();  // Flush this value before sending the request.
    }
    QtopiaIpcEnvelope env
        (QLatin1String("QPE") + d->item->itemName(), "setClonedScreen(int)");
    env << value;
}

/*!
    Returns the overlay layer that this screen currently occupies.
    A value of zero indicates the normal GUI layer.  A value less
    than zero indicates that the overlay is currently below the
    normal GUI layer.  A value greater than zero indicates that
    the overlay is currently above the normal GUI layer.
    The default value is zero.

    \sa setLayer(), supportedLayers(), transparencyColor()
*/
int QScreenInformation::layer() const
{
    return d->item->value("layer", 0).toInt();
}

/*!
    Sets the overlay layer for this screen to \a value.
    A \a value of zero indicates the normal GUI layer.  A \a value less
    than zero indicates that the overlay is currently below the
    normal GUI layer.  A \a value greater than zero indicates that
    the overlay is currently above the normal GUI layer.

    The layer can only be set to one of the values in the list
    returned by supportedLayers().  Requests to change the layer
    to something else will be ignored.

    Setting the layer value for an overlay screen will also make
    it visible.  The layer will be made invisible again once the
    QScreenInformation object is deleted, or the application exits.
    The isVisible() function can be used to determine if the layer
    is currently visible.

    \sa layer(), supportedLayers(), isVisible()
*/
void QScreenInformation::setLayer(int value)
{
    if (!d->layerRequested) {
        // Post a temporary value into the value space that will revert
        // to false when this application exits so that the system can
        // automatically revert the layer on the screen to the default.
        d->layerRequested = new QValueSpaceObject
            (QLatin1String("/Hardware/Screens/Layer/") +
             QString::number(d->screenNumber), this);
        d->layerRequested->setAttribute("", true);
        QValueSpaceObject::sync();  // Flush this value before sending the request.
    }
    QtopiaIpcEnvelope env
        (QLatin1String("QPE") + d->item->itemName(), "setLayer(int)");
    env << value;
}

/*!
    Returns the list of overlay layer numbers that are supported
    for this screen.  This can be used by the client application to
    locate an overlay layer that can be placed above or below the
    normal GUI layer.

    As an example, if the list contains -1, 1, and 2, then the layer
    can be placed one step below the main GUI layer, or either one
    or two steps above the main GUI layer.

    If the returned list is empty, then the overlay position is fixed
    at the value returned by layer().

    \sa layer(), setLayer()
*/
QList<int> QScreenInformation::supportedLayers() const
{
    QStringList list = d->item->value("supportedLayers").toStringList();
    QList<int> values;
    foreach (QString value, list)
        values.append(value.toInt());
    return values;
}

/*!
    Returns the color to fill an area of this screen with to cause
    layers below this one to be visible through the transparency gaps.

    \sa layer()
*/
QColor QScreenInformation::transparencyColor() const
{
    QString color = d->item->value("transparencyColor").toString();
    if (color.isEmpty())
        return QColor();
    else
        return QColor(color);
}

/*!
    \fn void QScreenInformation::changed()

    Signal that is emitted when one of isVisible(), type(), clonedScreen(),
    layer(), supportedLayers(), or transparencyColor() changes.

    \sa isVisible(), type(), clonedScreen(), layer()
    \sa supportedLayers(), transparencyColor()
*/

/*!
    \class QScreenInformationProvider
    \inpublicgroup QtBaseModule


    \brief The QScreenInformationProvider class provides the back end implementation of QScreenInformation.

    The following class represents a screen of type Television which by default
    is a clone of the primary LCD screen:

    \code
    class TvScreen : public QScreenInformationProvider
    {
        Q_OBJECT
    public:
        TvScreen(int screenNumber, QObject *parent=0);

    public slots:
        void videoCableInserted();
        void videoCableRemoved();

    protected:
        void changeClonedScreen(int value);
        void revertClonedScreen();
    };

    TvScreen::TvScreen(int screenNumber, QObject *parent)
        : QScreenInformationProvider(screenNumber, parent)
    {
        setType(QScreenInformation::Television);
        setVisible(false);
        setClonedScreen(QApplication::desktop()->primaryScreen());
    }

    void TvScreen::videoCableInserted()
    {
        setVisible(true);
    }

    void TvScreen::videoCableRemoved()
    {
        setVisible(false);
    }

    void TvScreen::changeClonedScreen(int value)
    {
        // Tell the hardware to change the clone mode.
        ...

        // Update the clone state as seen by client applications.
        setClonedScreen(value);
    }

    void TvScreen::revertClonedScreen()
    {
        // Change back to cloning the primary screen.
        changeClonedScreen(QApplication::desktop()->primaryScreen());
    }
    \endcode

    \sa QScreenInformation

    \ingroup hardware
*/

class QScreenInformationProviderPrivate
{
public:
    QValueSpaceObject *object;
    QValueSpaceItem *cloneRequested;
    QValueSpaceItem *layerRequested;
    bool sawCloneRequest;
    bool sawLayerRequest;
};

/*!
    Constructs a screen information provider object for \a screenNumber
    and attaches it to \a parent.  If \a screenNumber is -1, then
    the primary screen number is used.

    Object construction should be followed by calls to setVisible(),
    setType(), and setClonedScreen() to populate the screen's initial
    properties.

    \sa setVisible(), setType(), setClonedScreen()
*/
QScreenInformationProvider::QScreenInformationProvider(int screenNumber, QObject *parent)
    : QObject(parent)
{
    d = new QScreenInformationProviderPrivate();
    if (screenNumber == -1)
        screenNumber = QApplication::desktop()->primaryScreen();
    d->object = new QValueSpaceObject
        (QLatin1String("/Hardware/Screens/") + QString::number(screenNumber), this);
    d->cloneRequested = new QValueSpaceItem
        (QLatin1String("/Hardware/Screens/Cloned/") +
         QString::number(screenNumber), this);
    d->layerRequested = new QValueSpaceItem
        (QLatin1String("/Hardware/Screens/Layer/") +
         QString::number(screenNumber), this);
    d->sawCloneRequest = false;
    d->sawLayerRequest = false;
    connect(d->cloneRequested, SIGNAL(contentsChanged()), this, SLOT(checkForCloneRevert()));
    connect(d->layerRequested, SIGNAL(contentsChanged()), this, SLOT(checkForLayerRevert()));

    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor
        (QLatin1String("QPE") + d->object->objectPath(), this);
    QtopiaIpcAdaptor::connect
        (adaptor, MESSAGE(setClonedScreen(int)), this, SLOT(setClonedMessage(int)));
    QtopiaIpcAdaptor::connect
        (adaptor, MESSAGE(setLayer(int)), this, SLOT(setLayerMessage(int)));
}

/*!
    Destroys this screen information provider object.
*/
QScreenInformationProvider::~QScreenInformationProvider()
{
    delete d;
}

/*!
    Sets the state of QScreenInformation::isVisible() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::isVisible(),
    will be true.

    \sa QScreenInformation::isVisible(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setVisible(bool value)
{
    d->object->setAttribute("isVisible", value);
}

/*!
    Sets the state of QScreenInformation::type() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::type(),
    will be QScreenInformation::Normal.

    \sa QScreenInformation::type(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setType(QScreenInformation::Type value)
{
    d->object->setAttribute("type", (int)value);
}

/*!
    Sets the state of QScreenInformation::clonedScreen() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::clonedScreen(),
    will be -1.

    \sa QScreenInformation::clonedScreen(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setClonedScreen(int value)
{
    d->object->setAttribute("clonedScreen", value);
}

/*!
    Sets the state of QScreenInformation::layer() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::layer(),
    will be 0.

    \sa QScreenInformation::layer(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setLayer(int value)
{
    d->object->setAttribute("layer", value);
}

/*!
    Sets the state of QScreenInformation::supportedLayers() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::supportedLayers(),
    will be an empty list.

    \sa QScreenInformation::supportedLayers(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setSupportedLayers(const QList<int>& value)
{
    QStringList values;
    foreach (int val, value)
        values.append(QString::number(val));
    d->object->setAttribute("supportedLayers", values);
}

/*!
    Sets the state of QScreenInformation::transparencyColor() for this screen
    to \a value and cause QScreenInformation::changed() to be emitted.

    The default value, as seen by QScreenInformation::transparencyColor(),
    will be a null QColor value.

    \sa QScreenInformation::transparencyColor(), QScreenInformation::changed()
*/
void QScreenInformationProvider::setTransparencyColor(const QColor& value)
{
    d->object->setAttribute("transparencyColor", value.name());
}

/*!
    Changes the cloned screen number to \a value.  The default implementation
    does nothing.  Sub-classes should change the screen and then call
    setClonedScreen() to update the cloned screen number for clients.

    \sa setClonedScreen(), revertClonedScreen()
*/
void QScreenInformationProvider::changeClonedScreen(int value)
{
    // Nothing to do here.
    Q_UNUSED(value);
}

/*!
    Reverts the cloned screen number to its default value.  The default
    implementation does nothing.  Sub-classes should revert the screen
    and then call setClonedScreen() to update the cloned screen number
    for clients.

    \sa setClonedScreen(), changeClonedScreen()
*/
void QScreenInformationProvider::revertClonedScreen()
{
    // Nothing to do here.
}

/*!
    Changes the overlay layer number for this screen to \a value.
    The default implementation does nothing.  Sub-classes should
    change the screen and then call setLayer() to update the
    value for clients.

    Changing the layer on an overlay screen also makes it visible,
    so sub-classes should also call setVisible() to update that
    value for clients.  The layer will be made invisible again
    when revertLayer() is called.

    \sa revertLayer()
*/
void QScreenInformationProvider::changeLayer(int value)
{
    // Nothing to do here.
    Q_UNUSED(value);
}

/*!
    Reverts the overlay layer number for this screen to its
    default value and makes the layer invisible because the
    application that requested a layer change is no longer running.
    The default implementation does nothing.  Sub-classes should
    revert the screen and then call setLayer() to update the
    value for clients.

    \sa changeLayer()
*/
void QScreenInformationProvider::revertLayer()
{
    // Nothing to do here.
}

void QScreenInformationProvider::checkForCloneRevert()
{
    // If the cloneRequested value space entry changes from true to false,
    // then the application that made the request has either crashed or
    // destroyed its QScreenInformation object.
    if (d->sawCloneRequest && !d->cloneRequested->value("", false).toBool()) {
        d->sawCloneRequest = false;
        revertClonedScreen();
    }
}

void QScreenInformationProvider::checkForLayerRevert()
{
    // If the layerRequested value space entry changes from true to false,
    // then the application that made the request has either crashed or
    // destroyed its QScreenInformation object.
    if (d->sawLayerRequest && !d->layerRequested->value("", false).toBool()) {
        d->sawLayerRequest = false;
        revertLayer();
    }
}

void QScreenInformationProvider::setClonedMessage(int value)
{
    // We have received a request to change the cloned screen number.
    // If the cloneRequested value has already gone false, then the
    // application probably crashed before we got the request.  In that
    // case, we revert the cloned screen rather than change it.
    if (!d->cloneRequested->value("", false).toBool()) {
        d->sawCloneRequest = false;
        revertClonedScreen();
    } else {
        d->sawCloneRequest = true;
        changeClonedScreen(value);
    }
}

void QScreenInformationProvider::setLayerMessage(int value)
{
    // We have received a request to change the layer number.
    // If the layerRequested value has already gone false, then the
    // application probably crashed before we got the request.  In that
    // case, we revert the layer rather than change it.
    if (!d->layerRequested->value("", false).toBool()) {
        d->sawLayerRequest = false;
        revertLayer();
    } else {
        d->sawLayerRequest = true;
        changeLayer(value);
    }
}
