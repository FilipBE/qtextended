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

#ifndef QMEDIAHELIXSETTINGSSERVER_H
#define QMEDIAHELIXSETTINGSSERVER_H

#include <QAbstractIpcInterface>

class IHXClientEngine;

namespace qtopia_helix
{

class QMediaHelixSettingsServerPrivate;

class QMediaHelixSettingsServer : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QMediaHelixSettingsServer(IHXClientEngine* engine);
    ~QMediaHelixSettingsServer();

public slots:
    void setOption(QString const& name, QVariant const& value);

signals:
    void optionChanged(QString name, QVariant value);

private:
    QMediaHelixSettingsServerPrivate*   d;
};

}   // ns qtopia_helix

#endif
