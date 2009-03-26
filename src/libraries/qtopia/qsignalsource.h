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

#ifndef QSIGNALSOURCE_H
#define QSIGNALSOURCE_H

#include <QHardwareInterface>
#include <qtopiaipcmarshal.h>

class QTOPIA_EXPORT QSignalSource : public QHardwareInterface
{
    Q_OBJECT
public:
    enum Availability {
        Available,
        NotAvailable,
        Invalid
    };

    explicit QSignalSource( const QString& id = QString(), QObject* parent = 0 );
    virtual ~QSignalSource();

    QString type() const;
    Availability availability() const;
    int signalStrength() const;

signals:
    void availabilityChanged( QSignalSource::Availability availability );
    void signalStrengthChanged( int strength );

private:
    friend class QSignalSourceProvider;
    QSignalSource( const QString& id, QObject* parent,
                            QAbstractIpcInterface::Mode mode );
};

Q_DECLARE_USER_METATYPE_ENUM(QSignalSource::Availability);

class QSignalSourceProviderPrivate;
class QTOPIA_EXPORT QSignalSourceProvider : public QSignalSource
{
    Q_OBJECT
public:
    explicit QSignalSourceProvider( const QString& type, const QString& id, QObject* parent = 0 );
    virtual ~QSignalSourceProvider();

public slots:
    void setAvailability( QSignalSource::Availability availability );
    void setSignalStrength( int currentStrength );

private slots:
    void update();

private:
    QSignalSourceProviderPrivate *d;
};

#endif
