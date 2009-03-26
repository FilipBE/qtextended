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

#include "qdeviceindicatorsprovider.h"
#include <QtopiaChannel>
#include <QStringList>
#include <QValueSpaceObject>
#include <QSet>
#include <QDataStream>

// declare QDeviceIndicatorsProviderPrivate
class QDeviceIndicatorsProviderPrivate
{
public:
    QValueSpaceObject *m_provider;
    QStringList m_supported;

    static QSet<QString> providedIndicators;
};
QSet<QString> QDeviceIndicatorsProviderPrivate::providedIndicators;

/*!
    \class QDeviceIndicatorsProvider
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer
    \brief The QDeviceIndicatorsProvider class provides the backend for the QDeviceIndicator API.

    QDeviceIndicatorsProvider derived types control the status of device
    indicators.  During construction, derived types usually call
    addSupportedIndicator() followed by setIndicatorSupportedStates() to set up the indicators they provide. Derived classes
    must implement the changeIndicatorState() and changeIndicatorAttribute() functions which perform the actual hardware
    operations involved.

    Communication between the frontend QDeviceIndicators and backend QDeviceIndicatorsProvider is facilitated
    through the Qt Extended Value Space.  Whenever the state or attributes of an indicator are modified, the status
    of the indicators should be reflected in the Value Space via setIndicatorState() and setIndicatorAttribute().

    The following table shows a few common attributes and how they could be represented.
    \table
    \header
        \o Attribute
        \o type(units)
    \row
        \o Period (Flash state)
        \o int (ms)
    \row
        \o Duty Cycle(Flash state)
        \o int (%)
    \row
        \o TransitionTime (Throb state)
        \o int (ms)
    \row
        \o Color
        \o QColor
    \endtable

    Because the indicator names and supported states and attributes are device specific, it is expected
    that the implementer of the QDeviceIndicatorsProvider derived class will provide these to the user
    of QDeviceIndicators.  If the possible values a particular attribute need to be queryable, then another attribute
    can be made specifically for this eg a SupportedColors attribute whose type is QList<QColor>.  Generally this
    would not be necessary since the possible supported values should be provided to the user of QDeviceIndicators.

    This class is part of the Qt Extended server and a specific implementation should be provided
    as part of a server task.

    \sa QDeviceIndicators

 */

/*!
  Create a new QDeviceIndicatorsProvider instance with the specified \a parent.
 */
QDeviceIndicatorsProvider::QDeviceIndicatorsProvider(QObject *parent)
: QObject(parent), d(new QDeviceIndicatorsProviderPrivate)
{
    d->m_provider = new QValueSpaceObject("/Hardware/IndicatorLights", this);
    QObject::connect(d->m_provider, SIGNAL(itemSetValue(QByteArray,QVariant)),
                     this, SLOT(itemSetValue(QByteArray,QVariant)));
}

/*! \internal */
void QDeviceIndicatorsProvider::itemSetValue(const QByteArray &attribute,
                                           const QVariant &data)
{
    const QByteArray prefix = "Hardware/IndicatorLights/";
    QByteArray attr=attribute;
    if (attr.startsWith('/'))
        attr = attribute.mid(1);
    if (attr.startsWith(prefix))
        attr = attr.mid(prefix.size());

    for(int ii = 0; ii < d->m_supported.count(); ++ii)
    {
        if(attr.startsWith(d->m_supported.at(ii).toLatin1()))
        {
            if (!attr.contains('/'))
            {
                int val = data.toInt();
                if(val >= 0 && val <= QDeviceIndicators::Throb)
                {
                    QDeviceIndicators::IndicatorState status =
                        (QDeviceIndicators::IndicatorState)val;
                    changeIndicatorState(attr, status);
                }
            } else {
                QString indicatorName = attr.left(attr.indexOf('/'));
                QString indicatorAttribute = attr.mid(attr.indexOf('/')+1);
                changeIndicatorAttribute(indicatorName,indicatorAttribute, data);
            }

            return;
        }
    }
    // Not our indicator
}

/*!
    Add an \a indicator that this QDeviceIndicatorsProvider instance will
    provide.  changeIndicatorState() callbacks will only occur for indicators added
    in this way or with setSupportedIndicators().  This function will have no effect
    and produce a warning if an indicator already exists that is already being provided
    by this or another provider.
    
    The initial state of the \a indicator is QDeviceIndicators::Off.

    \sa setSupportedIndicators()
 */
void QDeviceIndicatorsProvider::addSupportedIndicator(const QString &indicator)
{
    if (QDeviceIndicatorsProviderPrivate::providedIndicators.contains(indicator))
    {
        qWarning("QDeviceIndicatorsProvider: Indicator '%s' already supported",
                indicator.toLatin1().constData());
        return;
    }

    QDeviceIndicatorsProviderPrivate::providedIndicators.insert(indicator);
    d->m_provider->setAttribute(indicator,(int)QDeviceIndicators::Off);
    d->m_supported.append(indicator);
}

/*!
    Sets the published list of the supported states of \a indicator
    to \a states.

    This function has no effect if \a indicator has not previously
    been set up.

    \sa setIndicatorState()
*/
void QDeviceIndicatorsProvider::setIndicatorSupportedStates
    (const QString &indicator, QList<QDeviceIndicators::IndicatorState> states)
{
    QList<QVariant> supportedStates;
    for (int i=0; i < states.count(); i++ )
        supportedStates << (int)states.at(i);

    if(d->m_supported.contains(indicator))
        d->m_provider->setAttribute(indicator + "/SupportedStates", supportedStates);
}

/*!
    \fn void QDeviceIndicatorsProvider::setSupportedIndicators(const QStringList &indicators)

    Set the \a indicators that this QDeviceIndicatorsProvider instance will
    provide.  changeIndicatorState() callbacks will only occur for indicators set
    in this way or added with addSupportedIndicator().  If an indicator in the
    in the \a indicators list is already provided by another QDeviceIndicators instance,
    then that indicator is not set and a warning is produced.

    The initial state of the \a indicators is QDeviceIndicators::Off.

    \sa addSupportedIndicator()
 */
void QDeviceIndicatorsProvider::setSupportedIndicators(const QStringList &in)
{
    QStringList list = in;
    for(int ii = 0; ii < in.count(); ++ii) {
        if(QDeviceIndicatorsProviderPrivate::providedIndicators.contains(in.at(ii))) {
            if (!d->m_supported.contains(in.at(ii)))
            {
                qWarning("QDeviceIndicatorsProvider: Indicator '%s' already supported by another "
                         "QDeviceIndicatorsProvider instance.", in.at(ii).toLatin1().constData());
                list.removeAt(list.indexOf(in.at(ii)));

            }
        }
    }

    for(int ii = 0; ii < d->m_supported.count(); ++ii) {
        QDeviceIndicatorsProviderPrivate::providedIndicators.remove(d->m_supported.at(ii));
        d->m_provider->removeAttribute(d->m_supported.at(ii));
    }

    for(int ii = 0; ii < list.count(); ++ii) {
        QDeviceIndicatorsProviderPrivate::providedIndicators.insert(list.at(ii));
        d->m_provider->setAttribute(list.at(ii), (int)QDeviceIndicators::Off);
    }

    d->m_supported = list;
}

/*!
    Set the published \a indicator state to \a state.  This is generally called
    during the changeIndicatorState() callback to indicate that the state was
    successfully changed or at startup to set the initial state, but it can be
    called at any time.

    This function has no effect if indicator has not previously been set up.

    \sa setIndicatorAttribute()
 */
void QDeviceIndicatorsProvider::setIndicatorState(const QString &indicator, QDeviceIndicators::IndicatorState state)
{
    for(int ii = 0; ii < d->m_supported.count(); ++ii) {
        if(d->m_supported.at(ii) == indicator) {
            d->m_provider->setAttribute(d->m_supported.at(ii), (int)state);
        }
    }
}

/*!
    Sets the published \a attribute of \a indicator to \a value.
    This is generally called during the changeIndicatorAttribute()
    callback to reflect the successful change in attribute or at startup
    to publish the attribute's initial value, but it can be
    called at any time.

    This function has no effect if \a indicator has not previously been added or set.

    \sa setIndicatorState()
*/
void QDeviceIndicatorsProvider::setIndicatorAttribute(const QString &indicator, const QString &attribute, const QVariant &value)
{
    for(int ii = 0; ii < d->m_supported.count(); ++ii) {
        if(d->m_supported.at(ii) == indicator) {
            d->m_provider->setAttribute(indicator + "/" + attribute, value);
        }
    }
}

/*!
    \fn void QDeviceIndicatorsProvider::changeIndicatorState(const QString &indicator, QDeviceIndicators::IndicatorState state) = 0

    Called when the \a indicator should be set to \a state.  This is usually
    in response to a QDeviceIndicators::setIndicatorState() call. Subclasses should implement
    the hardware specific operations from this function.

    \sa changeIndicatorAttribute()
 */

/*!
    \fn void QDeviceIndicatorsProvider::changeIndicatorAttribute(const QString &indicator, const QString &attribute, const QVariant &value) = 0

    Called when the \a attribute of \a indicator should be set to a new \a value.  This is usually
    in response to a QDeviceIndicators::setIndicatorAttribute() call. Subclasses should implement the
    hardware specific operations from this function.

    \sa changeIndicatorState()
 */
