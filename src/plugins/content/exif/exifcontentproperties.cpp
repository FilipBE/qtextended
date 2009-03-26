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
#include "exifcontentproperties.h"
#include <QtDebug>

ExifContentProperties::ExifContentProperties(const QContent &content)
    : m_content(content)
{
    QIODevice *device = m_content.open();

    if (device) {
        m_data.loadFromJpeg(device);

        device->close();

        delete device;
    }
}

ExifContentProperties::~ExifContentProperties()
{
    QIODevice *device = m_content.open(QIODevice::ReadWrite);

    if (device) {
        m_data.saveToJpeg(device);

        device->close();

        delete device;
    }
}

bool ExifContentProperties::isReadOnly() const
{
    return false;
}

QStringList ExifContentProperties::keys() const
{
    QStringList keys;

    foreach (QExifImageHeader::ImageTag tag, m_data.imageTags()) {
        QString key = tagKey(tag);

        if (!key.isEmpty())
            keys.append(key);
    }

    return keys;
}

QStringList ExifContentProperties::supportedKeys() const
{
    return QStringList()
            << QLatin1String("ImageWidth")
            << QLatin1String("ImageLength")
            << QLatin1String("XResolution")
            << QLatin1String("YResolution")
            << QLatin1String("CreationDate")
            << QLatin1String("Description")
            << QLatin1String("Make")
            << QLatin1String("Model")
            << QLatin1String("Software")
            << QLatin1String("Artist")
            << QLatin1String("Copyright")
            << QLatin1String("Comment")
            << QLatin1String("DateTimeOriginal")
            << QLatin1String("DateTimeDigitized");
}

QVariant ExifContentProperties::value(const QString &key, const QVariant &defaultValue) const
{
    QExifImageHeader::ImageTag tag = keyTag(key);

    QExifValue value = m_data.value(tag);

    switch(value.type()) {
    case QExifValue::Byte:
        return value.toByte();
    case QExifValue::Ascii:
        switch(tag) {
        case QExifImageHeader::DateTime:
        case QExifImageHeader::DateTimeOriginal:
        case QExifImageHeader::DateTimeDigitized:
            return value.toDateTime();
        default:
            return value.toString();
        }
    case QExifValue::Short:
        return value.toShort();
    case QExifValue::Long:
        return value.toLong();
    case QExifValue::Rational:
        return QVariant::fromValue(value.toRational());
    case QExifValue::Undefined:
        return value.toByteArray();
    case QExifValue::SignedLong:
        return value.toSignedLong();
    case QExifValue::SignedRational:
        return QVariant::fromValue(value.toSignedRational());
    default:
        return defaultValue;
    }
}

bool ExifContentProperties::setValue( const QString &key, const QVariant &value )
{
    QExifImageHeader::ImageTag tag = keyTag(key);

    switch (keyTag(key)) {
    case QExifImageHeader::ImageWidth:
    case QExifImageHeader::ImageLength:
    case QExifImageHeader::XResolution:
    case QExifImageHeader::YResolution:
        m_data.setValue(tag, value.toUInt());
        return true;
    case QExifImageHeader::ImageDescription:
    case QExifImageHeader::Make:
    case QExifImageHeader::Model:
    case QExifImageHeader::Software:
    case QExifImageHeader::Artist:
    case QExifImageHeader::Copyright:
        m_data.setValue(tag, value.toString() );
        return true;
    case QExifImageHeader::DateTime:
    case QExifImageHeader::DateTimeOriginal:
    case QExifImageHeader::DateTimeDigitized:
        m_data.setValue(tag, value.toDateTime() );
        return true;
    case QExifImageHeader::UserComment:
        m_data.setValue(tag, value.toByteArray() );
        return true;
    default:
        return false;
    }
}

bool ExifContentProperties::contains(const QString &key) const
{
    QExifImageHeader::ImageTag tag = keyTag(key);

    return m_data.contains(tag);
}

bool ExifContentProperties::remove( const QString &key )
{
    QExifImageHeader::ImageTag tag = keyTag(key);

    m_data.remove(tag);

    return true;
}

QString ExifContentProperties::tagKey(QExifImageHeader::ImageTag tag) const
{
    switch (tag) {
    case QExifImageHeader::ImageWidth:        return QLatin1String( "ImageWidth" );
    case QExifImageHeader::ImageLength:       return QLatin1String( "ImageLength" );
    case QExifImageHeader::XResolution:       return QLatin1String( "XResolution" );
    case QExifImageHeader::YResolution:       return QLatin1String( "YResolution" );
    case QExifImageHeader::DateTime:          return QLatin1String( "CreationDate" );
    case QExifImageHeader::ImageDescription:  return QLatin1String( "Description" );
    case QExifImageHeader::Make:              return QLatin1String( "Make" );
    case QExifImageHeader::Model:             return QLatin1String( "Model" );
    case QExifImageHeader::Software:          return QLatin1String( "Software" );
    case QExifImageHeader::Artist:            return QLatin1String( "Artist" );
    case QExifImageHeader::Copyright:         return QLatin1String( "Copyright" );
    default:
        return QString();
    }
}

QExifImageHeader::ImageTag ExifContentProperties::keyTag( const QString &key ) const
{
    if (key == QLatin1String("ImageWidth"))        return QExifImageHeader::ImageWidth;
    else if (key == QLatin1String("ImageLength"))  return QExifImageHeader::ImageLength;
    else if (key == QLatin1String("XResolution"))  return QExifImageHeader::XResolution;
    else if (key == QLatin1String("YResolution"))  return QExifImageHeader::YResolution;
    else if (key == QLatin1String("CreationDate")) return QExifImageHeader::DateTime;
    else if (key == QLatin1String("Description"))  return QExifImageHeader::ImageDescription;
    else if (key == QLatin1String("Make"))         return QExifImageHeader::Make;
    else if (key == QLatin1String("Model"))        return QExifImageHeader::Model;
    else if (key == QLatin1String("Software"))     return QExifImageHeader::Software;
    else if (key == QLatin1String("Artist"))       return QExifImageHeader::Artist;
    else if (key == QLatin1String("Copyright"))    return QExifImageHeader::Copyright;
    else
        return QExifImageHeader::ImageTag(0);

}
