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

#ifndef QMEDIAABSTRACTCONTROLSERVER_H
#define QMEDIAABSTRACTCONTROLSERVER_H

#include <QObject>
#include <QString>
#include <QVariant>

#include <qtopiaglobal.h>


class QMediaHandle;


class QMediaAbstractControlServerPrivate;

class QTOPIAMEDIA_EXPORT QMediaAbstractControlServer : public QObject
{
    Q_OBJECT

public:
    QMediaAbstractControlServer(QMediaHandle const& handle,
                                QString const& name,
                                QObject* parent = 0);
    ~QMediaAbstractControlServer();

signals:
    void controlAvailable(QString const& name);
    void controlUnavailable(QString const& name);

protected:
    void setValue(QString const& name, QVariant const& value);

    void proxyAll();

private slots:
    void itemRemove(const QByteArray &attribute);
    void itemSetValue(const QByteArray &attribute, const QVariant &value);

private:
    QMediaAbstractControlServerPrivate* d;
};

#endif
