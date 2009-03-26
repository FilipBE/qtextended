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

#include "qmailviewer.h"

#include <QApplication>
#include <QIcon>
#include <QMap>
#include <QWidget>

#include <qtopialog.h>
#include <qpluginmanager.h>

#include "qmailviewerplugin.h"

#define PLUGIN_KEY "viewers"

// Previously, we used plugins as a general extensibility mechanism for 
// adding viewer types.  Now, we will use this mechanism to instead control
// which parts of code are loaded, and when.

// Instead of querying the plugins for their meta-data, we will codify the
// ones we know about.  Unknown plugins can no longer be accessed.

class ViewerPluginDescriptor
{
public:
    ViewerPluginDescriptor(QPluginManager& manager) : pluginManager(manager), pluginInterface(0) {}
    virtual ~ViewerPluginDescriptor() {}

    // Support the interface of QMailViewerPluginInterface
    virtual QString key() const = 0;
    virtual QMailViewerFactory::PresentationType presentation() const = 0;
    virtual QList<int> types() const = 0;

    bool isSupported(QMailMessage::ContentType t, QMailViewerFactory::PresentationType pres) const 
    { 
        if ((pres != QMailViewerFactory::AnyPresentation) && (pres != presentation()))
            return false;

        return types().contains(t); 
    }

    // Load the plugin if necessary and create a viewer object
    virtual QMailViewerInterface* create(QWidget* parent)
    {
        if (!pluginInterface)
        {
            if (QObject* plugin = pluginManager.instance(pluginFilename()))
                pluginInterface = qobject_cast<QMailViewerPluginInterface*>(plugin);
        }

        return pluginInterface ? pluginInterface->create(parent) : 0;
    }

private:
    virtual QString pluginFilename() const
    {
        return key().toLower();
    }

    QPluginManager& pluginManager;
    QMailViewerPluginInterface* pluginInterface;
};

// Describe the plugins we know about

class GenericViewerPluginDescriptor : public ViewerPluginDescriptor
{
public:
    GenericViewerPluginDescriptor(QPluginManager& manager) : ViewerPluginDescriptor(manager) {}

    static QString pluginKey() { return "GenericViewer"; }
    virtual QString key() const { return pluginKey(); }

    virtual QMailViewerFactory::PresentationType presentation() const { return QMailViewerFactory::StandardPresentation; }

    virtual QList<int> types() const { return QList<int>() << QMailMessage::PlainTextContent
                                                           << QMailMessage::RichTextContent
                                                           << QMailMessage::ImageContent
                                                           << QMailMessage::AudioContent
                                                           << QMailMessage::VideoContent
                                                           << QMailMessage::MultipartContent
#ifndef QTOPIA_HOMEUI
                                                           << QMailMessage::VoicemailContent
                                                           << QMailMessage::VideomailContent
#endif
                                                           << QMailMessage::HtmlContent         // temporary...
                                                           << QMailMessage::VCardContent        // temporary...
                                                           << QMailMessage::VCalendarContent    // temporary...
                                                           << QMailMessage::ICalendarContent; } // temporary...
};

class SmilViewerPluginDescriptor : public ViewerPluginDescriptor
{
public:
    SmilViewerPluginDescriptor(QPluginManager& manager) : ViewerPluginDescriptor(manager) {}

    static QString pluginKey() { return "SmilViewer"; }
    virtual QString key() const { return pluginKey(); }

    virtual QMailViewerFactory::PresentationType presentation() const { return QMailViewerFactory::StandardPresentation; }

    virtual QList<int> types() const { return QList<int>() << QMailMessage::SmilContent; }
};

class ConversationViewerPluginDescriptor : public ViewerPluginDescriptor
{
public:
    ConversationViewerPluginDescriptor(QPluginManager& manager) : ViewerPluginDescriptor(manager) {}

    static QString pluginKey() { return "ConversationViewer"; }
    virtual QString key() const { return pluginKey(); }

    virtual QMailViewerFactory::PresentationType presentation() const { return QMailViewerFactory::ConversationPresentation; }

    virtual QList<int> types() const { return QList<int>() << QMailMessage::PlainTextContent; }
};

#ifdef QTOPIA_HOMEUI

class VoicemailViewerPluginDescriptor : public ViewerPluginDescriptor
{
public:
    VoicemailViewerPluginDescriptor(QPluginManager& manager) : ViewerPluginDescriptor(manager) {}

    static QString pluginKey() { return "VoicemailViewer"; }
    virtual QString key() const { return pluginKey(); }

    virtual QMailViewerFactory::PresentationType presentation() const { return QMailViewerFactory::StandardPresentation; }

    virtual QList<int> types() const { return QList<int>() << QMailMessage::VoicemailContent 
                                                           << QMailMessage::VideomailContent; }
};

#endif

typedef QMap<QString, ViewerPluginDescriptor*> PluginMap;

// Load all the viewer plugins into a map for quicker reference
static PluginMap initMap(QPluginManager& manager)
{
    PluginMap map;

    map.insert(GenericViewerPluginDescriptor::pluginKey(), new GenericViewerPluginDescriptor(manager));
#ifndef QTOPIA_NO_MMS
    map.insert(SmilViewerPluginDescriptor::pluginKey(), new SmilViewerPluginDescriptor(manager));
#endif
    map.insert(ConversationViewerPluginDescriptor::pluginKey(), new ConversationViewerPluginDescriptor(manager));
#ifdef QTOPIA_HOMEUI
    map.insert(VoicemailViewerPluginDescriptor::pluginKey(), new VoicemailViewerPluginDescriptor(manager));
#endif

    return map;
}

// Return a reference to a map containing all loaded plugin objects
static PluginMap& pluginMap()
{
    static QPluginManager pluginManager(PLUGIN_KEY);
    static PluginMap map(initMap(pluginManager));

    return map;
}

// Return the viewer plugin object matching the specified ID
static ViewerPluginDescriptor* mapping(const QString& key)
{
    PluginMap::ConstIterator it;
    if ((it = pluginMap().find(key)) != pluginMap().end())
        return it.value();

    return 0;
}

/*!
    \class QMailViewerInterface
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailViewerInterface class defines the interface to objects that can display a mail message.
    \ingroup messaginglibrary

    Qt Extended uses the QMailViewerInterface interface for displaying mail messages.  A class may implement the 
    QMailViewerInterface interface to display a mail message format.
    
    The message to be displayed is provided to the viewer class using the \l {QMailViewerInterface::setMessage()}
    {setMessage()} function.  If the message refers to external resources, these should be provided using the 
    \l {QMailViewerInterface::setResource()}{setResource()} function.  The  \l {QMailViewerInterface::clear()}{clear()}
    function clears any message or resources previously set.

    The viewer object should emit the \l {QMailViewerInterface::anchorClicked()}{anchorClicked()} signal if the user 
    selects a link in the message.  If the message supports a concept of completion, then the 
    \l {QMailViewerInterface::finished()}{finished()} signal should be emitted after the display has been completed.

    Rather than creating objects that implement the QMailViewerInterface directly, clients should create an object
    of an appropriate type by using the QMailViewerFactory class:

    \code
    QString key = QMailViewerFactory::defaultKey( QMailViewerFactory::SmilContent );
    QMailViewerInterface* smilViewer = QMailViewerFactory::create( key, this, "smilViewer" );
    \endcode

    To allow a class to be created through the QMailViewerFactory interface, a plug-in class derived from
    QMailViewerPlugin should be implemented.

    \sa QMailViewerFactory, QMailViewerPlugin
*/

/*!
    \fn bool QMailViewerInterface::setMessage(const QMailMessage& mail)

    Displays the contents of \a mail.  Returns whether the message could be successfully displayed.
*/

/*!
    \fn void QMailViewerInterface::clear()

    Resets the display to have no content, and removes any resource associations.
*/

/*!
    \fn QWidget* QMailViewerInterface::widget() const

    Returns the widget implementing the display interface.
*/

/*!
    \fn QMailViewerInterface::replyToSender()

    This signal is emitted by the viewer to initiate a reply action.
*/

/*!
    \fn QMailViewerInterface::replyToAll()

    This signal is emitted by the viewer to initiate a reply-to-all action.
*/

/*!
    \fn QMailViewerInterface::forwardMessage()

    This signal is emitted by the viewer to initiate a message forwarding action.
*/

/*!
    \fn QMailViewerInterface::completeMessage()

    This signal is emitted by the viewer to initiate a message completion action.  
    This is only meaningful if the message has not yet been completely retrieved.

    \sa QMailMessage::status(), QMailMessageServer::completeRetrieval()
*/

/*!
    \fn QMailViewerInterface::deleteMessage()

    This signal is emitted by the viewer to initiate a message deletion action.
*/

/*!
    \fn QMailViewerInterface::saveSender()

    This signal is emitted by the viewer to request that the sender's address should be saved as a Contact.
*/

/*!
    \fn QMailViewerInterface::contactDetails(const QContact &contact)

    This signal is emitted by the viewer to request a display of \a{contact}'s details.
*/

/*!
    \fn QMailViewerInterface::anchorClicked(const QUrl& link)

    This signal is emitted when the user presses the select key while the display has the 
    anchor \a link selected.
*/

/*!
    \fn QMailViewerInterface::messageChanged(const QMailMessageId &id)

    This signal is emitted by the viewer to report that it is now viewing a different message, 
    identified by \a id.
*/

/*!
    \fn QMailViewerInterface::viewMessage(const QMailMessageId &id, QMailViewerFactory::PresentationType type)

    This signal is emitted by the viewer to request a display of the message identified by \a id, 
    using the presentation style \a type.
*/

/*!
    \fn QMailViewerInterface::sendMessage(const QMailMessage &message)

    This signal is emitted by the viewer to send a new message, whose contents are held by \a message.
*/

/*!
    \fn QMailViewerInterface::finished()

    This signal is emitted when the display of the current mail message is completed.  This signal 
    is emitted only for message types that define a concept of completion, such as SMIL slideshows.
*/

/*!
    Constructs the QMailViewerInterface object with the parent widget \a parent.
*/
QMailViewerInterface::QMailViewerInterface( QWidget *parent )
    : QObject( parent )
{
}

/*! 
    Destructs the QMailViewerInterface object.
*/
QMailViewerInterface::~QMailViewerInterface()
{
}

/*!
    Scrolls the display to position the \a link within the viewable area.
*/
void QMailViewerInterface::scrollToAnchor(const QString& link)
{
    // default implementation does nothing
    Q_UNUSED(link)
}

/*!
    Allows the viewer object to add any relevant actions to the application \a menu supplied.
*/
void QMailViewerInterface::addActions( QMenu* menu ) const
{
    // default implementation does nothing
    Q_UNUSED(menu)
}

/*!
    Allows the viewer object to handle the notification of the arrival of new messages, 
    identified by \a list.

    Return true to indicate that the event has been handled, or false to let the caller handle
    the new message event.
*/
bool QMailViewerInterface::handleIncomingMessages( const QMailMessageIdList &list ) const
{
    // default implementation does nothing
    Q_UNUSED(list)
    return false;
}

/*!
    Allows the viewer object to handle the notification of the transmission of queued messages, 
    identified by \a list.

    Return true to indicate that the event has been handled, or false to let the caller handle
    the new message event.
*/
bool QMailViewerInterface::handleOutgoingMessages( const QMailMessageIdList &list ) const
{
    // default implementation does nothing
    Q_UNUSED(list)
    return false;
}

/*! 
    Supplies the viewer object with a resource that may be referenced by a mail message.  The resource 
    identified as \a name will be displayed as the object \a value.  
*/
void QMailViewerInterface::setResource(const QUrl& name, QVariant value)
{
    // default implementation does nothing
    Q_UNUSED(name)
    Q_UNUSED(value)
}


/*!
    \class QMailViewerFactory
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailViewerFactory class creates objects implementing the QMailViewerInterface interface.
    \ingroup messaginglibrary

    The QMailViewerFactory class creates objects that are able to display mail messages, and 
    implement the \c QMailViewerInterface interface.  The factory chooses an implementation
    based on the type of message to be displayed.

    The QMailViewerInterface class describes the interface supported by classes that can be created
    by the QMailViewerFactory class.  To create a new class that can be created via the QMailViewerFactory,
    implement a plug-in that derives from QMailViewerPlugin.

    \sa QMailViewerInterface, QMailViewerPlugin
*/

/*!
    \enum QMailViewerFactory::PresentationType

    This enum defines the types of presentation that message viewer components may implement.

    \value AnyPresentation Do not specify the type of presentation to be employed.
    \value StandardPresentation Present the message in the standard fashion for the relevant content type.
    \value ConversationPresentation Present the message in the context of a conversation with a contact.
    \value UserPresentation The first value that can be used for application-specific purposes.
*/

/*!
    Returns a list of keys identifying classes that can display a message containing \a type content,
    using the presentation type \a pres.
*/
QStringList QMailViewerFactory::keys(QMailMessage::ContentType type, PresentationType pres)
{
    QStringList in;

    foreach (PluginMap::mapped_type plugin, pluginMap())
        if (plugin->isSupported(type, pres))
            in << plugin->key();

    return in;
}

/*!
    Returns the key identifying the first class found that can display message containing \a type content,
    using the presentation type \a pres.
*/
QString QMailViewerFactory::defaultKey(QMailMessage::ContentType type, PresentationType pres)
{
    QStringList list(QMailViewerFactory::keys(type, pres));
    return (list.isEmpty() ? QString() : list.first());
}

/*!
    Creates a viewer object of the class identified by \a key, setting the returned object to 
    have the parent widget \a parent.
*/
QMailViewerInterface *QMailViewerFactory::create(const QString &key, QWidget *parent)
{
    return mapping(key)->create(parent);
}

