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

#ifndef MPICKER_H
#define MPICKER_H

#include <picker.h>
#include <qstringlist.h>

class InputMatcherSet;

class ModePicker : public Picker
{
    Q_OBJECT
public:
    ModePicker(InputMatcherSet *, QWidget *parent=0);
    ~ModePicker();

signals:
    void modeSelected(const QString &, bool);

protected slots:
    void setModeFor(int, int);

protected:
    void showEvent(QShowEvent *ev);
    void drawCell(QPainter *p, int, int, bool);

private:
    void updateModeList();

    QStringList list;
    InputMatcherSet *set;
};

#endif

