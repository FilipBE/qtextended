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

#ifndef WPICKER_H
#define WPICKER_H

#include "picker.h"
#include <qstring.h>
#include <qstringlist.h>

class QTOPIA_INPUTMATCH_EXPORT WordPicker : public Picker
{
    Q_OBJECT
public:
    WordPicker(QWidget *parent=0);
    ~WordPicker();

    void setChoices(const QStringList &);

signals:
    void wordChosen(const QString &);

protected:
    void drawCell(QPainter *p, int, int, bool);

private slots:
    void sendWordChoice(int, int);

private:
    QStringList choices;
};

#endif

