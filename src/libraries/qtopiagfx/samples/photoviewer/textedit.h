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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QObject>
#include <GfxClock>
#include "gfxcanvas.h"

class TextEdit : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    TextEdit(GfxCanvasItem *parent);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

    QString text() const;
    void confirmText();
signals:
    void tentativeTextChanged(const QString &);
    void textChanged(const QString &);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void newText();
    GfxCanvasText *_text;
    QString str;
    GfxTimeLine tl;
    int _timer;

    char lastKey;
    int lastKeyOp;

    QList<int> widths;
    GfxCanvasColor line;
};

#endif
