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
#include "qbluetoothremotedevicedialog_p.h"

#include <qbluetoothremotedevice.h>
#include <qbluetoothsdpuuid.h>
#include "qbluetoothremotedeviceselector_p.h"

#include <qtopiaapplication.h>
#include <qwaitwidget.h>
#include <qtopialog.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QSoftMenuBar>
#include <QMenuBar>
#include <QTimer>
#include <QActionEvent>
#include <QAbstractListModel>


class DeviceFilterModel : public QAbstractListModel
{
    Q_OBJECT
public:
    DeviceFilterModel(const QList<QBluetoothRemoteDeviceDialogFilter *> &filters, QObject *parent = 0);

    virtual int rowCount(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    const QList<QBluetoothRemoteDeviceDialogFilter *> &m_filters;
};

DeviceFilterModel::DeviceFilterModel(const QList<QBluetoothRemoteDeviceDialogFilter *> &filters, QObject *parent)
    : QAbstractListModel(parent),
      m_filters(filters)
{
}

int DeviceFilterModel::rowCount(const QModelIndex &) const
{
    return m_filters.count();
}

QVariant DeviceFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QBluetoothRemoteDeviceDialogFilter *filter = m_filters.value(index.row(), 0);
        if (filter)
            return filter->title();
    }
    return QVariant();
}


/*!
    \class QBluetoothRemoteDeviceDialogFilter
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRemoteDeviceDialogFilter class provides a filter on the devices that are displayed by a QBluetoothRemoteDeviceDialog.

    This clss allows the programmer to control whether particular devices
    should be displayed in a QBluetoothRemoteDeviceDialog.

    For example, this will create a QBluetoothRemoteDeviceDialog that only
    displays computers and phones:
    \code
    QBluetoothRemoteDeviceDialogFilter filter;
    QSet<QBluetooth::DeviceMajor> majors;
    majors.insert(QBluetooth::Computer);
    majors.insert(QBluetooth::Phone);
    filter.setAcceptedDeviceMajors(majors);

    QBluetoothRemoteDeviceDialog dialog;
    dialog.setFilter(&filter);
    \endcode

    By default the QBluetoothRemoteDeviceDialogFilter will accept all devices,
    including those with an invalid class of device information.

    \bold {Note:} The programmer should be careful when using device majors,
    device minors and service classes as a means of filtering devices, as some
    devices may not have the correct class of device information.

    \ingroup qtopiabluetooth
    \sa QBluetoothRemoteDeviceDialog
 */

struct QBluetoothRemoteDeviceDialogFilterPrivate
{
    QSet<QBluetooth::DeviceMajor> deviceMajors;
    QBluetooth::ServiceClasses serviceClasses;
    QString title;
};

/*!
    Constructs a QBluetoothRemoteDeviceDialogFilter that will accept all devices.
    The title is set to "Default".
 */
QBluetoothRemoteDeviceDialogFilter::QBluetoothRemoteDeviceDialogFilter()
    : d(new QBluetoothRemoteDeviceDialogFilterPrivate)
{
    d->serviceClasses = QBluetooth::AllServiceClasses;
    d->title = QObject::tr("Default");
}

/*!
    Constructs a QBluetoothRemoteDeviceDialogFilter with \a title, that will
    accept all devices.
 */
QBluetoothRemoteDeviceDialogFilter::QBluetoothRemoteDeviceDialogFilter(const QString &title)
    : d(new QBluetoothRemoteDeviceDialogFilterPrivate)
{
    d->serviceClasses = QBluetooth::AllServiceClasses;
    d->title = title;
}

/*!
    Destroys the dialog filter.
 */
QBluetoothRemoteDeviceDialogFilter::~QBluetoothRemoteDeviceDialogFilter()
{
    delete d;
}

/*!
    Returns the title for this filter.
*/
QString QBluetoothRemoteDeviceDialogFilter::title() const
{
    return d->title;
}

/*!
    Sets the filter to accept devices that match at least one of the
    device majors contained in \a deviceMajors. If a device does not match at
    least one of the specified device majors, it will not be displayed in
    the associated device selector.

    For example, to create a filter that only accepts computers and phones:
    \code
        QBluetoothRemoteDeviceDialogFilter filter;
        QSet<QBluetooth::DeviceMajor> majors;
        majors.insert(QBluetooth::Computer);
        majors.insert(QBluetooth::Phone);
        filter.setAcceptedDeviceMajors(majors);
    \endcode

    If the filter should accept devices regardless of their device majors,
    pass an empty set to this method. (This is the default value.)

    \sa acceptedDeviceMajors()
 */
void QBluetoothRemoteDeviceDialogFilter::setAcceptedDeviceMajors(const QSet<QBluetooth::DeviceMajor> &deviceMajors)
{
    d->deviceMajors = deviceMajors;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value is an empty set (i.e. the filter will accept devices with
    any device major).

    \sa setAcceptedDeviceMajors()
 */
QSet<QBluetooth::DeviceMajor> QBluetoothRemoteDeviceDialogFilter::acceptedDeviceMajors() const
{
    return d->deviceMajors;
}

/*!
    Sets the filter to accept devices that match the given \a serviceClasses.

    For example, to create a filter that only accepts devices with the
    ObjectTransfer and Telephony service classes:
    \code
        QBluetoothRemoteDeviceDialogFilter filter;
        filter.setAcceptedServiceClasses(QBluetooth::ObjectTransfer | QBluetooth::Telephony);
    \endcode

    If the filter should accept devices regardless of their service classes,
    pass QBluetooth::AllServiceClasses to this method. (This is the default
    value.)

    \sa acceptedServiceClasses()
 */
void QBluetoothRemoteDeviceDialogFilter::setAcceptedServiceClasses(QBluetooth::ServiceClasses serviceClasses)
{
    d->serviceClasses = serviceClasses;
}

/*!
    Returns the device majors that are accepted by this filter. By default,
    this value is QBluetooth::AllServiceClasses (i.e. the filter will accept
    devices with any service class).

    \sa setAcceptedServiceClasses()
 */
QBluetooth::ServiceClasses QBluetoothRemoteDeviceDialogFilter::acceptedServiceClasses() const
{
    return d->serviceClasses;
}

/*!
    Returns whether this filter allows the device \a device to be displayed
    in the associated device selector.

    If the accepted service class is QBluetooth::AllServiceClasses, the
    default implementation will also accept devices that have invalid service
    class values.
 */
bool QBluetoothRemoteDeviceDialogFilter::filterAcceptsDevice(const QBluetoothRemoteDevice &device)
{
    if (d->deviceMajors.size() > 0) {
        if (!d->deviceMajors.contains(device.deviceMajor()))
            return false;
    }

    // only do the & operation if the filter value is not AllServiceClasses,
    // otherwise you miss e.g. devices that have service class of 0
    if (d->serviceClasses != QBluetooth::AllServiceClasses) {
        if (!(d->serviceClasses & device.serviceClasses()))
            return false;
    }

    return true;
}


//================================================================

/*
   This class just blinks a little Bluetooth icon.
 */
DiscoveryStatusIcon::DiscoveryStatusIcon(QObject *parent, int blinkInterval)
    : QObject(parent),
      m_iconLabel(new QLabel),
      m_timer(new QTimer)
{
    m_timer->setInterval(blinkInterval);
    connect(m_timer, SIGNAL(timeout()), SLOT(toggleIconImage()));

    int size = QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    m_pixmapOffline = QIcon(":icon/bluetooth/bluetooth-offline").pixmap(size);
    m_pixmapOnline = QIcon(":icon/bluetooth/bluetooth-online").pixmap(size);

    m_iconLabel->setPixmap(m_pixmapOffline);
    m_iconLabel->setAlignment(Qt::AlignRight);
    m_iconLabel->setMinimumSize(m_pixmapOffline.size());
}

DiscoveryStatusIcon::~DiscoveryStatusIcon()
{
    delete m_iconLabel;
    delete m_timer;
}

void DiscoveryStatusIcon::setState(State state)
{
    switch (state) {
        case Active:
            m_timer->start();
            m_iconLabel->setPixmap(m_pixmapOnline);
            break;
        case Inactive:
            m_timer->stop();
            m_iconLabel->setPixmap(m_pixmapOffline);
            break;
        case Disabled:
            m_iconLabel->setPixmap( QIcon(":icon/bluetooth/bluetooth-notavail").
                    pixmap(m_pixmapOnline.size()));
            break;
    }
}

QLabel *DiscoveryStatusIcon::iconLabel() const
{
    return m_iconLabel;
}

void DiscoveryStatusIcon::toggleIconImage()
{
    if (m_iconLabel->pixmap()->serialNumber() == m_pixmapOnline.serialNumber())
        m_iconLabel->setPixmap(m_pixmapOffline);
    else
        m_iconLabel->setPixmap(m_pixmapOnline);
}


//=========================================================================


QBluetoothRemoteDeviceDialogPrivate::QBluetoothRemoteDeviceDialogPrivate(QBluetoothLocalDevice *local, QBluetoothRemoteDeviceDialog *parent)
    : QWidget(parent),
      m_filterSelectorEnabled(true),
      TEXT_DISCOVERY_CANCEL( tr("Stop searching") ),
      m_parent(parent),
      m_local(local),
      m_firstShow(true),
      m_currFilterIndex(-1)
{
    if (!m_local)
        m_local = new QBluetoothLocalDevice(this);

    initWidgets();
    initLayout();
    initActions();

    connect(m_local, SIGNAL(discoveryCancelled()), SLOT(discoveryCompleted()));
    connect(m_local, SIGNAL(discoveryCompleted()), SLOT(discoveryCompleted()));
    connect(m_local, SIGNAL(remoteDeviceFound(QBluetoothRemoteDevice)),
            SLOT(discoveredDevice(QBluetoothRemoteDevice)));

    connect(&m_sdap, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
            SLOT(serviceSearchCompleted(QBluetoothSdpQueryResult)));
    connect(&m_sdap, SIGNAL(searchCancelled()),
            SLOT(serviceSearchCancelCompleted()));

    m_discovering = false;
    m_cancellingDiscovery = false;
    m_cancellingServiceSearch = false;

    parent->setWindowTitle(tr("Select a device"));

    // have a "cancel" instead of "return" key
    QtopiaApplication::setMenuLike(parent, true);
}

QBluetoothRemoteDeviceDialogPrivate::~QBluetoothRemoteDeviceDialogPrivate()
{
    cleanUp();
    delete m_browser;
}

void QBluetoothRemoteDeviceDialogPrivate::cleanUp()
{
    if (m_local && m_local->isValid() && m_discovering)
        cancelDiscovery();
}


void QBluetoothRemoteDeviceDialogPrivate::addFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    if (m_filters.isEmpty())
        m_filters.append(new QBluetoothRemoteDeviceDialogFilter(tr("All")));

    int index = m_filters.size() - 1;
    m_filters.insert(index, filter);
    setCurrentFilter(index);

    // if not visible, don't enable the selector until showEvent()
    if (m_filterSelectorEnabled && isVisible())
        enableFilterSelector();
}

void QBluetoothRemoteDeviceDialogPrivate::removeFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    int index = m_filters.indexOf(filter);
    if (index == -1)
        return;

    m_filters.removeAt(index);
    if (m_filterCombo)
        m_filterCombo->removeItem(index);

    if (m_filters.size() == 1) {    // only "All" filter left
        if (isVisible())
            disableFilterSelector();
        setCurrentFilter(0);
    } else {
        if (m_currFilterIndex == index)     // removing current filter
            setCurrentFilter(m_currFilterIndex - 1);
    }
}

void QBluetoothRemoteDeviceDialogPrivate::clearFilters()
{
    qDeleteAll(m_filters.begin(), m_filters.end());
    m_filters.clear();
    m_currFilterIndex = -1;

    disableFilterSelector();
}

void QBluetoothRemoteDeviceDialogPrivate::setCurrentFilter(int index)
{
    if (index >= 0 && index < m_filters.size()) {
        m_currFilterIndex = index;
        if (m_filterCombo)
            m_filterCombo->setCurrentIndex(index);
    }
}

void QBluetoothRemoteDeviceDialogPrivate::setCurrentFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    if (filter)
        setCurrentFilter(m_filters.indexOf(filter));
}

QBluetoothRemoteDeviceDialogFilter *QBluetoothRemoteDeviceDialogPrivate::currentFilter() const
{
    return m_filters.value(m_currFilterIndex, 0);
}

void QBluetoothRemoteDeviceDialogPrivate::filterIndexChanged(int index)
{
    QBluetoothRemoteDeviceDialogFilter *filter = m_filters.value(index, 0);
    if (!filter)
        return;

    for (int i=0; i<m_discoveredDevices.size(); i++) {
        if (filter->filterAcceptsDevice(m_discoveredDevices[i])) {
            m_browser->showDevice(m_discoveredDevices[i].address());
        } else {
            m_browser->hideDevice(m_discoveredDevices[i].address());
            qLog(Bluetooth) << "QBluetoothRemoteDeviceDialog: not displaying"
                << m_discoveredDevices[i].address().toString() << ", device rejected by filter";
        }
    }

    if (!m_discovering)
        m_statusLabel->setText(describeDiscoveryResults());
}

void QBluetoothRemoteDeviceDialogPrivate::showFilterDialog()
{
    QtopiaApplication::execDialog(m_filterDialog);
}

void QBluetoothRemoteDeviceDialogPrivate::enableFilterSelector()
{
    if (!m_filterCombo) {
        m_filterCombo = new QComboBox;
        m_filterCombo->setModel(new DeviceFilterModel(m_filters, this));
        connect(m_filterCombo, SIGNAL(currentIndexChanged(int)),
                SLOT(filterIndexChanged(int)));
        m_filterCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(new QLabel(tr("Show:")));
        layout->addWidget(m_filterCombo);

        if (Qtopia::mousePreferred()) {
            m_mainLayout->addLayout(layout);
        } else {
            m_filterDialog = new QDialog;
            m_filterDialog->setWindowTitle(tr("Choose view option"));
            m_filterDialog->setLayout(layout);

            m_chooseFilterAction = m_menu->addAction(tr("Show..."), this,
                    SLOT(showFilterDialog()));
        }
    }

    if (Qtopia::mousePreferred()) {
        m_filterCombo->show();
    } else {
        m_chooseFilterAction->setVisible(true);
    }
}

void QBluetoothRemoteDeviceDialogPrivate::disableFilterSelector()
{
    if (Qtopia::mousePreferred()) {
        if (m_filterCombo)
            m_filterCombo->hide();
    } else {
        if (m_chooseFilterAction)
            m_chooseFilterAction->setVisible(false);
    }
}


QBluetoothAddress QBluetoothRemoteDeviceDialogPrivate::selectedDevice() const
{
    if (m_local && m_local->isValid())
        return m_browser->selectedDevice();
    return QBluetoothAddress();
}

void QBluetoothRemoteDeviceDialogPrivate::triggeredDiscoveryAction()
{
    if (m_discoveryAction->text() == TEXT_DISCOVERY_CANCEL) {
        cancelDiscovery();
    } else {    // start/restart
        startDiscovery();
    }
}

QString QBluetoothRemoteDeviceDialogPrivate::describeDiscoveryResults()
{
    int count = 0;
    for (int i=0; i<m_discoveredDevices.count(); i++) {
        if (!m_browser->isDeviceHidden(m_discoveredDevices[i].address()))
            count++;
    }

    if (count == 0) {
        return tr("No devices found");
    } else if (count == 1) {
        return tr("Found 1 device");
    } else {
        return tr("Found %1 devices", "%1 = number (#>1) of devices").arg(count);
    }
}

void QBluetoothRemoteDeviceDialogPrivate::startDiscovery()
{
    if (m_discovering)
        return;

    if (!m_local || !m_local->isValid()) {
        m_statusLabel->setText(tr("Bluetooth not available."));
        m_statusIcon->setState(DiscoveryStatusIcon::Disabled);
        return;
    }

    m_discovering = true;
    m_cancellingDiscovery = false;

    // remove all devices from the list
    m_browser->clear();
    m_discoveredDevices.clear();
    setDeviceActionsEnabled(false);

    m_statusLabel->setText(tr("Searching for devices..."));
    m_statusIcon->setState(DiscoveryStatusIcon::Active);
    m_discoveryAction->setText(TEXT_DISCOVERY_CANCEL);
    m_discoveryAction->setIcon(m_discoveryCancelIcon);

    m_discoveryAttempts = 0;
    reallyStartDiscovery();
}

void QBluetoothRemoteDeviceDialogPrivate::reallyStartDiscovery()
{
    // user may have cancelled discovery during delayed discoveries
    if (!m_discovering)
        return;

    // can't start a discovery while service search is still in progress
    if (m_cancellingServiceSearch ||
            (!m_local->discoverRemoteDevices() && m_local->error() == QBluetoothLocalDevice::InProgress) ) {
        if (m_discoveryAttempts < 100) {
            // delay discovery, try again later
            m_discoveryAttempts++;
            QTimer::singleShot(500, this, SLOT(reallyStartDiscovery()));
        } else {
            QMessageBox::warning(this, tr("Bluetooth Error"),
                tr("Bluetooth not available."));
            discoveryCompleted();
        }
    }
}

bool QBluetoothRemoteDeviceDialogPrivate::cancelDiscovery()
{
    if (!m_local || !m_local->isValid() || !m_discovering || m_cancellingDiscovery)
        return false;

    if (!m_local->cancelDiscovery()) {
        qLog(Bluetooth) << "QBluetoothRemoteDeviceDialog: cannot cancel discovery";
        return false;
    }

    m_cancellingDiscovery = true;
    m_statusLabel->setText(tr("Canceling..."));
    return true;
}

void QBluetoothRemoteDeviceDialogPrivate::discoveryCompleted()
{
    m_statusLabel->setText(describeDiscoveryResults());
    m_statusIcon->setState(DiscoveryStatusIcon::Inactive);
    m_discoveryAction->setText(tr("Search again"));
    m_discoveryAction->setIcon(m_discoveryStartIcon);

    m_discovering = false;
    m_cancellingDiscovery = false;
}

void QBluetoothRemoteDeviceDialogPrivate::discoveredDevice(const QBluetoothRemoteDevice &device)
{
    if (!m_discovering)
        return;

    m_discoveredDevices << device;
    m_browser->insert(device);

    QBluetoothRemoteDeviceDialogFilter *filter = m_filters.value(m_currFilterIndex, 0);
    if (filter && !filter->filterAcceptsDevice(device)) {
        m_browser->hideDevice(device.address());
        qLog(Bluetooth) << "QBluetoothRemoteDeviceDialog: not displaying"
            << device.address().toString() << ", device rejected by filter";
    } else {
        m_browser->showDevice(device.address());
        if (m_browser->count() == 1)
            m_browser->selectDevice(device.address());
    }
}

void QBluetoothRemoteDeviceDialogPrivate::deviceSelectionChanged()
{
    QBluetoothAddress addr = m_browser->selectedDevice();
    setDeviceActionsEnabled(addr.isValid());
}

void QBluetoothRemoteDeviceDialogPrivate::activated(const QBluetoothAddress &addr)
{
    if (m_validProfiles.isEmpty()) {
        // don't need to check profiles to validate device
        deviceActivatedOk();
    } else {
        // must cancel any current discovery, or SDP query will fail
        if (m_discovering)
            cancelDiscovery();
        m_validationWaitWidget->show();
        m_deviceUnderValidation = addr;
        validateProfiles();
    }
}

void QBluetoothRemoteDeviceDialogPrivate::deviceActivatedOk()
{
    if (m_discovering) {
        if (cancelDiscovery()) {
            QTimer timer;
            timer.start(500);
            while (m_discovering) {
                if (!timer.isActive())
                    break;
                qApp->processEvents();
            }
        }
    }

    m_validationWaitWidget->hide();
    // close the dialog
    m_parent->accept();
}

void QBluetoothRemoteDeviceDialogPrivate::validateProfiles()
{
    if (!m_validationWaitWidget->isVisible())
        return;

    if (m_cancellingDiscovery) {
        QTimer::singleShot(100, this, SLOT(validateProfiles()));
        return;
    }

    if (m_cancellingServiceSearch) {
        QTimer::singleShot(100, this, SLOT(validateProfiles()));
        return;
    }

    if (!m_deviceUnderValidation.isValid()) {
        m_validationWaitWidget->hide();
        return;
    }

    if (!m_local || !m_local->isValid()) {
        QMessageBox::warning(this, tr("Bluetooth Error"),
            tr("<P>Bluetooth is not available, cannot verify services."));
        m_validationWaitWidget->hide();
        return;
    }

    if (!m_sdap.searchServices(m_deviceUnderValidation, *m_local, QBluetoothSdpUuid::L2cap))
        validationError();
}

/*
    Call if there was an error starting the search or the QBluetoothSdpQueryResult has
    an error. Don't call if search was cancelled, or the search completed
    successfullly but the device doesn't have the required profiles.
*/
void QBluetoothRemoteDeviceDialogPrivate::validationError()
{
    QMessageBox::warning(this, tr("Service Error"),
        tr("<P>Unable to verify services. Try again, or choose another device."));
    m_validationWaitWidget->hide();

    // don't reset the address - becomes confusing if multiple searches are
    // cancelled
    //m_deviceUnderValidation = QBluetoothAddress();
}

void QBluetoothRemoteDeviceDialogPrivate::serviceSearchCompleted(const QBluetoothSdpQueryResult &result)
{
    const QList<QBluetoothSdpRecord> services = result.services();

    if (!m_deviceUnderValidation.isValid()) {
        validationError();
        return;
    }

    if (!result.isValid()) {
        validationError();
        return;
    }

    if (serviceProfilesMatch(services)) {
        // device is now validated, emit the activation signal
        deviceActivatedOk();
    } else {
        QMessageBox::warning(this, tr("Service Error"),
            tr("<P>Device does not have the necessary services.  Try again, or choose another device."));
        m_validationWaitWidget->hide();
    }
}

void QBluetoothRemoteDeviceDialogPrivate::serviceSearchCancelled()
{
    if (!m_cancellingServiceSearch) {
        m_cancellingServiceSearch = true;
        m_sdap.cancelSearch();
    }

    m_validationWaitWidget->hide();
}

void QBluetoothRemoteDeviceDialogPrivate::serviceSearchCancelCompleted()
{
    m_cancellingServiceSearch = false;
}

bool QBluetoothRemoteDeviceDialogPrivate::serviceProfilesMatch(const QList<QBluetoothSdpRecord> services)
{
    QSetIterator<QBluetooth::SDPProfile> iter(m_validProfiles);
    for (int i=0; i<services.size(); i++) {
        while (iter.hasNext()) {
            if (services[i].isInstance(iter.next())) {
                return true;
            }
        }
        iter.toFront();
    }
    return false;
}

void QBluetoothRemoteDeviceDialogPrivate::addDeviceAction(QAction *action)
{
    m_deviceActions.append(action);

    // if no separator, create it, then insert the new action before the separator.
    if (!m_deviceActionsSeparator) {
        m_deviceActionsSeparator = m_menu->insertSeparator(m_discoveryAction);
    }
    m_menu->insertAction(m_deviceActionsSeparator, action);
}

void QBluetoothRemoteDeviceDialogPrivate::removeDeviceAction(QAction *action)
{
    int index = m_deviceActions.indexOf(action);
    if (index == -1)
        return;

    m_deviceActions.removeAt(index);
    m_menu->removeAction(action);

    // if no more device actions, remove the separator
    if (m_deviceActions.size() == 0) {
        delete m_deviceActionsSeparator;
        m_deviceActionsSeparator = 0;
    }
}

void QBluetoothRemoteDeviceDialogPrivate::setDeviceActionsEnabled(bool enabled)
{
    for (int i=0; i<m_deviceActions.size(); i++)
        m_deviceActions[i]->setEnabled(enabled);
}


//------------------------


void QBluetoothRemoteDeviceDialogPrivate::initWidgets()
{
    // don't show connection status because it will appear connected whenever
    // a remote name query is performed during the discovery
    QBluetoothRemoteDeviceSelector::DisplayFlags flags =
            QBluetoothRemoteDeviceSelector::DeviceIcon |
            QBluetoothRemoteDeviceSelector::Name |
            QBluetoothRemoteDeviceSelector::Alias |
            QBluetoothRemoteDeviceSelector::PairingStatus;
    m_browser = new QBluetoothRemoteDeviceSelector(flags, m_local);
    connect(m_browser, SIGNAL(activated(QBluetoothAddress)),
            SLOT(activated(QBluetoothAddress)));
    connect(m_browser, SIGNAL(selectionChanged()),
            SLOT(deviceSelectionChanged()));

    m_statusLabel = new QLabel;
    m_statusIcon = new DiscoveryStatusIcon(this);

    m_validationWaitWidget = new QWaitWidget(this);
    m_validationWaitWidget->setText(tr("Verifying services..."));
    m_validationWaitWidget->setCancelEnabled(true);
    connect(m_validationWaitWidget, SIGNAL(cancelled()),
            SLOT(serviceSearchCancelled()));

    m_filterCombo = 0;
    m_filterDialog = 0;
}

void QBluetoothRemoteDeviceDialogPrivate::initLayout()
{
    m_mainLayout = new QVBoxLayout;

    // top bar with text "Searching..." and the bluetooth icon
    QHBoxLayout *statusBar = new QHBoxLayout;
    statusBar->setContentsMargins(6, 0, 6, 0);
    statusBar->addWidget(m_statusLabel);
    statusBar->addWidget(m_statusIcon->iconLabel());

    m_mainLayout->addLayout(statusBar);
    m_mainLayout->addWidget(m_browser);

    m_mainLayout->setSpacing(3);
    m_mainLayout->setMargin(0);

    setLayout(m_mainLayout);
}

void QBluetoothRemoteDeviceDialogPrivate::initActions()
{
    m_chooseFilterAction = 0;

    m_discoveryStartIcon = QIcon(":icon/find");
    m_discoveryCancelIcon = QIcon(":icon/reset");
    m_discoveryAction = new QAction(m_discoveryStartIcon,
                                    tr("Search for devices"), this);
    connect(m_discoveryAction, SIGNAL(triggered()),
            SLOT(triggeredDiscoveryAction()));

    m_menu = QSoftMenuBar::menuFor(this);
    QSoftMenuBar::setHelpEnabled(this, false);

    // add generic actions (shown below device actions)
    m_menu->addAction(m_discoveryAction);

    // separator that separates generic actions and device actions
    m_deviceActionsSeparator = 0;
}

void QBluetoothRemoteDeviceDialogPrivate::showEvent(QShowEvent *e)
{
    if (m_firstShow) {
        QTimer::singleShot(0, this, SLOT(triggeredDiscoveryAction()));
        m_firstShow = false;
    }

    if (m_filterSelectorEnabled) {
        if (m_filters.size() > 1) // has more than just 'All' filter
            enableFilterSelector();
        else
            disableFilterSelector();
    } else {
        disableFilterSelector();
    }

    QWidget::showEvent(e);
}


//=======================================================================

/*!
    \class QBluetoothRemoteDeviceDialog
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRemoteDeviceDialog class allows the user to perform a bluetooth device discovery and select a particular device.

    When a remote device dialog is first displayed, it automatically
    starts a device discovery in order to display remote devices found
    in the vicinity.

    The setFilter() function can be used to ensure that the device dialog
    only displays certain types of devices. Also, the setValidationProfiles()
    function can be used to ensure that the user can only activate a device
    if offers a particular service profile. For example, here is a dialog that
    will only display phone devices, and only allows the user to select a
    device if the device offers a service with the Object Push Profile:

    \code
    QSet<QBluetooth::DeviceMajor> deviceMajors;
    deviceMajors.insert(QBluetooth::Phone);
    QBluetoothRemoteDeviceDialogFilter filter;
    filter.setAcceptedDeviceMajors(deviceMajors);

    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(QBluetooth::ObjectPushProfile);

    QBluetoothRemoteDeviceDialog *dialog = new QBluetoothRemoteDeviceDialog;
    dialog->setFilter(&filter);
    dialog->setValidationProfiles(profiles);
    if (QtopiaApplication::execDialog(dialog) == QDialog::Accepted) {
        QBluetoothAddress selectedDevice = dialog->selectedDevice();
    }
    \endcode

    This will produce a dialog similar to this:

    \image qbluetoothremotedevicedialog.png "Screenshot of example dialog"

    The static function getRemoteDevice() is the easiest way to run a device
    dialog. It runs a modal device selection dialog, then returns the
    address of the device that was activated by the user. Using getRemoteDevice(),
    the last section of the above example code could be rewritten thus:

    \code
    QBluetoothAddress selectedDevice =
            QBluetoothRemoteDeviceDialog::getRemoteDevice(0, profiles, &filter);
    \endcode

    You can also run the dialog without using getRemoteDevice() or
    QtopiaApplication::execDialog(). When the user activates a device and the
    device has the required profiles as set by setValidationProfiles(), the
    accepted() signal is emitted, and the activated device can be found by
    calling selectedDevice(). If the dialog is canceled, the rejected() signal
    is emitted.

    QBluetoothRemoteDeviceDialog also allows custom menu actions to be added
    through QWidget::addAction(). Any added actions will be enabled when a
    device is selected, and disabled when no devices are selected. They are
    also disabled during device discoveries.

    \ingroup qtopiabluetooth
    \sa QBluetoothRemoteDeviceDialogFilter
 */

/*!
    Constructs a QBluetoothRemoteDeviceDialog with the given parent widget
    \a parent and the window flags \a flags.
 */
QBluetoothRemoteDeviceDialog::QBluetoothRemoteDeviceDialog(QWidget *parent, Qt::WFlags flags)
    : QDialog(parent, flags),
      m_data(new QBluetoothRemoteDeviceDialogPrivate(0, this))
{
    // don't want margins on sides because it's annoying when trying to
    // grab scrollbar on touchscreens
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 6, 0, 0);
    l->addWidget(m_data);
}

/*!
    Constructs a QBluetoothRemoteDeviceDialog with the given parent widget
    \a parent and the window flags \a flags. \a local will be used to query
    the local bluetooth device. If \a local is 0, the default device will
    be used.
 */
QBluetoothRemoteDeviceDialog::QBluetoothRemoteDeviceDialog(QBluetoothLocalDevice *local, QWidget *parent, Qt::WFlags flags)
    : QDialog(parent, flags),
      m_data(new QBluetoothRemoteDeviceDialogPrivate(local, this))
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 6, 0, 0);
    l->addWidget(m_data);
}

/*!
    Destroys the device dialog.
 */
QBluetoothRemoteDeviceDialog::~QBluetoothRemoteDeviceDialog()
{
}

/*!
    Shows a remote device dialog with the validation profiles \a profiles,
    the device filter \a filter, and the parent \a parent. If \a filter is
    0, the dialog will not filter the device display.

    Returns the address of the remote device that was activated, or
    an invalid address if the dialog was cancelled.

    \sa setValidationProfiles(), QBluetoothRemoteDeviceDialogFilter
 */
QBluetoothAddress QBluetoothRemoteDeviceDialog::getRemoteDevice(QWidget *parent, QSet<QBluetooth::SDPProfile> profiles, QBluetoothRemoteDeviceDialogFilter *filter)
{
    QBluetoothRemoteDeviceDialog dlg(0, parent, 0);
    dlg.setValidationProfiles(profiles);
    dlg.setFilter(filter);

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted)
        return dlg.selectedDevice();
    else
        return QBluetoothAddress();
}

/*!
    Set the validation SDP profiles to \a profiles.

    When this is set, a device can only be chosen if it has at least one
    service that matches one or more of the given profiles.

    For example, a dialog like this will only allow the user to select a
    device if it has a service with the OBEX Object Push profile:

    \code
    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(QBluetooth::ObjectPushProfile);
    QBluetoothRemoteDeviceDialog dialog;
    dialog.setValidationProfiles(profiles);
    \endcode

    (If the user chooses a device that does not have the Object Push Profile,
    the dialog will not be closed, and a message box will be displayed
    informing the user that the selected device does not have the necessary
    services.)

    \sa validationProfiles()
 */
void QBluetoothRemoteDeviceDialog::setValidationProfiles(QSet<QBluetooth::SDPProfile> profiles)
{
    m_data->m_validProfiles = profiles;
}

/*!
    Returns the SDP profiles that are used to validate an activated device.

    \sa setValidationProfiles()
 */
QSet<QBluetooth::SDPProfile> QBluetoothRemoteDeviceDialog::validationProfiles() const
{
    return m_data->m_validProfiles;
}

/*!
    \obsolete

    Use addFilter() and removeFilter() instead.
 */
void QBluetoothRemoteDeviceDialog::setFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    m_data->removeFilter(currentFilter());
    if (filter)
        m_data->addFilter(filter);
}

/*!
    \obsolete

    Use currentFilter() instead.
 */
QBluetoothRemoteDeviceDialogFilter *QBluetoothRemoteDeviceDialog::filter() const
{
    return m_data->currentFilter();
}

/*!
    Adds \a filter to the group of filters to be used for this dialog.

    This dialog takes ownership of the filter.

    \sa setFilterSelectionEnabled()
*/
void QBluetoothRemoteDeviceDialog::addFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    m_data->addFilter(filter);
}

/*!
    Removes \a filter from the group of filters for this dialog.

    Ownership of the filter is passed to the caller.

    \sa addFilter(), clearFilters()
*/
void QBluetoothRemoteDeviceDialog::removeFilter(QBluetoothRemoteDeviceDialogFilter *filter)
{
    m_data->removeFilter(filter);
}

/*!
    Removes and deletes all filters that have been added to this dialog.

    \sa removeFilter()
*/
void QBluetoothRemoteDeviceDialog::clearFilters()
{
    m_data->clearFilters();
}

/*!
    Sets \a filter to be the current filter. It must have been already added
    using addFilter().

    \sa currentFilter()
*/
void QBluetoothRemoteDeviceDialog::setCurrentFilter( QBluetoothRemoteDeviceDialogFilter *filter )
{
    m_data->setCurrentFilter(filter);
}

/*!
    Returns the current filter.

    \sa setCurrentFilter()
 */
QBluetoothRemoteDeviceDialogFilter *QBluetoothRemoteDeviceDialog::currentFilter() const
{
    return m_data->currentFilter();
}

/*!
    If \a enabled is \c true, the dialog enables the end user to change between
    filters that have been added to this display. This is useful, for example,
    if the current filter is too restrictive and has excluded the device that
    the user wants to select; in this case, the user can change to a less
    restrictive filter, and locate the desired device.

    An "All" filter is automatically added that allows the user to view all
    discovered devices.

    By default, filter selection is enabled.

    \sa filterSelectionEnabled()
*/
void QBluetoothRemoteDeviceDialog::setFilterSelectionEnabled(bool enabled)
{
    m_data->m_filterSelectorEnabled = enabled;
    if (enabled)
        m_data->enableFilterSelector();
    else
        m_data->disableFilterSelector();
}

/*!
    Returns whether the dialog enables the end user to change between the
    added filters.

    \sa setFilterSelectionEnabled()
*/
bool QBluetoothRemoteDeviceDialog::filterSelectionEnabled() const
{
    return m_data->m_filterSelectorEnabled;
}

/*!
    Returns the address of the currently selected device.

    Note that a dialog's validation profiles (see setValidationProfiles())
    are only applied when a device is activated, and not when they are merely
    selected. Therefore, if any validation profiles have been set, the
    programmer should be aware that the device returned by this method may not
    have been validated against these profiles, if this function is called
    before the user has activated a device in the dialog.

    \sa setValidationProfiles()
 */
QBluetoothAddress QBluetoothRemoteDeviceDialog::selectedDevice() const
{
    return m_data->selectedDevice();
}

/*!
    \reimp
*/
void QBluetoothRemoteDeviceDialog::done(int r)
{
    m_data->cleanUp();
    QDialog::done(r);
}

/*!
    \reimp
 */
void QBluetoothRemoteDeviceDialog::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
        case QEvent::ActionAdded:
            m_data->addDeviceAction(action);
            break;
        case QEvent::ActionRemoved:
            m_data->removeDeviceAction(action);
            break;
        default:
            break;
    }
}

#include "qbluetoothremotedevicedialog.moc"
