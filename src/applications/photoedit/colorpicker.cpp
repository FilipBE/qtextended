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
#include "colorpicker.h"

#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionButton>
#include <QColorSelectorDialog>

ColorPicker::ColorPicker(QWidget *parent)
    : QToolButton(parent)
    , m_color(palette().color(QPalette::Button))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::PushButton));

    connect(this, SIGNAL(clicked()), this, SLOT(select()));
}

QColor ColorPicker::color() const
{
    return m_color;
}

void ColorPicker::setColor(const QColor &color)
{
    m_color = color;

    QPalette pal = palette();
    pal.setColor(QPalette::Button, m_color);
    setPalette(pal);
}
void ColorPicker::select()
{
    setColor(QColorSelectorDialog::getColor(m_color, this));
}
