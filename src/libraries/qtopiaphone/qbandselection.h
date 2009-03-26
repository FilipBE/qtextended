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

#ifndef QBANDSELECTION_H
#define QBANDSELECTION_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <QList>

class QTOPIAPHONE_EXPORT QBandSelection : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(BandMode)
public:
    explicit QBandSelection( const QString& service = QString(),
                             QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QBandSelection();

    enum BandMode
    {
        Automatic,
        Manual
    };

public slots:
    virtual void requestBand();
    virtual void requestBands();
    virtual void setBand( QBandSelection::BandMode mode, const QString& value );

signals:
    void band( QBandSelection::BandMode mode, const QString& value );
    void bands( const QStringList& list );
    void setBandResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QBandSelection::BandMode)

#endif
