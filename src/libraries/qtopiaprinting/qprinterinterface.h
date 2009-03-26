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

#ifndef QPRINTERINTERFACE_H
#define QPRINTERINTERFACE_H

#include <qtopiaglobal.h>
#include <QMap>
#include <qcontent.h>

class QTOPIAPRINTING_EXPORT QtopiaPrinterInterface
{
public:
    virtual ~QtopiaPrinterInterface();

    virtual void print( QMap<QString, QVariant> options ) = 0;
    virtual void printFile( const QString &fileName, const QString &mimeType = QString() ) = 0;
    virtual void printHtml( const QString &html ) = 0;
    virtual bool abort() = 0;
    virtual QString name() = 0;
    virtual bool isAvailable() = 0;

protected:
    int m_opType;
};

Q_DECLARE_INTERFACE(QtopiaPrinterInterface,
        "com.trolltech.Qtopia.QtopiaPrinterInterface")

class QTOPIAPRINTING_EXPORT QtopiaPrinterPlugin : public QObject, public QtopiaPrinterInterface
{
    Q_OBJECT
public:
    explicit QtopiaPrinterPlugin( QObject *parent = 0 );
    virtual ~QtopiaPrinterPlugin();
};

#endif

