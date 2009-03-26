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
#ifndef QOTAREADER_H
#define QOTAREADER_H

#include <qwbxmlreader.h>

#include <QMap>
#include <QVariant>
#include <qtopianetworkinterface.h>

class QOtaCharacteristic;
class QOtaCharacteristicList;
class QOtaCharacteristicPrivate;

typedef QMap<QString, QString> QOtaParameters;
typedef QMap<QString, QList<QOtaCharacteristic> > QOtaCharacteristicListBase;

class QTOPIAPHONE_EXPORT QOtaCharacteristic
{
public:
    QOtaCharacteristic();
    QOtaCharacteristic( const QOtaCharacteristic& c );
    ~QOtaCharacteristic();

    void clear();

    const QString& type() const;
    void setType( const QString& type );

    const QOtaParameters& parms() const;
    void addParm( const QString& name, const QString& value );

    const QOtaCharacteristicList& children() const;
    void addChild( QOtaCharacteristic& child );

    QOtaCharacteristic& operator=( const QOtaCharacteristic& c );

private:
    QOtaCharacteristicPrivate *d;
};


class QTOPIAPHONE_EXPORT QOtaCharacteristicList : public QOtaCharacteristicListBase
{
public:
    QOtaCharacteristicList() {}
    QOtaCharacteristicList( const QOtaCharacteristicList& list )
        : QOtaCharacteristicListBase( list ) {}
    ~QOtaCharacteristicList() {}

    QtopiaNetworkProperties toConfig() const;

    QString parameter( const QString& name1 ) const;
    QString parameter( const QString& name1, const QString& name2 ) const;
    QString parameter( const QString& name1, const QString& name2,
                       const QString& name3 ) const;
    QString parameter( const QString& name1, const QString& name2,
                       const QString& name3, const QString& name4 ) const;

    QString appParameter( const QString& app, const QString& name1 ) const;
    QString appParameter( const QString& app, const QString& name1,
                          const QString& name2 ) const;
    QString appParameter( const QString& app, const QString& name1,
                          const QString& name2, const QString& name3 ) const;

private:
    const QOtaParameters *section( const QString& name1 ) const;
    const QOtaParameters *section
        ( const QString& name1, const QString& name2 ) const;
    const QOtaParameters *section
        ( const QString& name1, const QString& name2,
          const QString& name3 ) const;

    const QOtaCharacteristic *appSection( const QString& app ) const;
    const QOtaParameters *appSection
        ( const QString& app, const QString& name1 ) const;
    const QOtaParameters *appSection
        ( const QString& app, const QString& name1,
          const QString& name2 ) const;

    static void add( QtopiaNetworkProperties& cfg,
                     const QString& name, const QString& value );

};


class QTOPIAPHONE_EXPORT QOtaReader : public QWbXmlReader
{
public:
    enum QOtaType { Nokia, Wap };

    QOtaReader( QOtaType type=Nokia );
    virtual ~QOtaReader();

    // Parse a binary-WBXML characteristics stream.
    QOtaCharacteristicList *parseCharacteristics( const QByteArray& input );
    QOtaCharacteristicList *parseCharacteristics( QIODevice& input );

    // Parse a plain-XML characteristics stream.
    static QOtaCharacteristicList *parseCharacteristics
        ( QXmlReader *reader, const QXmlInputSource& input );
};

#endif
