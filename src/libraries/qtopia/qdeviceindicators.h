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

#ifndef QDEVICEINDICATORS_H
#define QDEVICEINDICATORS_H

#include <QObject>
#include <qtopiaglobal.h>
#include <QVariant>

class QDeviceIndicatorsPrivate;
class QTOPIA_EXPORT QDeviceIndicators : public QObject
{
Q_OBJECT
public:
    explicit QDeviceIndicators(QObject *parent = 0);
    virtual ~QDeviceIndicators();

    enum IndicatorState {Off, On, Flash, Throb};

    bool isIndicatorSupported(const QString &);
    bool isIndicatorStateSupported(const QString &, IndicatorState);

    QVariant indicatorAttribute(const QString &name, const QString &attribute) const;
    void setIndicatorAttribute(const QString &name, const QString &attribute,
            const QVariant &value);

    IndicatorState indicatorState(const QString &);
    void setIndicatorState(const QString &name, IndicatorState state);
    QStringList supportedIndicators() const;

signals:
    void indicatorStateChanged(const QString &name, QDeviceIndicators::IndicatorState newState);

private:
    QDeviceIndicatorsPrivate *d;
};

#endif
