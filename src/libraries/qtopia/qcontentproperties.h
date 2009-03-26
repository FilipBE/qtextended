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
#ifndef QCONTENTPROPERTIES_H
#define QCONTENTPROPERTIES_H

#include <QVariant>
#include <qtopiaglobal.h>

class QContent;
class QContentPropertiesPrivate;

class QTOPIA_EXPORT QContentProperties
{
    Q_DISABLE_COPY(QContentProperties)
public:
    QContentProperties( const QContent &content );
    ~QContentProperties();

    QContent content() const;

    bool isReadOnly() const;

    QStringList keys() const;
    QStringList supportedKeys() const;

    QVariant value( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    bool setValue( const QString &key, const QVariant &value );

    bool contains( const QString &key ) const;

    bool remove( const QString &key );

private:
    QContentPropertiesPrivate *d;
};

class QTOPIA_EXPORT QContentPropertiesEngine
{
public:
    virtual ~QContentPropertiesEngine();

    virtual bool isReadOnly() const = 0;

    virtual QStringList keys() const = 0;
    virtual QStringList supportedKeys() const = 0;

    virtual QVariant value( const QString &key, const QVariant &defaultValue ) const = 0;
    virtual bool setValue( const QString &key, const QVariant &value ) = 0;

    virtual bool contains( const QString &key ) const = 0;
    virtual bool remove( const QString &key ) = 0;
};

#endif
