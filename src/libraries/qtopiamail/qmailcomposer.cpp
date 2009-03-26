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

#include "qmailcomposer.h"

#include <QApplication>
#include <QIcon>
#include <QMap>
#include <QWidget>
#include <qcontent.h>
#include <qtopialog.h>
#include <qpluginmanager.h>
#include "qmailaccount.h"
#include "qmailmessage.h"
#include "qmailcomposerplugin.h"

#define PLUGIN_KEY "composers"

// Previously, we used plugins as a general extensibility mechanism for
// adding composer types.  Now, we will use this mechanism to instead control
// which parts of code are loaded, and when.

// Instead of querying the plugins for their meta-data, we will codify the
// ones we know about.  Unknown plugins can no longer be accessed.

class ComposerPluginDescriptor
{
public:
    ComposerPluginDescriptor(QPluginManager& manager) : pluginManager(manager), pluginInterface(0) {}
    virtual ~ComposerPluginDescriptor() {}

    // Support the interface of QMailComposerPluginInterface
    virtual QString key() const = 0;

    virtual QList<QMailMessage::MessageType> messageTypes() const = 0;

    virtual QList<QMailMessage::ContentType> contentTypes() const = 0;

    virtual QString name(QMailMessage::MessageType type) const = 0;

    virtual QString displayName(QMailMessage::MessageType type) const = 0;

    virtual QIcon displayIcon(QMailMessage::MessageType type) const = 0;

    virtual bool isSupported(QMailMessage::MessageType t, QMailMessage::ContentType c = QMailMessage::NoContent) const
    {
        bool supportsMessageType(t == QMailMessage::AnyType || messageTypes().contains(t));
        bool supportsContentType(c == QMailMessage::NoContent || contentTypes().contains(c));

        return (supportsMessageType && supportsContentType);
    }

    // Load the plugin if necessary and create a composer object
    virtual QMailComposerInterface* create(QWidget* parent)
    {
        if (!pluginInterface)
        {
            if (QObject* plugin = pluginManager.instance(pluginFilename()))
                pluginInterface = qobject_cast<QMailComposerPluginInterface*>(plugin);
        }

        return pluginInterface ? pluginInterface->create(parent) : 0;
    }

private:
    virtual QString pluginFilename() const
    {
        // The plugin manager knows how to find libraries with this name on each platform
        return key().toLower();
    }

    QPluginManager& pluginManager;
    QMailComposerPluginInterface* pluginInterface;
};

// Describe the plugins we know about
class EmailComposerPluginDescriptor : public ComposerPluginDescriptor
{
public:
    EmailComposerPluginDescriptor(QPluginManager& manager) : ComposerPluginDescriptor(manager) {}

    static QString pluginKey() { return "EmailComposer"; }

    virtual QString key() const { return pluginKey(); }

    virtual QList<QMailMessage::MessageType> messageTypes() const
    {
        return QList<QMailMessage::MessageType>() << QMailMessage::Email;
    }

    virtual QList<QMailMessage::ContentType> contentTypes() const
    {
        return QList<QMailMessage::ContentType>() << QMailMessage::RichTextContent
                                                  << QMailMessage::PlainTextContent
                                                  << QMailMessage::VCardContent
                                                  << QMailMessage::MultipartContent;
    }

    virtual QString name(QMailMessage::MessageType) const { return qApp->translate("EmailComposerPlugin","Email"); }

    virtual QString displayName(QMailMessage::MessageType) const { return qApp->translate("EmailComposerPlugin","Email"); }

    virtual QIcon displayIcon(QMailMessage::MessageType) const { return QIcon(":icon/email"); }

};

#ifdef QTOPIA_HOMEUI

class VideomailComposerPluginDescriptor : public ComposerPluginDescriptor
{
public:
    VideomailComposerPluginDescriptor(QPluginManager& manager) : ComposerPluginDescriptor(manager) {}

    static QString pluginKey() { return "VideomailComposer"; }

    virtual QString key() const { return pluginKey(); }

    virtual QList<QMailMessage::MessageType> messageTypes() const
    {
        return QList<QMailMessage::MessageType>() << QMailMessage::Email;
    }

    virtual QList<QMailMessage::ContentType> contentTypes() const
    {
        return QList<QMailMessage::ContentType>() << QMailMessage::VideomailContent;
    }

    virtual QString name(QMailMessage::MessageType) const { return qApp->translate("VideomailComposerPlugin","Videomail"); }

    virtual QString displayName(QMailMessage::MessageType) const { return qApp->translate("VideomailComposerPlugin","Video mail"); }

    virtual QIcon displayIcon(QMailMessage::MessageType) const { return QIcon(":icon/email"); }

};

#endif //QTOPIA_HOMEUI

class GenericComposerPluginDescriptor : public ComposerPluginDescriptor
{
public:
    GenericComposerPluginDescriptor(QPluginManager& manager) : ComposerPluginDescriptor(manager) {}

    static QString pluginKey() { return "GenericComposer"; }

    virtual QString key() const { return pluginKey(); }

    virtual QList<QMailMessage::MessageType> messageTypes() const
    {
        return QList<QMailMessage::MessageType>()
#ifndef QTOPIA_NO_SMS
            << QMailMessage::Sms
#endif
            << QMailMessage::Instant;
    }

    virtual QList<QMailMessage::ContentType> contentTypes() const
    {
        return QList<QMailMessage::ContentType>()
#ifndef QTOPIA_NO_SMS
            << QMailMessage::VCardContent
#endif
            << QMailMessage::PlainTextContent;
    }

    virtual QString name(QMailMessage::MessageType type) const
    {
        return qApp->translate("GenericComposerPlugin", (type == QMailMessage::Sms ? "Text message" : "Instant message"));
    }

    virtual QString displayName(QMailMessage::MessageType) const
    {
        return qApp->translate("GenericComposerPlugin", "Message");
    }

    virtual QIcon displayIcon(QMailMessage::MessageType type) const
    {
        if (type == QMailMessage::Sms) {
            return QIcon(":icon/txt");
        }

#ifdef QTOPIA_HOMEUI
        return QIcon(":icon/home/message");
#else
        return QIcon(":icon/im");
#endif
    }
};

#ifndef QTOPIA_NO_MMS

class MMSComposerPluginDescriptor : public ComposerPluginDescriptor
{
public:
    MMSComposerPluginDescriptor(QPluginManager& manager) : ComposerPluginDescriptor(manager) {}

    static QString pluginKey() { return "MMSComposer"; }

    virtual QString key() const { return pluginKey(); }

    virtual QList<QMailMessage::MessageType> messageTypes() const
    {
        return QList<QMailMessage::MessageType>() << QMailMessage::Mms;
    }

    virtual QList<QMailMessage::ContentType> contentTypes() const
    {
        return QList<QMailMessage::ContentType>() << QMailMessage::SmilContent;
    }

    virtual QString name(QMailMessage::MessageType) const { return qApp->translate("MMSComposerPlugin","Multimedia message"); }

    virtual QString displayName(QMailMessage::MessageType) const { return qApp->translate("MMSComposerPlugin","MMS"); }

    virtual QIcon displayIcon(QMailMessage::MessageType) const { return QIcon(":icon/multimedia"); }

};

#endif

typedef QMap<QString, ComposerPluginDescriptor*> PluginMap;

// Load all the viewer plugins into a map for quicker reference
static PluginMap initMap(QPluginManager& manager)
{
    PluginMap map;

    map.insert(EmailComposerPluginDescriptor::pluginKey(), new EmailComposerPluginDescriptor(manager));
#ifdef QTOPIA_HOMEUI
    map.insert(VideomailComposerPluginDescriptor::pluginKey(), new VideomailComposerPluginDescriptor(manager));
#endif
    map.insert(GenericComposerPluginDescriptor::pluginKey(), new GenericComposerPluginDescriptor(manager));

#ifndef QTOPIA_NO_MMS
    map.insert(MMSComposerPluginDescriptor::pluginKey(), new MMSComposerPluginDescriptor(manager));
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

// Return the composer plugin object matching the specified ID
static ComposerPluginDescriptor* mapping(const QString& key)
{
    PluginMap::ConstIterator it;
    if ((it = pluginMap().find(key)) != pluginMap().end())
        return it.value();

    qWarning() << "Failed attempt to map composer:" << key;
    return 0;
}

/*!
    \class QMailComposerInterface
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailComposerInterface class defines the interface to objects that can compose a mail message.
    \ingroup messaginglibrary

    Qt Extended uses the QMailComposerInterface interface for composing mail messages.  A class may implement the
    QMailComposerInterface interface to compose a mail message format.

    The composer class may start composing with no associated message, or it may be provided with an existing
    message to edit, via the \l {QMailComposerInterface::setMessage()}{setMessage()} function.
    A client can query whether the composer object is empty with the
    \l {QMailComposerInterface::isEmpty()}{isEmpty()} function, and extract the
    composed message with the \l {QMailComposerInterface::message()}{message()} function.  If the
    message type supports attachments, these can be attached with the
    \l {QMailComposerInterface::attach()}{attach()} function.  The current state of composition can be cleared
    with the \l {QMailComposerInterface::clear()}{clear()} function.

    The composer object should emit the \l {QMailComposerInterface::changed()}{changed()} signal
    whenever the composed message changes. If composition is cancelled, the composer should emit the
    \l {QMailComposerInterface::cancel()}{cancel()} signal. When the message is ready to send, the composer should
    emit the \l {QMailComposerInterface::sendMessage()}{sendMessage()} signal. For composers which need to inform
    of state changes during composition, such as multi-page composers,
    the \l {QMailComposerInterface::contextChanged()}{contextChanged()} signal should be emitted to allow container
    objects to update their view of the \l {QMailComposerInterface::contextTitle()}{contextTitle()} string.

    Each composer class must export metadata describing itself and the messages it is able to compose.  To do
    this, the composer must implement the
    \l {QMailComposerInterface::key()}{key()},
    \l {QMailComposerInterface::messageTypes()}{messageTypes()},
    \l {QMailComposerInterface::name()}{name()},
    \l {QMailComposerInterface::displayName()}{displayName()} and
    \l {QMailComposerInterface::displayIcon()}{displayIcon()} functions.

    \code
    QString key = QMailComposerFactory::defaultKey( QMailMessage::Email );
    QMailComposerInterface* emailComposer = QMailComposerFactory::create( key, this, "emailComposer" );
    \endcode

    To allow a class to be created through the QMailComposerFactory interface, a plug-in class derived from
    QMailComposerPlugin should be implemented.

    \sa QMailComposerFactory, QMailComposerPlugin
*/

/*!
    \enum QMailComposerInterface::ComposeContext

    Identifies the desired context for message composition.

    \value Create Create a new message
    \value Reply Create a reply message to a previously received message
    \value ReplyToAll Create a reply message addressed to all recipients of a previously received message
    \value Forward Create a message that forwards an existing message
*/


/*!
    Constructs the QMailComposerInterface object with the parent widget \a parent.
*/
QMailComposerInterface::QMailComposerInterface( QWidget *parent )
    : QWidget( parent )
{
}

/*!
    Destructs the QMailComposerInterface object.
*/
QMailComposerInterface::~QMailComposerInterface()
{
}

/*!
    Returns a string identifying the composer.
*/

QString QMailComposerInterface::key() const
{
    QString val = metaObject()->className();
    val.chop(9); // remove trailing "Interface"
    return val;
}

/*!
    Returns the message types created by the composer.
*/
QList<QMailMessage::MessageType> QMailComposerInterface::messageTypes() const
{
    return mapping(key())->messageTypes();
}

/*!
    Returns the content types created by the composer.
*/
QList<QMailMessage::ContentType> QMailComposerInterface::contentTypes() const
{
    return mapping(key())->contentTypes();
}

/*!
    Returns the translated name of the message type \a type created by the composer.
*/
QString QMailComposerInterface::name(QMailMessage::MessageType type) const
{
    return mapping(key())->name(type);
}

/*!
    Returns the translated name of the message type \a type created by the composer,
    in a form suitable for display on a button or menu.
*/
QString QMailComposerInterface::displayName(QMailMessage::MessageType type) const
{
    return mapping(key())->displayName(type);
}

/*!
    Returns the icon representing the message type \a type created by the composer.
*/
QIcon QMailComposerInterface::displayIcon(QMailMessage::MessageType type) const
{
    return mapping(key())->displayIcon(type);
}

/*!
    Sets the message to contain body with \a text, if that is meaningful to the message type created by the composer.
    The text has the mime-type \a type.
*/
void QMailComposerInterface::setBody( const QString& text, const QString &type )
{
    // default implementation does nothing
    Q_UNUSED(text)
    Q_UNUSED(type)
}

/*!
    Adds \a item as an attachment to the message in the composer. The \a action parameter
    specifies what the composer should do with \a item.
*/
void QMailComposerInterface::attach( const QContent& item, QMailMessage::AttachmentsAction action )
{
    // default implementation does nothing
    Q_UNUSED(item)
    Q_UNUSED(action)
}

/*!
    Sets the composer to append \a signature to the body of the message, when creating a message.
*/
void QMailComposerInterface::setSignature( const QString& signature )
{
    // default implementation does nothing
    Q_UNUSED(signature)
}

/*!
    Sets the composer to produce a message of type \a type.
*/
void QMailComposerInterface::setMessageType( QMailMessage::MessageType type )
{
    // default implementation does nothing - override for composers supporting multiple types
    Q_UNUSED(type)
}

/*!
    \fn bool QMailComposerInterface::isEmpty() const

    Returns true if the composer contains no message content; otherwise returns false.
*/

/*!
    \fn QMailMessage QMailComposerInterface::message() const

    Returns the current content of the composer.
*/

/*!
    \fn void QMailComposerInterface::setMessage(const QMailMessage& mail)

    Presets the content of the composer to \a mail.
*/

/*!
    \fn void QMailComposerInterface::clear()

    Clears any message content contained in the composer.
*/

/*!
    \fn QString QMailComposerInterface::contextTitle() const

    Returns a string description of the current composition context.
*/

/*!
    \fn QString QMailComposerInterface::from() const

    Returns the from address string for the currently composed message.
*/

/*!
    \fn QMailAccount QMailComposerInterface::fromAccount() const

    Returns the sending account for the currently composed message or an
    invalid \c QMailAccount if no account could be set.
*/

/*!
    \fn bool QMailComposerInterface::isDetailsOnlyMode() const

    Returns \c true if the composer is in details only mode, or \c false otherwise.
    This only applies to composers which may present a different view for message
    address details entry.
*/

/*!
    \fn void QMailComposerInterface::setDetailsOnlyMode(bool val)

    If supported, sets the composer into details only mode if \a val is \c true.
    Otherwise the composer is set into normal composition mode.
*/

/*!
    \fn bool QMailComposerInterface::isReadyToSend() const

    Returns \c true if the composed message is ready to send or \c false otherwise.
*/

/*!
    \fn void QMailComposerInterface::reply(const QMailMessage& source, int type)

    Presets the content of the composer from the message \a source. The message
    may be presented differently based on the type of composition specified by \a type.
*/

/*!
    \fn void QMailComposerInterface::setDefaultAccount(const QMailAccountId& id)

    Sets the default sending account to the QMailAccount with a \c QMailAccountId \a id.
*/

/*!
    \fn void QMailComposerInterface::setFrom(const QString& fromAddress)

    Sets the composed message from address to \a fromAddress.
*/

/*!
    \fn void QMailComposerInterface::setSubject(const QString& subject)

    Sets the composed message subject to \a subject.
*/

/*!
    \fn void QMailComposerInterface::setTo(const QString& to)

    Sets the composed message recipient address to \a to.
*/

/*!
    \fn QString QMailComposerInterface::to() const

    Returns the recipient address of the composed message.
*/

/*!
    \fn void QMailComposerInterface::cancel()

    Signal that is emitted when message composition is cancelled.

    \sa changed()
*/

/*!
    \fn void QMailComposerInterface::changed()

    Signal that is emitted when the currently composed message has been changed.

    \sa cancel()
*/

/*!
    \fn void QMailComposerInterface::contextChanged()

    Signal that is emitted when the message composition context has changed. For example
    when transitioning from message body composition to message details composition in a multi page
    composer.

    \sa cancel(), changed()
*/

/*!
    \fn void QMailComposerInterface::sendMessage()

    Signal that is emitted when message composition has finished and the message is ready to send.

    \sa isReadyToSend()
*/

/*!
    \class QMailComposerFactory
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailComposerFactory class creates objects implementing the QMailComposerInterface interface.
    \ingroup messaginglibrary

    The QMailComposerFactory class creates objects that are able to compose mail messages, and
    that implement the QMailComposerInterface interface.  The factory chooses an implementation
    based on the type of message to be composed.

    The QMailComposerInterface class describes the interface supported by classes that can be created
    by the QMailComposerFactory class.  To create a new class that can be created via the QMailComposerFactory,
    implement a plug-in that derives from QMailComposerPlugin.

    \sa QMailComposerInterface, QMailComposerPlugin
*/

/*!
    Returns a list of keys identifying classes that can compose messages of type \a type containing \a contentType content.
*/
QStringList QMailComposerFactory::keys( QMailMessage::MessageType type , QMailMessage::ContentType contentType)
{
    QStringList in;

    foreach (PluginMap::mapped_type plugin, pluginMap())
        if (plugin->isSupported(type, contentType))
            in << plugin->key();

    return in;
}

/*!
    Returns the key identifying the first class found that can compose messages of type \a type.
*/
QString QMailComposerFactory::defaultKey( QMailMessage::MessageType type )
{
    QStringList list(QMailComposerFactory::keys(type));
    return (list.isEmpty() ? QString() : list.first());
}

/*!
    Returns the message types created by the composer identified by \a key.

    \sa QMailComposerInterface::messageTypes()
*/
QList<QMailMessage::MessageType> QMailComposerFactory::messageTypes( const QString& key )
{
    return mapping(key)->messageTypes();
}

/*!
    Returns the name for the message type \a type created by the composer identified by \a key.

    \sa QMailComposerInterface::name()
*/
QString QMailComposerFactory::name(const QString &key, QMailMessage::MessageType type)
{
    return mapping(key)->name(type);
}

/*!
    Returns the display name for the message type \a type created by the composer identified by \a key.

    \sa QMailComposerInterface::displayName()
*/
QString QMailComposerFactory::displayName(const QString &key, QMailMessage::MessageType type)
{
    return mapping(key)->displayName(type);
}

/*!
    Returns the display icon for the message type \a type created by the composer identified by \a key.

    \sa QMailComposerInterface::displayIcon()
*/
QIcon QMailComposerFactory::displayIcon(const QString &key, QMailMessage::MessageType type)
{
    return mapping(key)->displayIcon(type);
}

/*!
    Creates a composer object of the class identified by \a key, setting the returned object to
    have the parent widget \a parent.
*/
QMailComposerInterface *QMailComposerFactory::create( const QString& key, QWidget *parent )
{
    return mapping(key)->create(parent);
}

