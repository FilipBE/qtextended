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
#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QToolButton>
#include <QFrame>
#include <QColor>

class QDialog;
class QModelIndex;

class ColorPicker : public QToolButton
{
    Q_OBJECT
public:
    ColorPicker(QWidget *parent = 0);

    QColor color() const;
    void setColor(const QColor &color);

private slots:
    void select();

private:
    QColor m_color;
};

#endif
