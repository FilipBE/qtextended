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
#ifndef EXIFCONTENTPROPERTIES_H
#define EXIFCONTENTPROPERTIES_H

#include <QContentPropertiesEngine>
#include <QExifImageHeader>
#include <QContent>

class ExifContentProperties : public QContentPropertiesEngine
{
public:
    ExifContentProperties( const QContent &content );
    virtual ~ExifContentProperties();

    virtual bool isReadOnly() const;

    virtual QStringList keys() const;
    virtual QStringList supportedKeys() const;

    virtual QVariant value( const QString &key, const QVariant &defaultValue ) const;
    virtual bool setValue( const QString &key, const QVariant &value );

    virtual bool contains( const QString &key ) const;
    virtual bool remove( const QString &key );
private:
    QString tagKey(QExifImageHeader::ImageTag tag) const;
    QExifImageHeader::ImageTag keyTag(const QString &key) const;

    QContent m_content;
    QExifImageHeader m_data;
};

#endif
