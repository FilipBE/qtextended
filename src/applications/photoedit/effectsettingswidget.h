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
#ifndef EFFECTSETTINGSWIDGET_H
#define EFFECTSETTINGSWIDGET_H

#include "effectmodel.h"

#include <QWidget>

class EffectSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    EffectSettingsWidget(const QList<EffectParameter> &parameters, QWidget *parent = 0);

    QMap<QString, QVariant> settings() const;

    void resetDefaults();

private:
    QList<EffectParameter> m_parameters;
    QList<QWidget *> m_selectors;
};

#endif
