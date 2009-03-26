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
#include "effectsettingswidget.h"
#include "colorpicker.h"

#include <QFormLayout>
#include <QSlider>

EffectSettingsWidget::EffectSettingsWidget(const QList<EffectParameter> &parameters, QWidget *parent)
    : QWidget(parent)
    , m_parameters(parameters)
{
    QFormLayout *layout = new QFormLayout;

    foreach (EffectParameter parameter, parameters) {
        switch (parameter.type()) {
        case QVariant::Int:
            {
                QSlider *slider = new QSlider(Qt::Horizontal);
                slider->setRange(parameter.minimum().toInt(), parameter.maximum().toInt());
                slider->setValue(parameter.defaultValue().toInt());
                slider->setSingleStep(parameter.step().toInt());

                layout->addRow(parameter.name(), slider);
                m_selectors.append(slider);
            }
            break;
        case QVariant::Color:
            {
                ColorPicker *picker = new ColorPicker;
                picker->setColor(qvariant_cast<QColor>(parameter.defaultValue()));

                layout->addRow(parameter.name(), picker);
                m_selectors.append(picker);
            }
            break;
        default:
            m_selectors.append(0);
        }
    }

    setLayout(layout);
}

QMap<QString, QVariant> EffectSettingsWidget::settings() const
{
    QMap<QString, QVariant> settings;

    for (int i = 0; i < m_parameters.count(); ++i) {
        EffectParameter parameter = m_parameters.at(i);

        switch (parameter.type()) {
        case QVariant::Int:
            if (QSlider *slider = qobject_cast<QSlider *>(m_selectors.at(i)))
                settings.insert(parameter.key(), slider->value());
            break;
        case QVariant::Color:
            if (ColorPicker *picker = qobject_cast<ColorPicker *>(m_selectors.at(i)))
                settings.insert(parameter.key(), picker->color());
            break;
        default:
            break;
        }
    }
    return settings;
}

void EffectSettingsWidget::resetDefaults()
{
    for (int i = 0; i < m_parameters.count(); ++i) {
        EffectParameter parameter = m_parameters.at(i);

        switch (parameter.type()) {
        case QVariant::Int:
            if (QSlider *slider = qobject_cast<QSlider *>(m_selectors.at(i)))
                slider->setValue(parameter.defaultValue().toInt());
            break;
        case QVariant::Color:
            if (ColorPicker *picker = qobject_cast<ColorPicker *>(m_selectors.at(i)))
                picker->setColor(qvariant_cast<QColor>(parameter.defaultValue()));
            break;
        default:
            break;
        }
    }
}
