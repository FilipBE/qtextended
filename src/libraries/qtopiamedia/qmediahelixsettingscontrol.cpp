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

#include <QAbstractIpcInterface>

#include "qmediahelixsettingscontrol.h"

// {{{ QMediaHelixSettingsControlPrivate
class QMediaHelixSettingsControlPrivate : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QMediaHelixSettingsControlPrivate():
        QAbstractIpcInterface("/Media/Control/Helix",
                              QMediaHelixSettingsControl::name(),
                              "HelixGlobalSettings")
    {
    }

    QStringList availableOptions()
    {
        return value("AvailableOptions").toStringList();
    }

    QVariant optionValue(QString const& name)
    {
        return value(name);
    }

public slots:
    void setOption(QString const& name, QVariant const& value)
    {
        invoke(SLOT(setOption(QString,QVariant)), name, value);
    }

signals:
    void optionChanged(QString name, QVariant value);
};
// }}}


/*!
    \class QMediaHelixSettingsControl
    \inpublicgroup QtMediaModule

    \brief The QMediaHelixSettingsControl class is used to alter settings of the helix engine.

    This class can be used to query and alter options within the Helix DNA 
    Client engine using the Helix preferences mechanism.

    \ingroup multimedia

    Options are set using a QString and QVariant pair. The QString being the
    option to set and the QVariant being the value of the option.

    \code
        QMediaHelixSettingsControl settings;
        settings.setOption( "ServerTimeOut", 10 );
    \endcode

    Options are applied at the global scope and will affect all media played
    using the Helix DNA Client engine.

    A list of options available to set and query can be obtained by using
    availableOptions().

    It is only available when the Helix Client is being used as the media
    provider in the Media System.
*/

/*!
    Create a QMediaHelixSettingsControl

    The \a parent is the Parent QObject.
*/

QMediaHelixSettingsControl::QMediaHelixSettingsControl
(
 QObject*               parent
):
    QObject(parent)
{
    d = new QMediaHelixSettingsControlPrivate;

    connect(d, SIGNAL(optionChanged(QString,QVariant)),
            this, SIGNAL(optionChanged(QString,QVariant)));
}

/*!
    Destroy a QMediaHelixSettingsControl
*/

QMediaHelixSettingsControl::~QMediaHelixSettingsControl()
{
    delete d;
}

/*!
    Return a list of options supported by the Qt Extended Helix integration.
*/

QStringList QMediaHelixSettingsControl::availableOptions()
{
    return d->availableOptions();
}

/*!
    Set a Helix option by name into the Helix Client Engine.

    The \a name parameter is the name of the helix option to set.
    The \a value parameter is the value of the helix option.
*/

void QMediaHelixSettingsControl::setOption(QString const& name, QVariant const& value)
{
    d->setOption(name, value);
}

/*!
    Retrieve a value by name from the Helix Client Engine.

    The \a name parameter is the name of the helix option to fetch.
*/

QVariant QMediaHelixSettingsControl::optionValue(QString const& name)
{
    return d->optionValue(name);
}

/*!
    Return the name of the control.
*/

QString QMediaHelixSettingsControl::name()
{
    return QLatin1String("Settings");
}

/*!
    \fn QMediaHelixSettingsControl::optionChanged(QString name, QVariant value);

    This Signal is emitted when the system has processed the option changed
    request initiated with setOption. The \a name and \a value paramters
    match the values specified with the setOption call.
*/

#include "qmediahelixsettingscontrol.moc"

