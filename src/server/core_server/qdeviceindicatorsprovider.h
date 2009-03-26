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

#ifndef QDEVICEINDICATORSPROVIDER_H
#define QDEVICEINDICATORSPROVIDER_H

#include <QObject>
#include <QDeviceIndicators>

class QDeviceIndicatorsProviderPrivate;
class QDeviceIndicatorsProvider : public QObject
{
Q_OBJECT
public:
    QDeviceIndicatorsProvider(QObject *parent = 0);

protected:
    void addSupportedIndicator(const QString &);
    void setIndicatorSupportedStates(const QString &,
            QList<QDeviceIndicators::IndicatorState>);

    void setSupportedIndicators(const QStringList &);
    void setIndicatorState(const QString &, QDeviceIndicators::IndicatorState);
    void setIndicatorAttribute(const QString &name, const QString &attribute,
                                const QVariant &value);

    virtual void changeIndicatorState(const QString &,
                                      QDeviceIndicators::IndicatorState) = 0;
    virtual void changeIndicatorAttribute(const QString &name,
                            const QString &attribute, const QVariant &value)= 0;

private slots:
    void itemSetValue(const QByteArray &attribute, const QVariant &data);

private:
    QDeviceIndicatorsProviderPrivate *d;
};

#endif
