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

#ifndef BLUETOOTHPLUGIN_H
#define BLUETOOTHPLUGIN_H

#include <qprinterinterface.h>

class BluetoothPrinterPluginPrivate;

class QTOPIA_PLUGIN_EXPORT BluetoothPrinterPlugin : public QtopiaPrinterPlugin
{
    Q_OBJECT
public:
    BluetoothPrinterPlugin( QObject *parent = 0 );
    ~BluetoothPrinterPlugin();

    void print( QMap<QString, QVariant> options );
    void printFile( const QString &fileName, const QString &mimeType = QString() );
    void printHtml( const QString &html );
    bool abort();
    bool isAvailable();
    QString name();

private:
    BluetoothPrinterPluginPrivate *d;
};

#endif
