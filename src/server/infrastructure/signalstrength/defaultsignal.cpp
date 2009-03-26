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

#include "defaultsignal.h"
#include <QSettings>
#include <QSignalSource>
#include <QHardwareManager>
#include "qtopiaserverapplication.h"

/*! 
   \class DefaultSignal
    \inpublicgroup QtConnectivityModule
    \inpublicgroup QtTelephonyModule
   \ingroup QtopiaServer::Task
   \brief The DefaultSignal class provides a proxy for another system signal.

   To simplify the design of system themes and other mechanisms that source 
   information directly from the value space, the DefaultSignal class proxies
   the information of another, configurable system signal provider under the name
   "DefaultSignal".

   The Signal to proxy can be manually configured by setting the 
   \c {SignalSources/DefaultSignalSource} key in the \c {Trolltech/HardwareAccessories}
   configuration file.  If this setting is missing, the DefaultSignal class 
   will proxy the first available signal source of type modem. If Qt Extended is configured
   without a modem the first wlan signal source that becomes available will be used.

   The DefaultSignal class provides the \c {DefaultSignal} task.
   It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */

/*!
  Constructs a new DefaultSignal instance with the given \a parent.
  */
DefaultSignal::DefaultSignal(QObject *parent)
: QSignalSourceProvider(QLatin1String("virtual"), QLatin1String("DefaultSignal"), parent),
  m_signalSource(0), m_accessories(0)
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("HardwareAccessories"));
    m_primary = cfg.value(QLatin1String("SignalSources/DefaultSignalSource")).toString();

    if(m_primary.isEmpty()) {
        // No configured default, use first signal
        m_accessories = new QHardwareManager(QLatin1String("QSignalSource"), this);
        QObject::connect(m_accessories, SIGNAL(providerAdded(QString)),
                         this, SLOT(accessoryAdded(QString)));
        QStringList accs = m_accessories->providers();
        for(int ii = 0; m_accessories && ii < accs.count(); ++ii)
            accessoryAdded(accs.at(ii));
    } else {
        m_signalSource = new QSignalSource(m_primary, this);
        initSignalSource();
    }
        
    syncSignalSource();
}

void DefaultSignal::syncSignalSource()
{
    if(m_signalSource) {
        setAvailability(m_signalSource->availability());
        setSignalStrength(m_signalSource->signalStrength());
    } else {
        setAvailability(QSignalSource::Invalid);
        setSignalStrength(-1);
    }
}

void DefaultSignal::initSignalSource()
{
    QObject::connect(m_signalSource, SIGNAL(availabilityChanged(QSignalSource::Availability)), this, SLOT(pAvailabilityChanged(QSignalSource::Availability)));
    QObject::connect(m_signalSource, SIGNAL(signalStrengthChanged(int)), this, SLOT(pSignalStrengthChanged(int)));
}

void DefaultSignal::accessoryAdded( const QString& acc )
{
    QSignalSource *ps = new QSignalSource(acc, this);
#if QTOPIA_CELL
    if ( ps->type() == QLatin1String("modem") ) {
#elif QTOPIA_VOIP
    if ( ps->type() == QLatin1String("wlan") ) {
#else
    if ( ps->type() != QLatin1String("virtual") ) {
#endif
        m_signalSource = ps;
        m_accessories->disconnect();
        m_accessories->deleteLater();
        m_accessories = 0;
        initSignalSource();
        syncSignalSource();
    } else {
        delete ps;
    }
}

void DefaultSignal::pAvailabilityChanged(QSignalSource::Availability a)
{
    setAvailability(a);
}

void DefaultSignal::pSignalStrengthChanged(int c)
{
    setSignalStrength(c);
}

QTOPIA_TASK(DefaultSignal, DefaultSignal);
#include "defaultsignal.h"
