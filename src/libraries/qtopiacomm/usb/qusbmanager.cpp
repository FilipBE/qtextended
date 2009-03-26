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

#include "qusbmanager.h"
#include "qusbgadget.h"

#include <qtopialog.h>

#include <QSettings>
#include <QValueSpaceItem>

#include <QDebug>

/*!
    \class QUsbManager
    \inpublicgroup QtBaseModule


    \brief The QUsbManager class managers all available USB gadgets providers.

    \sa QUsbGadget, UsbGadgetTask

    \ingroup hardware
*/

/*!
    \fn QUsbManager::deactivateCompleted()

    This signal is emitted when all gadgets that have been deactivated through this
    manager object are deactivated.

    \sa deactivateGadgets()
*/

/*!
    \fn QUsbManager::deactivateAborted()

    This signal is emitted if a gadget fails to deactivate.

    \sa deactivateGadgets()
*/

/*!
    \fn QUsbManager::cableConnectedChanged(bool connected)

    This signal is emitted when the connection state of the USB cable changes.

    \a connected will be set to true when the USB cable is connected, false otherwise.

    \sa cableConnected()
*/

class QUsbManagerPrivate
{
public:
    QList<QUsbGadget *> m_deactivate;
    QValueSpaceItem *m_cableConnected;
};

/*!
    Constructs a new QUsbManager object and attach it to \a parent.  The prefered
    method of constructing a QUsbManager object is with the instance().
*/
QUsbManager::QUsbManager(QObject *parent)
    : QAbstractIpcInterfaceGroupManager("/Hardware/UsbGadget", parent)
{
    d = new QUsbManagerPrivate;

    d->m_cableConnected = new QValueSpaceItem("/Hardware/UsbGadget/cableConnected");
    QObject::connect(d->m_cableConnected, SIGNAL(contentsChanged()),
                        this, SLOT(cableConnectedChanged()));
}

/*!
    Destructor.
*/
QUsbManager::~QUsbManager()
{
    delete d;
}

/*!
    Returns list of the active gadgets.

    It is the callers responsibility to free the memory used by the
    \l {QUsbGadget}{QUsbGadgets} in the list.
*/
QList<QUsbGadget *> QUsbManager::activeGadgets()
{
    QList<QUsbGadget *> list;

    foreach (QString group, groups()) {
        foreach (QString interface, interfaces(group)) {
            QUsbGadget *gadget = new QUsbGadget(interface, group);
            if (gadget->active())
                list << gadget;
            else
                delete gadget;
        }
    }

    return list;
}

/*!
    Deactivates all active gadgets.  The \l deactivateCompleted() signal is emitted
    once all gadgets are deactivated.  If a gadget fails to deactivate the
    \l deactivateAborted() signal is emitted.
*/
void QUsbManager::deactivateGadgets()
{
    foreach (QString group, groups()) {
        foreach (QString interface, interfaces(group)) {
            QUsbGadget *gadget = new QUsbGadget(interface, group);
            if (gadget->active()) {
                d->m_deactivate << gadget;
                connect(gadget, SIGNAL(deactivated()), this, SLOT(gadgetDeactivated()));
                connect(gadget, SIGNAL(deactivateFailed()), this, SLOT(gadgetActivated()));
                gadget->deactivate();
            } else {
                delete gadget;
            }
        }
    }

    if (d->m_deactivate.isEmpty())
        emit deactivateCompleted();
}

/*!
    Returns true if the \a gadget can be loaded.  That is if no other gadget is currently
    active, or a compatible composite configuration would result if \a gadget is loaded.

    Typically only a single gadget driver can be loaded at a time. However, some hardware
    supports loading multple gadget drivers at the same time resulting in a composite gadget.
    Valid composite gadget configurations are defined in the \c {Trolltech/Usb.conf}
    configuration file.  The \c {PeripheralController/CompositeGadgets} configuration setting
    lists the group names that define valid composite gadget configurations.  The \c {Gadgets}
    setting under each composite group specifies which gadget drivers form the composite
    configuration.

    For example, the following configuration file defines two composite gadget
    configurations. The first with the Ethernet and Storage gadgets and the second with
    the Ethernet and Serial gadgets.

    \code
        [PeripheralController]
        SupportedGadgets="Ethernet,Storage,Serial"
        CompositeGadgets="Composite0,Composite1"

        [Ethernet]
        ...

        [Storage]
        ...

        [Serial]
        ...

        [Composite0]
        Gadgets="Ethernet,Storage"

        [Composite1]
        Gadgets="Ethernet,Serial"
    \endcode
*/
bool QUsbManager::canActivate(const QByteArray &gadget)
{
    QSet<QByteArray> activeGadgets;

    // create a QSet of the current active gadgets
    foreach (QString group, groups()) {
        foreach (QString interface, interfaces(group)) {
            QUsbGadget *gadget = new QUsbGadget(interface, group);

            if (gadget->active())
                activeGadgets << gadget->gadget();

            delete gadget;
        }
    }

    // no other gadget is loaded, return true
    if (activeGadgets.count() == 0)
        return true;

    // add gadget to the current active gadget set
    activeGadgets << gadget;

    // read in all composite configurations from Usb.conf, and create a QSet for each
    QSettings settings("Trolltech", "Usb");

    QList<QByteArray> compositeConfigurations =
        settings.value("PeripheralController/CompositeGadgets").toByteArray().split(',');

    foreach (QByteArray composite, compositeConfigurations) {
        settings.beginGroup(composite);

        QSet<QByteArray> compositeSet = QSet<QByteArray>::fromList(settings.value("Gadgets").toByteArray().split(','));

        settings.endGroup();

        // if active gadget set is a subset of any allowable composite configuration return true
        compositeSet.intersect(activeGadgets);
        if (compositeSet == activeGadgets)
            return true;
    }

    // else return false
    return false;
}

/*!
    Returns true if the USB cable is connected.

    This corresponds to the value space location \c {/Hardware/UsbGadget/cableConnected}.

    \sa cableConnectedChanged()
*/
bool QUsbManager::cableConnected() const
{
    return d->m_cableConnected->value().toBool();
}

/*!
    \internal
*/
void QUsbManager::gadgetDeactivated()
{
    foreach (QUsbGadget *gadget, d->m_deactivate) {
        if (!gadget->active()) {
            d->m_deactivate.removeAll(gadget);
            delete gadget;
        }
    }
    if (d->m_deactivate.isEmpty())
        emit deactivateCompleted();
}

/*!
    \internal
*/
void QUsbManager::gadgetActivated()
{
    foreach (QUsbGadget *gadget, d->m_deactivate) {
        d->m_deactivate.removeAll(gadget);
        delete gadget;
    }

    emit deactivateAborted();
}

/*!
    \internal
*/
void QUsbManager::cableConnectedChanged()
{
    emit cableConnectedChanged(d->m_cableConnected->value().toBool());
}

#include "qusbmanager.moc"

