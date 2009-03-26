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

#ifndef QTIMEZONESELECTOR_H
#define QTIMEZONESELECTOR_H

#include <qtopiaglobal.h>
#include <QWidget>

class QTimeZoneSelectorPrivate;

class QTOPIA_EXPORT QTimeZoneSelector : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentZone READ currentZone WRITE setCurrentZone)
    Q_PROPERTY(bool allowNoZone READ allowNoZone WRITE setAllowNoZone)
public:
    explicit QTimeZoneSelector( QWidget* parent = 0 );
    ~QTimeZoneSelector();

    QString currentZone() const;
    void setCurrentZone( const QString& id );

    void setAllowNoZone(bool);
    bool allowNoZone() const;

signals:
    void zoneChanged( const QString& id );

private slots:
    void tzActivated( int index );

private:
    QTimeZoneSelectorPrivate *d;
};

#endif
