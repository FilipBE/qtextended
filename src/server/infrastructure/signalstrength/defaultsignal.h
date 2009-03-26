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

#ifndef DEFAULTSIGNAL_H
#define DEFAULTSIGNAL_H

#include <QSignalSourceProvider>

class QHardwareManager;
class QSignalSource;
class DefaultSignal : public QSignalSourceProvider
{
Q_OBJECT
public:
    DefaultSignal(QObject *parent = 0);

private slots:
    void accessoryAdded( const QString& newAccessory );

    void pAvailabilityChanged(QSignalSource::Availability);
    void pSignalStrengthChanged(int);

private:
    void initSignalSource();
    void syncSignalSource();

    QString m_primary;
    QSignalSource *m_signalSource;
    QHardwareManager *m_accessories;
};

#endif
