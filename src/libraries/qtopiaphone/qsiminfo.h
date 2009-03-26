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

#ifndef QSIMINFO_H
#define QSIMINFO_H

#include <qcomminterface.h>
#include <QDateTime>

class QTOPIAPHONE_EXPORT QSimInfo : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(QString identity READ identity)
    Q_PROPERTY(QDateTime insertedTime READ insertedTime)
public:
    explicit QSimInfo( const QString& service = QString(), QObject *parent = 0,
                       QCommInterface::Mode mode = Client );
    ~QSimInfo();

    QString identity() const;
    QDateTime insertedTime() const;

signals:
    void inserted();
    void removed();
    void notInserted();

protected:
    void setIdentity( const QString& identity );
};

#endif
