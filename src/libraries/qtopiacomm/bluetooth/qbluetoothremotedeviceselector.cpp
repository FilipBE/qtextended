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

#include "qbluetoothremotedeviceselector_p.h"

#include <qbluetoothremotedevice.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothaddress.h>
#include "qbluetoothnamespace_p.h"

#include <qtopianamespace.h>
#include <qtopiaapplication.h>
#include <qtopiaitemdelegate.h>

#include <QHash>
#include <QVBoxLayout>
#include <QListView>
#include <QAbstractListModel>
#include <QPainter>
#include <QPainterPath>


class RemoteDeviceData
{
public:
    RemoteDeviceData(const QBluetoothAddress &address);
    void setName(const QString &name);
    void setAlias(const QString &alias);
    bool updateLabel(QBluetoothRemoteDeviceSelector::DisplayFlags flags);
    bool updateIcon(QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses);

    QBluetoothAddress m_address;

    QString m_label;
    QFont m_labelFont;
    QString m_name;
    QString m_alias;

    QIcon m_deviceIcon;
    QBluetooth::DeviceMajor m_major;
    quint8 m_minor;
    QBluetooth::ServiceClasses m_serviceClasses;

    bool m_paired;
    bool m_connected;
    //int m_rssi;

private:
    bool m_canDisplayName;
};

RemoteDeviceData::RemoteDeviceData(const QBluetoothAddress &address)
    : m_address(address),
      m_major(QBluetooth::Uncategorized),
      m_minor(0),
      m_serviceClasses(0),
      m_paired(false),
      m_connected(false),
      m_canDisplayName(false)
{
}

void RemoteDeviceData::setName(const QString &name)
{
    if (name != m_name) {
        m_name = name;
        if (!name.isEmpty()) {
            QVariant v = Qtopia::findDisplayFont(name);
            if (v.isNull()) {
                m_labelFont = QFont();
                m_canDisplayName = false;
            } else {
                m_labelFont = v.value<QFont>();
                m_canDisplayName = true;
            }
        }
    }
}

void RemoteDeviceData::setAlias(const QString &alias)
{
    if (alias != m_alias) {
        m_alias = alias;
        m_labelFont = QFont();
    }
}

bool RemoteDeviceData::updateLabel(QBluetoothRemoteDeviceSelector::DisplayFlags flags)
{
    QString newLabel;
    if (flags & QBluetoothRemoteDeviceSelector::Alias && !m_alias.isEmpty())
        newLabel = m_alias;
    else if (flags & QBluetoothRemoteDeviceSelector::Name && !m_name.isEmpty() && m_canDisplayName)
        newLabel = m_name;
    else
        newLabel = m_address.toString();

    if (newLabel != m_label) {
        m_label = newLabel;
        return true;
    }
    return false;
}

bool RemoteDeviceData::updateIcon(QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses)
{
    if (major == m_major && minor == m_minor && serviceClasses == m_serviceClasses)
        return false;

    m_major = major;
    m_minor = minor;
    m_serviceClasses = serviceClasses;
    m_deviceIcon = find_device_icon(major, minor, serviceClasses);
    return true;
}


//===================================================================


class RemoteDeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    RemoteDeviceModel(QBluetoothLocalDevice *local,
                      QBluetoothRemoteDeviceSelector::DisplayFlags displayFlags,
                      QObject *parent = 0);
    ~RemoteDeviceModel();

    bool addDevice(const QBluetoothRemoteDevice &device);
    void updateDevice(const QBluetoothRemoteDevice &device,
                      QBluetoothRemoteDeviceSelector::DisplayFlags flags);
    bool removeDevice(const QBluetoothAddress &address);
    void clear();

    QModelIndex indexFromAddress(const QBluetoothAddress &addr) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;


    QBluetoothRemoteDeviceSelector::DisplayFlags m_displayFlags;
    QPointer<QBluetoothLocalDevice> m_local;
    QHash<QBluetoothAddress, int> m_rows;
    QList<RemoteDeviceData*> m_items;

private slots:
    void remoteNameUpdated(const QBluetoothAddress &addr, const QString &name);
    void remoteAliasChanged(const QBluetoothAddress &addr, const QString &alias = QString());
    void remoteClassUpdated(const QBluetoothAddress &address,
        QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses);
    void remoteDeviceConnected(const QBluetoothAddress &addr);
    void remoteDeviceDisconnected(const QBluetoothAddress &addr);
    void pairingCreated(const QBluetoothAddress &address);
    void pairingRemoved(const QBluetoothAddress &address);

private:
    bool updateDeviceItem(RemoteDeviceData *data, const QBluetoothRemoteDevice &device, QBluetoothRemoteDeviceSelector::DisplayFlags flags);
    void initConnectedIcon();

    QIcon m_pairedIcon;
    QPixmap m_connectedPixmap;
};

RemoteDeviceModel::RemoteDeviceModel(QBluetoothLocalDevice *local, QBluetoothRemoteDeviceSelector::DisplayFlags displayFlags, QObject *parent)
    : QAbstractListModel(parent),
      m_displayFlags(displayFlags),
      m_local(local)
{
    if (!m_local)
        m_local = new QBluetoothLocalDevice(this);

    if (displayFlags & QBluetoothRemoteDeviceSelector::DeviceIcon) {
        connect(m_local, SIGNAL(remoteClassUpdated(const QBluetoothAddress &,
                    QBluetooth::DeviceMajor, quint8, QBluetooth::ServiceClasses)),
                SLOT(remoteClassUpdated(const QBluetoothAddress &,
                    QBluetooth::DeviceMajor, quint8, QBluetooth::ServiceClasses)));
    }
    if (displayFlags & QBluetoothRemoteDeviceSelector::Name) {
        connect(m_local, SIGNAL(remoteNameUpdated(QBluetoothAddress,QString)),
                SLOT(remoteNameUpdated(QBluetoothAddress,QString)));
    }
    if (displayFlags & QBluetoothRemoteDeviceSelector::Alias) {
        connect(m_local, SIGNAL(remoteAliasChanged(QBluetoothAddress,QString)),
                SLOT(remoteAliasChanged(QBluetoothAddress,QString)));
        connect(m_local, SIGNAL(remoteAliasRemoved(QBluetoothAddress)),
                SLOT(remoteAliasChanged(QBluetoothAddress)));
    }
    if (displayFlags & QBluetoothRemoteDeviceSelector::PairingStatus) {
        m_pairedIcon = QIcon(":image/bluetooth/paired");
        connect(m_local, SIGNAL(pairingCreated(QBluetoothAddress)),
                SLOT(pairingCreated(QBluetoothAddress)));
        connect(m_local, SIGNAL(pairingRemoved(QBluetoothAddress)),
                SLOT(pairingRemoved(QBluetoothAddress)));
    }

    if (displayFlags & QBluetoothRemoteDeviceSelector::ConnectionStatus) {
        connect(m_local, SIGNAL(remoteDeviceConnected(QBluetoothAddress)),
                SLOT(remoteDeviceConnected(QBluetoothAddress)));
        connect(m_local, SIGNAL(remoteDeviceDisconnected(QBluetoothAddress)),
                SLOT(remoteDeviceDisconnected(QBluetoothAddress)));
    }
}

RemoteDeviceModel::~RemoteDeviceModel()
{
    while (m_items.size() > 0)
        delete m_items.takeLast();
}

QModelIndex RemoteDeviceModel::indexFromAddress(const QBluetoothAddress &addr) const
{
    int row = m_rows.value(addr, -1);
    if (row != -1)
        return index(row, 0);
    return QModelIndex();
}

bool RemoteDeviceModel::addDevice(const QBluetoothRemoteDevice &device)
{
    QBluetoothAddress addr = device.address();
    if (m_rows.contains(addr))
        return false;

    RemoteDeviceData *data = new RemoteDeviceData(addr);
    updateDeviceItem(data, device, m_displayFlags);

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_rows.insert(addr, m_items.count());
    m_items.append(data);
    endInsertRows();
    return true;
}

void RemoteDeviceModel::updateDevice(const QBluetoothRemoteDevice &device, QBluetoothRemoteDeviceSelector::DisplayFlags flags)
{
    QBluetoothAddress addr = device.address();
    int index = m_rows.value(addr, -1);
    if (index != -1) {
        if (updateDeviceItem(m_items[index], device, flags)) {
            QModelIndex modelIndex = indexFromAddress(addr);
            emit dataChanged(modelIndex, modelIndex);
        }
    }
}

bool RemoteDeviceModel::updateDeviceItem(RemoteDeviceData *data, const QBluetoothRemoteDevice &device, QBluetoothRemoteDeviceSelector::DisplayFlags flags)
{
    if (!data || !m_local)
        return false;

    bool needDisplayUpdate = false;
    QBluetoothAddress addr = device.address();

    if (flags & QBluetoothRemoteDeviceSelector::Name)
        data->setName(device.name());
    if (flags & QBluetoothRemoteDeviceSelector::Alias)
        data->setAlias(m_local->remoteAlias(addr));
    if (data->updateLabel(flags))
        needDisplayUpdate = true;

    if (flags & QBluetoothRemoteDeviceSelector::DeviceIcon) {
        if (data->updateIcon(device.deviceMajor(), device.deviceMinor(),
                device.serviceClasses())) {
            needDisplayUpdate = true;
        }
    }

    if (flags & QBluetoothRemoteDeviceSelector::PairingStatus) {
        bool paired = m_local->isPaired(addr);
        if (data->m_paired != paired) {
            data->m_paired = paired;
            needDisplayUpdate = true;
        }
    }

    if (flags & QBluetoothRemoteDeviceSelector::ConnectionStatus) {
        bool connected = m_local->isConnected(addr);
        if (data->m_connected != connected) {
            data->m_connected = connected;
            needDisplayUpdate = true;
            if (data->m_connected && m_connectedPixmap.isNull())
                initConnectedIcon();
        }
    }

    return needDisplayUpdate;
}

bool RemoteDeviceModel::removeDevice(const QBluetoothAddress &address)
{
    int row = m_rows.value(address, -1);
    if (row == -1)
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_rows.remove(address);
    delete m_items.takeAt(row);

    // update row for all devices listed after the removed device
    RemoteDeviceData *data;
    for (int r=row; r<m_items.size(); r++) {
        data = m_items[r];
        m_rows[data->m_address] = r;
    }
    endInsertRows();
    return true;
}

void RemoteDeviceModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_rows.clear();
    while (m_items.size() > 0)
        delete m_items.takeLast();
    endInsertRows();
}

int RemoteDeviceModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_items.count();
}

int RemoteDeviceModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant RemoteDeviceModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || row >= m_items.count())
        return QVariant();

    RemoteDeviceData *data = m_items[row];
    if (!data)
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return data->m_label;
        case Qt::DecorationRole:
            if (m_displayFlags & QBluetoothRemoteDeviceSelector::DeviceIcon)
                return data->m_deviceIcon;
        case Qt::FontRole:
            if (m_displayFlags & QBluetoothRemoteDeviceSelector::ConnectionStatus)
                data->m_labelFont.setBold(data->m_connected);
            return data->m_labelFont;
        case Qt::SizeHintRole:
            // must set size if icon will be displayed, or rows will be sized
            // according to text height and won't grow when icons are added
            if (m_displayFlags & QBluetoothRemoteDeviceSelector::DeviceIcon) {
                int extent = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
                int margin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
                return QSize(extent + margin*2, extent + margin*2);
            }
        case Qtopia::AdditionalDecorationRole:
        {
            bool showPaired = (m_displayFlags & QBluetoothRemoteDeviceSelector::PairingStatus &&
                    data->m_paired);
            bool showConnected = (m_displayFlags & QBluetoothRemoteDeviceSelector::ConnectionStatus &&
                    data->m_connected);
            // if connected and paired, prefer connected icon to paired icon
            if (showConnected)
                return m_connectedPixmap;
            else if (showPaired)
                return m_pairedIcon;
        }
        default:
            break;
    }

    return QVariant();
}

//----------- slots for dynamic item updates: ----------

void RemoteDeviceModel::remoteAliasChanged(const QBluetoothAddress &addr, const QString &alias)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        data->setAlias(alias);
        if (data->updateLabel(m_displayFlags))
            emit dataChanged(i, i);
    }
}

void RemoteDeviceModel::remoteNameUpdated(const QBluetoothAddress &addr, const QString &name)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        data->setName(name);
        if (data->updateLabel(m_displayFlags))
            emit dataChanged(i, i);
    }
}

void RemoteDeviceModel::remoteClassUpdated(const QBluetoothAddress &addr,
        QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        if (data->updateIcon(major, minor, serviceClasses))
            emit dataChanged(i, i);
    }
}

void RemoteDeviceModel::remoteDeviceConnected(const QBluetoothAddress &addr)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        if (!data->m_connected) {
            data->m_connected = true;
            if (m_connectedPixmap.isNull())
                initConnectedIcon();
            emit dataChanged(i, i);
        }
    }
}

void RemoteDeviceModel::remoteDeviceDisconnected(const QBluetoothAddress &addr)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        if (data->m_connected) {
            data->m_connected = false;
            emit dataChanged(i, i);
        }
    }
}

void RemoteDeviceModel::pairingCreated(const QBluetoothAddress &addr)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        if (!data->m_paired) {
            data->m_paired = true;
            emit dataChanged(i, i);
        }
    }
}

void RemoteDeviceModel::pairingRemoved(const QBluetoothAddress &addr)
{
    QModelIndex i = indexFromAddress(addr);
    if (i.isValid()) {
        RemoteDeviceData *data = m_items[i.row()];
        if (data->m_paired) {
            data->m_paired = false;
            emit dataChanged(i, i);
        }
    }
}

//-----------------------------------------------------

void RemoteDeviceModel::initConnectedIcon()
{
    int extent = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) / 2;
    m_connectedPixmap = QIcon(":icon/bluetooth/bluetooth-connected").pixmap(extent, extent);
}


//==================================================================

class QBluetoothRemoteDeviceSelectorPrivate : public QObject
{
    Q_OBJECT
public:
    QBluetoothRemoteDeviceSelectorPrivate(QBluetoothRemoteDeviceSelector *parent,
                                          QBluetoothLocalDevice *local,
                                          QBluetoothRemoteDeviceSelector::DisplayFlags flags);
    QBluetoothAddress selectedDevice() const;

    QBluetoothRemoteDeviceSelector *m_parent;
    QListView *m_view;
    RemoteDeviceModel *m_model;

private slots:
    void activated(const QModelIndex &index);
    void currentRowChanged(const QModelIndex &, const QModelIndex &);

private:
    void initView();
    QBluetoothAddress m_lastSelectedAddr;
};

QBluetoothRemoteDeviceSelectorPrivate::QBluetoothRemoteDeviceSelectorPrivate(QBluetoothRemoteDeviceSelector *parent, QBluetoothLocalDevice *local, QBluetoothRemoteDeviceSelector::DisplayFlags displayFlags)
    : QObject(parent),
      m_parent(parent)
{
    m_view = new QListView;
    m_model = new RemoteDeviceModel(local, displayFlags, this);
    m_view->setModel(m_model);

    initView();
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect(m_view, SIGNAL(activated(QModelIndex)),
            SLOT(activated(QModelIndex)));

    QVBoxLayout *layout = new QVBoxLayout(parent);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_view);
}

void QBluetoothRemoteDeviceSelectorPrivate::initView()
{
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setItemDelegate(new QtopiaItemDelegate(this));
    m_view->setUniformItemSizes(true);

    int dimens = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    m_view->setIconSize(QSize(dimens, dimens));

    /*
    QHeaderView *header = m_view->horizontalHeader();
    header->setResizeMode(0, QHeaderView::Stretch);
    if (header->count() > 1) {
        header->setResizeMode(1, QHeaderView::Fixed);
        header->setResizeMode(1, QHeaderView::ResizeToContents);
    }
    */
}

QBluetoothAddress QBluetoothRemoteDeviceSelectorPrivate::selectedDevice() const
{
    QModelIndex index = m_view->selectionModel()->currentIndex();
    if (index.isValid() && index.row() < m_model->m_items.count())
        return m_model->m_items[index.row()]->m_address;

    return QBluetoothAddress();
}

void QBluetoothRemoteDeviceSelectorPrivate::activated(const QModelIndex &index)
{
    if (index.isValid() && index.row() < m_model->m_items.count())
        emit m_parent->activated(m_model->m_items[index.row()]->m_address);
}

void QBluetoothRemoteDeviceSelectorPrivate::currentRowChanged(const QModelIndex &, const QModelIndex &)
{
    QBluetoothAddress addr = selectedDevice();
    if (addr != m_lastSelectedAddr) {
        m_lastSelectedAddr = addr;
        emit m_parent->selectionChanged();
    }
}


//==================================================================

/*!
    \internal
    \class QBluetoothRemoteDeviceSelector
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRemoteDeviceSelector class provides a widget that allows the user to select a bluetooth device from a group of devices.

    This provides consistent presentation of a group of bluetooth devices when
    you need to show a group of devices to the user, e.g. when you want to
    show a list of devices, or if you need the user to select a particular
    device.

    This class is used in QBluetoothRemoteDeviceDialog to separate
    the UI elements from the functionality for device discovery, etc. It
    is also used in the Bluetooth settings application.

    It is somewhat like a QListWidget that can only contain remote Bluetooth
    devices: you can insert() or remove() a device, check whether the display
    contains() a device, set the currently selected device, and so on.

    \ingroup qtopiabluetooth
*/

/*!
    \enum QBluetoothRemoteDeviceSelector::DisplayFlag
    \brief Describes the attributes that should be displayed for devices in the selector. These flags can also be used to update the display in update().

    \value DeviceIcon Display an icon for each device, according to the device's class of device information.
    \value Name Display the name of each device.
    \value Alias Display the alias of each device.
    \value PairingStatus Display an icon that indicates whether a remote device is paired with the local bluetooth device. (If the ConnectionStatus flag is also set, and a remote device is both paired and connected, the connection status icon will be displayed in preference to the pairing status icon.)
    \value ConnectionStatus If a remote device is connected to the local device, display an icon that indicates the remote device is connected, and display the remote device name in bold text.

    If neither the Name nor Alias flags are set, or if they are set but a
    a device does not have a name or alias, the address of the device will be
    used as its textual description instead.

    If both the Name and Alias flags are set, the alias will be shown in
    preference to the name.

    \sa QBluetoothLocalDevice::setRemoteAlias()
*/


/*!
    Constructs a device selector with \a parent and \a flags. The display
    will be constructed according to \a displayFlags and \a local will be used
    to query the local bluetooth device settings. If \a local is 0, the
    default device will be used.
*/
QBluetoothRemoteDeviceSelector::QBluetoothRemoteDeviceSelector(DisplayFlags displayFlags, QBluetoothLocalDevice *local, QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags),
      m_data(new QBluetoothRemoteDeviceSelectorPrivate(this, local, displayFlags))
{
}

/*!
    Constructs a device selector with \a parent and \a flags. The display
    will be constructed with all display flags enabled.
*/
QBluetoothRemoteDeviceSelector::QBluetoothRemoteDeviceSelector(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    DisplayFlags displayFlags = DeviceIcon | Name | Alias | PairingStatus | ConnectionStatus;
    QBluetoothLocalDevice *local = new QBluetoothLocalDevice(this);
    m_data = new QBluetoothRemoteDeviceSelectorPrivate(this, local, displayFlags);
}

/*!
    Destroys the selector.
*/
QBluetoothRemoteDeviceSelector::~QBluetoothRemoteDeviceSelector()
{
    delete m_data;
}

/*!
    Returns the selector's display flags, as given in the constructor.
*/
QBluetoothRemoteDeviceSelector::DisplayFlags QBluetoothRemoteDeviceSelector::flags() const
{
    return m_data->m_model->m_displayFlags;
}

/*!
    Adds the remote bluetooth \a device to the display, if it has not
    already been added.

    Returns \c true if the device was added, or \c false if it has previously
    been added and thus was not added again.
*/
bool QBluetoothRemoteDeviceSelector::insert(const QBluetoothRemoteDevice &device)
{
    return m_data->m_model->addDevice(device);
}

/*!
    Updates the selector's display for \a device for the attributes described
    by \a flags. For example, if the flags match the DeviceIcon flag, this
    will update the device type icon for the matching device in the selector,
    according to the class of device information in \a device.

    You will probably only need to call this if you called insert() with a
    device value that was missing some of its field values, and you want
    to update the display now that the device values have changed. For example:

    \code
    // We have some device we want to add to the selector, but we only know
    // the device's address.
    QBluetoothRemoteDevice device("00:11:22:aa:bb::cc");

    QBluetoothRemoteDeviceSelector selector;
    selector.insert(device);

    // Since the device was only created with an address, its name() won't
    // have been filled in, and so the selector won't be showing its name
    // the selector at the moment, since it didn't know the name. So we'll
    // update it so that it does.
    QBluetoothLocalDevice local;
    local.updateRemoteDevice(device);     // fills in the devie's name attribute
    selector.update(device, QBluetoothRemoteDeviceSelector::Name);
    \endcode

    You don't need to call this method for value changes that are accessible
    from the signals in QBluetoothLocalDevice. For example, if the
    QBluetoothLocalDevice::pairingCreated() signal is emitted, and
    the PairingStatus flag was used in constructing the selector, then the
    selector will automatically update the display for that device to show
    that it is now paired.
*/
void QBluetoothRemoteDeviceSelector::update(const QBluetoothRemoteDevice &device, DisplayFlags flags)
{
    m_data->m_model->updateDevice(device, flags);
}

/*!
    \overload
    Updates the selector's display for \a device for the attributes returned
    by flags().
*/
void QBluetoothRemoteDeviceSelector::update(const QBluetoothRemoteDevice &device)
{
    m_data->m_model->updateDevice(device, m_data->m_model->m_displayFlags);
}


/*!
    Removes the device \a address from the display.

    Returns \c false if the device was not present in the display and thus
    could not be removed; otherwise returns true.
*/
bool QBluetoothRemoteDeviceSelector::remove(const QBluetoothAddress &address)
{
    return m_data->m_model->removeDevice(address);
}

/*!
    Returns \c true if the device with \a address has been added to the selector;
    otherwise returns \c false.
*/
bool QBluetoothRemoteDeviceSelector::contains(const QBluetoothAddress &address) const
{
    return m_data->m_model->m_rows.contains(address);
}

/*!
    Returns the number of devices in the selector.
*/
int QBluetoothRemoteDeviceSelector::count() const
{
    return m_data->m_model->m_items.count();
}

/*!
    Returns a list of the devices in the selector.
*/
QList<QBluetoothAddress> QBluetoothRemoteDeviceSelector::devices() const
{
    return m_data->m_model->m_rows.keys();
}

/*!
    Returns the device that is currently selected in the selector.
*/
QBluetoothAddress QBluetoothRemoteDeviceSelector::selectedDevice() const
{
    return m_data->selectedDevice();
}

/*!
    Removes all devices from the selector.
*/
void QBluetoothRemoteDeviceSelector::clear()
{
    m_data->m_model->clear();
    m_data->m_view->selectionModel()->clear(); // emits currentChanged()
}

/*!
    Sets the currently selected device to the device with \a address.
*/
void QBluetoothRemoteDeviceSelector::selectDevice(const QBluetoothAddress &address)
{
    QModelIndex i = m_data->m_model->indexFromAddress(address);
    if (i.isValid())
        m_data->m_view->setCurrentIndex(i);
}

/*!
    Returns whether the device with \a address is currently hidden.
*/
bool QBluetoothRemoteDeviceSelector::isDeviceHidden(const QBluetoothAddress &address) const
{
    QModelIndex i = m_data->m_model->indexFromAddress(address);
    if (i.isValid())
        return m_data->m_view->isRowHidden(i.row());
    return false;    
}

/*!
    Hides the device with \a address.

    \sa showDevice(), isDeviceHidden()
*/
void QBluetoothRemoteDeviceSelector::hideDevice(const QBluetoothAddress &address)
{
    QModelIndex i = m_data->m_model->indexFromAddress(address);
    if (i.isValid())
        m_data->m_view->setRowHidden(i.row(), true);
}

/*!
    Shows the device with \a address.

    \sa hideDevice(), isDeviceHidden()
*/
void QBluetoothRemoteDeviceSelector::showDevice(const QBluetoothAddress &address)
{
    QModelIndex i = m_data->m_model->indexFromAddress(address);
    if (i.isValid())
        m_data->m_view->setRowHidden(i.row(), false);
}


Q_IMPLEMENT_USER_METATYPE_ENUM(QBluetoothRemoteDeviceSelector::DisplayFlag)

#include "qbluetoothremotedeviceselector.moc"
