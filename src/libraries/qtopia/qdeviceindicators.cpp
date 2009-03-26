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

#include "qdeviceindicators.h"
#include <QValueSpaceItem>
#include <QHash>
#include <QSet>
#include <QStringList>

// declare QDeviceIndicatorsPrivate
class QDeviceIndicatorsPrivate : public QObject
{
Q_OBJECT
public:
    QDeviceIndicatorsPrivate();

signals:
    void indicatorStateChanged(const QString &name,
                               QDeviceIndicators::IndicatorState newState);

private slots:
    void update();

public:
    typedef QHash<QString, QDeviceIndicators::IndicatorState> Indicators;
    Indicators m_states;
    QValueSpaceItem *m_item;
};
Q_GLOBAL_STATIC(QDeviceIndicatorsPrivate, deviceIndicators);

// declare QDeviceIndicatorsPrivate
QDeviceIndicatorsPrivate::QDeviceIndicatorsPrivate()
: QObject(0), m_item(0)
{
    m_item = new QValueSpaceItem("/Hardware/IndicatorLights");
    QObject::connect(m_item, SIGNAL(contentsChanged()), this, SLOT(update()));
    update();
}

void QDeviceIndicatorsPrivate::update()
{
    QStringList subPaths = m_item->subPaths();

    QSet<QString> seen;
    for(int ii = 0; ii < subPaths.count(); ++ii) {
        const QString &subPath = subPaths.at(ii);

        seen.insert(subPath);
        QDeviceIndicators::IndicatorState newState =
            (QDeviceIndicators::IndicatorState)m_item->value(subPath).toInt();


        Indicators::Iterator iter = m_states.find(subPath);
        if(iter == m_states.end()) {
            m_states.insert(subPath, newState);
            emit indicatorStateChanged(subPath, newState);
        } else if(*iter != newState) {
            *iter = newState;
            emit indicatorStateChanged(subPath, newState);
        }
    }

    for(Indicators::Iterator iter = m_states.begin();
        iter != m_states.end(); ++iter) {

        const QString &subPath = iter.key();
        if(seen.contains(subPath))
            continue;

        QDeviceIndicators::IndicatorState newState =
            (QDeviceIndicators::IndicatorState)m_item->value(subPath,QDeviceIndicators::Off).toInt();

        if(*iter != newState) {
            *iter = newState;
            emit indicatorStateChanged(subPath, newState);
        }
    }
}

// define QDeviceIndicatorsPrivate

/*!
    \class QDeviceIndicators
    \inpublicgroup QtBaseModule

    \brief The QDeviceIndicators class allows applications to control indicator lights.

    The QDeviceIndicators class can be used by applications to control visual
    lights on a device.  For example, many devices have an "Email" light that is
    illuminated whenever there are new messages.  Each light has a name, and is manipulated
    through it's state such as On or Flash and attributes such as Color.  The names of each indicator,
    and their supported states and attributes are device specific.
    Typically the implementer of QDeviceIndicatorsProvider would document these and
    provide them to the user of QDeviceIndicators.

    QDeviceIndicators acts as a thin convenience wrapper around entries in the
    Qt Extended Value Space.  The current status on indicators is located under the
    \c {/Hardware/IndicatorLights} path.  For example, the "Email" indicator
    state could be read and set directly through the
    \c {/Hardware/IndicatorLights/Email} item.

    \i {Note:} Because QDeviceIndicators uses Value Space, it is not thread
    safe and may only be used from an application's main thread.


    \ingroup hardware
    \sa QDeviceIndicatorsProvider
 */

/*!
    \enum QDeviceIndicators::IndicatorState

    Represents the state of a device indicator.
    \value Off The indicator light is not illuminated.
    \value On The indicator light is illuminated.
    \value Flash The indicator is flashing on and off.
    \value Throb The indicator is smoothly cycling from zero
        intensity to full intensity and back again
 */

/*!
    Create a new QDeviceIndicators instance with the provided \a parent.
 */
QDeviceIndicators::QDeviceIndicators(QObject *parent)
: QObject(parent), d(0)
{
    d = deviceIndicators();
    QObject::connect(d,
                    SIGNAL(indicatorStateChanged(QString,QDeviceIndicators::IndicatorState)),
                    this,
                    SIGNAL(indicatorStateChanged(QString,QDeviceIndicators::IndicatorState)));
}

/*!
    Destroys the instance.
 */
QDeviceIndicators::~QDeviceIndicators()
{
}

/*!
    Returns the list of all indicators supported by the device.

    \sa isIndicatorSupported()
 */
QStringList QDeviceIndicators::supportedIndicators() const
{
    return d->m_states.keys();
}

/*!
    Returns true if the indicator \a name is supported by the device.

    \sa supportedIndicators()
 */
bool QDeviceIndicators::isIndicatorSupported(const QString &name)
{
    return d->m_states.contains(name);
}

/*!
    Returns true if the indicator \a name supports a given \a state.
    Returns false if the \a state is not supported or the indicator \a name
    is not recognised.

    \sa indicatorState(), setIndicatorState()
 */
bool QDeviceIndicators::isIndicatorStateSupported(const QString &name,
                                                  IndicatorState state)
{
    if (!d->m_states.contains(name))
        return false;
    QList<QVariant> states = d->m_item->value(name + "/SupportedStates").toList();
    for (int i=0; i < states.count(); i++ )
    {
        if (state == states.at(i).toInt() )
            return true;
    }
    return false;
}

/*!
    Returns the value of the \a attribute of the given indcator's \a name.
    An invalid QVariant is returned if the indicator \a name does not exist
    or \a attribute is not supported.
*/
QVariant QDeviceIndicators::indicatorAttribute(const QString &name, const QString &attribute) const
{
    if (!d->m_states.contains(name))
        return QVariant();
    else
        return d->m_item->value(name + '/' + attribute);
}
/*!
    Attempts to set the \a attribute of indicator \a name to \a value.  Setting an attribute
    is an asynchronous event.  Honoring or ignoring the request is up to the implementation of
    QDeviceIndicatorsProvider, typically valid requests would always be honored.
*/
void QDeviceIndicators::setIndicatorAttribute(const QString &name, const QString &attribute,
                                    const QVariant &value)
{
    if (isIndicatorSupported(name))
    {
        d->m_item->setValue(name + '/' + attribute, value);
        d->m_item->sync();
    }
}

/*!
    Returns the current indicator state for \a name.

    \sa isIndicatorStateSupported()
 */
QDeviceIndicators::IndicatorState QDeviceIndicators::indicatorState(const QString &name)
{
    QDeviceIndicatorsPrivate::Indicators::ConstIterator iter = d->m_states.find(name);
    if(iter == d->m_states.end())
        return Off;
    else
        return *iter;
}

/*!
    Attempts to change the indicator \a name to the provided \a state.  Setting
    an indicator is an asynchronous event.  If necessary, callers should monitor
    the indicatorStateChanged() signal to determine when the state change occurs.

    \sa isIndicatorStateSupported()
 */
void QDeviceIndicators::setIndicatorState(const QString &name,
                                          IndicatorState state)
{
    d->m_item->setValue(name, state);
    d->m_item->sync();
}

/*!
    \fn void QDeviceIndicators::indicatorStateChanged(const QString &name, IndicatorState newState)

    Emitted whenever the indicator \a name changes state.  \a newState will be
    set to the new indicator state.

    \sa indicatorState(), setIndicatorState()
 */

#include "qdeviceindicators.moc"
