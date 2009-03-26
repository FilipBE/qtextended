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

#ifndef QMEDIAHELIXSETTINGSCONTROL_H
#define QMEDIAHELIXSETTINGSCONTROL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <qtopiaglobal.h>


class QMediaHelixSettingsControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaHelixSettingsControl : public QObject
{
    Q_OBJECT

public:
    QMediaHelixSettingsControl(QObject* parent = 0);
    ~QMediaHelixSettingsControl();

    QStringList availableOptions();

    void setOption(QString const& name, QVariant const& value);
    QVariant optionValue(QString const& name);

    static QString name();

signals:
    void optionChanged(QString name, QVariant value);

private:
    Q_DISABLE_COPY(QMediaHelixSettingsControl);

    QMediaHelixSettingsControlPrivate*   d;
};


#endif
