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

#ifndef QTHEMEITEMATTRIBUTE_H
#define QTHEMEITEMATTRIBUTE_H

#include <qtopiaglobal.h>
#include <QVariant>
#include <QMap>
#include <QStringList>

class QThemeItemAttributePrivate;

class QTOPIATHEMING_EXPORT QThemeItemAttribute
{

public:
    QThemeItemAttribute();
    QThemeItemAttribute(const QVariant &value);
    ~QThemeItemAttribute();

    QVariant value(const QString &state = "default") const;
    void setValue(const QVariant &value, const QString &state = "default");
    void setDefaultValue(const QVariant &);
    void reset(const QString &state = "default");
    bool isModified(const QString &state = "default") const;
    QStringList states() const;

private:
    QMap<QString, QVariant> _value;
    QVariant _defaultValue;
};

#endif
