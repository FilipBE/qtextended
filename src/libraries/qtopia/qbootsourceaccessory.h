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

#ifndef QBOOTSOURCEACCESSORY_H
#define QBOOTSOURCEACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QBootSourceAccessory
//
// ============================================================================

class QTOPIA_EXPORT QBootSourceAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    enum Source
    {
        Unknown = 0,
        PowerKey = 1,
        Charger = 2,
        Alarm = 3,
        Watchdog = 4,
        Software = 5
    };

    explicit QBootSourceAccessory( const QString& id = QString(), QObject *parent = 0,
                                   QAbstractIpcInterface::Mode mode = Client );
    ~QBootSourceAccessory();

    QBootSourceAccessory::Source bootSource() const;

signals:
    void bootSourceModified();
};

// ============================================================================
//
// QBootSourceAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QBootSourceAccessoryProvider : public QBootSourceAccessory
{
    Q_OBJECT
public:
    explicit QBootSourceAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QBootSourceAccessoryProvider();

public slots:
    void setBootSource( QBootSourceAccessory::Source source );
};

#endif
