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

#ifndef CITYINFO_H
#define CITYINFO_H

// Qt4 Headers
#include <QFrame>
#include <QDateTime>

class CityInfo : public QFrame
{
    Q_OBJECT
public:
    CityInfo(QWidget *parent, Qt::WFlags f = 0 );

    QString zone() const { return mZone; }
    QSize sizeHint() const;
QSizePolicy sizePolicy() const
     { return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed); }
    QString text() const;
public slots:
    void setZone(const QString &zone);
    void setUtcTime(const QDateTime &);

protected:
    void paintEvent( QPaintEvent *);

private:

    QString mZone;
    QDateTime mUtc;
};

#endif
