/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** WARNING: Use of this file may require additional third party patent licensing.
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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "picker.h"
#include <qbitmap.h>

class QTOPIA_INPUTMATCH_EXPORT SymbolPicker : public Picker
{
    Q_OBJECT
public:
    SymbolPicker(QWidget *parent=0);
    ~SymbolPicker();

signals:
    void symbolClicked(int unicode, int keycode);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendSymbolForPos(int, int);

private:
    QString chars;
    int mapRows;
    bool havePress;
    QFont appFont;
    QBitmap crBitmap;
    QChar **symbols;
};

#endif

