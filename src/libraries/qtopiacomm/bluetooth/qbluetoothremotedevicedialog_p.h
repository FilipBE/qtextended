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

#ifndef QBLUETOOTHREMOTEDEVICEDIALOG_P_H
#define QBLUETOOTHREMOTEDEVICEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qbluetoothremotedevicedialog.h"
#include <qbluetoothsdpquery.h>
#include <qbluetoothlocaldevice.h>

#include <QList>
#include <QIcon>
#include <QComboBox>

class QBluetoothRemoteDeviceSelector;
class QLabel;
class QWaitWidget;
class QBluetoothSdpQueryResult;
class QActionGroup;
class QMenu;
class QVBoxLayout;


class DiscoveryStatusIcon : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        Active,
        Inactive,
        Disabled
    };

    DiscoveryStatusIcon(QObject *parent = 0, int blinkInterval = 300);
    ~DiscoveryStatusIcon();

    void setState(State state);
    QLabel *iconLabel() const;

private slots:
    void toggleIconImage();

private:
    QLabel *m_iconLabel;
    QTimer *m_timer;
    QPixmap m_pixmapOnline;
    QPixmap m_pixmapOffline;
};


class QBluetoothRemoteDeviceDialogPrivate : public QWidget
{
    Q_OBJECT

public:
    QBluetoothRemoteDeviceDialogPrivate(QBluetoothLocalDevice *local, QBluetoothRemoteDeviceDialog *parent);
    ~QBluetoothRemoteDeviceDialogPrivate();

    void addFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    void removeFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    void clearFilters();

    void setCurrentFilter(int index);
    void setCurrentFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    QBluetoothRemoteDeviceDialogFilter *currentFilter() const;

    QBluetoothAddress selectedDevice() const;

    void addDeviceAction(QAction *action);
    void removeDeviceAction(QAction *action);

    void enableFilterSelector();
    void disableFilterSelector();

    void cleanUp();

    QSet<QBluetooth::SDPProfile> m_validProfiles;
    bool m_filterSelectorEnabled;

protected:
    void showEvent(QShowEvent *event);

private slots:
    void filterIndexChanged(int index);
    void showFilterDialog();

    void deviceSelectionChanged();
    void activated(const QBluetoothAddress &addr);

    void triggeredDiscoveryAction();
    void discoveryCompleted();
    void discoveredDevice(const QBluetoothRemoteDevice &device);
    void reallyStartDiscovery();

    void validateProfiles();

    void serviceSearchCompleted(const QBluetoothSdpQueryResult &result);
    void serviceSearchCancelled();
    void serviceSearchCancelCompleted();

private:
    QString describeDiscoveryResults();
    void startDiscovery();
    bool cancelDiscovery();

    void deviceActivatedOk();

    void validationError();
    bool serviceProfilesMatch(const QList<QBluetoothSdpRecord> services);

    void setDeviceActionsEnabled(bool enabled);

    void initWidgets();
    void initLayout();
    void initActions();

    const QString TEXT_DISCOVERY_CANCEL;

    QBluetoothRemoteDeviceDialog *m_parent;
    QPointer<QBluetoothLocalDevice> m_local;
    bool m_firstShow;

    // device discovery
    bool m_discovering;
    bool m_cancellingDiscovery;
    int m_discoveryAttempts;

    // service search
    QBluetoothSdpQuery m_sdap;
    QBluetoothAddress m_deviceUnderValidation;
    bool m_cancellingServiceSearch;

    // gui
    QBluetoothRemoteDeviceSelector *m_browser;
    QLabel *m_statusLabel;
    DiscoveryStatusIcon *m_statusIcon;
    QWaitWidget *m_validationWaitWidget;

    // actions
    QMenu *m_menu;
    QAction *m_discoveryAction;
    QList<QAction*> m_deviceActions;
    QAction *m_deviceActionsSeparator;

    // action icons
    QIcon m_discoveryStartIcon;
    QIcon m_discoveryCancelIcon;

    // filters
    QComboBox *m_filterCombo;
    QDialog *m_filterDialog;
    QAction *m_chooseFilterAction;
    QList<QBluetoothRemoteDeviceDialogFilter *> m_filters;
    int m_currFilterIndex;
    QList<QBluetoothRemoteDevice> m_discoveredDevices;

    QVBoxLayout *m_mainLayout;
};

#endif
