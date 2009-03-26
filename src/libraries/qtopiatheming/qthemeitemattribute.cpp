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

#include <QThemeItemAttribute>
#include <QDebug>

QThemeItemAttribute::QThemeItemAttribute()
{
}

QThemeItemAttribute::QThemeItemAttribute(const QVariant &value)
{
    setDefaultValue(value);
}

QThemeItemAttribute::~QThemeItemAttribute()
{
}

QVariant QThemeItemAttribute::value(const QString &state) const
{
    if (!_value.contains(state))
        return _value["default"];
    return _value[state];
}

void QThemeItemAttribute::setValue(const QVariant &value, const QString &state)
{
    _value[state] = value;
}

void QThemeItemAttribute::setDefaultValue(const QVariant &value)
{
    _defaultValue = value;
    _value["default"] = value;
}

void QThemeItemAttribute::reset(const QString &state)
{
    _value[state] = _defaultValue;
}

bool QThemeItemAttribute::isModified(const QString &state) const
{
    return (_value[state] != _defaultValue);
}

QStringList QThemeItemAttribute::states() const
{
    QStringList keys;
    foreach(QString key, _value.keys())
    keys << key;
    return keys;
}

