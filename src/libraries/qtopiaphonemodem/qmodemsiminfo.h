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

#ifndef QMODEMSIMINFO_H
#define QMODEMSIMINFO_H

#include <qsiminfo.h>

class QModemService;
class QAtResult;
class QModemSimInfoPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimInfo : public QSimInfo
{
    Q_OBJECT
public:
    explicit QModemSimInfo( QModemService *service );
    ~QModemSimInfo();

protected slots:
    void simInserted();
    void simRemoved();

private slots:
    void requestIdentity();
    void cimi( bool ok, const QAtResult& result );
    void serviceItemPosted( const QString& item );

private:
    QModemSimInfoPrivate *d;

    static QString extractIdentity( const QString& content );
};

#endif
