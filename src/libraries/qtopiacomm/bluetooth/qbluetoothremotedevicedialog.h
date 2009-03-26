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

#ifndef QBLUETOOTHREMOTEDEVICEDIALOG_H
#define QBLUETOOTHREMOTEDEVICEDIALOG_H

#include <qbluetoothnamespace.h>
#include <qbluetoothaddress.h>

#include <QDialog>
#include <QSet>

class QBluetoothRemoteDeviceDialogFilterPrivate;

class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceDialogFilter
{
public:
    QBluetoothRemoteDeviceDialogFilter();
    QBluetoothRemoteDeviceDialogFilter(const QString &title);
    virtual ~QBluetoothRemoteDeviceDialogFilter();

    QString title() const;

    void setAcceptedDeviceMajors( const QSet<QBluetooth::DeviceMajor> &deviceMajors );
    QSet<QBluetooth::DeviceMajor> acceptedDeviceMajors() const;

    void setAcceptedServiceClasses( QBluetooth::ServiceClasses serviceClasses );
    QBluetooth::ServiceClasses acceptedServiceClasses() const;

    virtual bool filterAcceptsDevice( const QBluetoothRemoteDevice &device );

private:
    Q_DISABLE_COPY(QBluetoothRemoteDeviceDialogFilter);
    QBluetoothRemoteDeviceDialogFilterPrivate *d;
};


class QBluetoothRemoteDeviceDialogPrivate;
class QActionEvent;

class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceDialog : public QDialog
{
    friend class QBluetoothRemoteDeviceDialogPrivate;
    Q_OBJECT

public:
    explicit QBluetoothRemoteDeviceDialog( QWidget *parent = 0,
                                           Qt::WFlags flags = 0 );
    explicit QBluetoothRemoteDeviceDialog( QBluetoothLocalDevice *local = 0,
                                           QWidget *parent = 0,
                                           Qt::WFlags flags = 0 );
    virtual ~QBluetoothRemoteDeviceDialog();

    static QBluetoothAddress getRemoteDevice(
            QWidget *parent = 0,
            QSet<QBluetooth::SDPProfile> profiles = QSet<QBluetooth::SDPProfile>(),
            QBluetoothRemoteDeviceDialogFilter *filter = 0);

    void setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles );
    QSet<QBluetooth::SDPProfile> validationProfiles() const;

    void addFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    void removeFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    void clearFilters();

    void setCurrentFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    QBluetoothRemoteDeviceDialogFilter *currentFilter() const;

    void setFilterSelectionEnabled(bool enabled);
    bool filterSelectionEnabled() const;

    QBluetoothAddress selectedDevice() const;

    // obsolete
    void setFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    QBluetoothRemoteDeviceDialogFilter *filter() const;

public slots:
    virtual void done( int r );

protected:
    virtual void actionEvent(QActionEvent *event);

private:
    QBluetoothRemoteDeviceDialogPrivate *m_data;
};

#endif
