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

#ifndef CHARLIST_H
#define CHARLIST_H

#include <qwidget.h>
#include <qbitmap.h>

class QFontMetrics;

class CharList : public QWidget
{
    Q_OBJECT
public:
    CharList(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
    ~CharList();

    void setMicroFocus( int x, int y );
    void setAppFont(const QFont &f) { appFont = f; }
    void setChars(const QStringList &ch);
    void setCurrent(const QString &ch);

protected:
    void paintEvent(QPaintEvent*);

private:
    int cellHeight;
    int cellWidth;
    QFont appFont;
    QFontMetrics *fm;
    QStringList chars;
    QString current;
};

#endif

