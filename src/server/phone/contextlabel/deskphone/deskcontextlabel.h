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

#ifndef DESKCONTEXTLABEL_H
#define DESKCONTEXTLABEL_H

#include "serverthemeview.h"

#include "qtopiainputevents.h"
#include "qsoftmenubarprovider.h"
#include "qabstractcontextlabel.h"

class DeskphoneContextLabelPrivate;
class QThemeItem;

class DeskphoneContextLabel : public QAbstractContextLabel, public QtopiaKeyboardFilter
{
    Q_OBJECT
public:
    DeskphoneContextLabel(QWidget *parent, Qt::WFlags flags);
    ~DeskphoneContextLabel();

    QSize reservedSize() const;

private slots:
    void initializeButtons();
    void itemPressed(QThemeItem *item);
    void itemReleased(QThemeItem *item);
    void keyChanged(const QSoftMenuBarProvider::MenuButton &button);
    virtual void themeLoaded();
    void updateLabels();

protected:
    virtual bool filter(int unicode, int keycode, int modifiers, bool press,
                        bool autoRepeat);

private:
    DeskphoneContextLabelPrivate *d;
};

#endif
