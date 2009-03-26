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

#ifndef THEMEDCONTEXTLABEL_H
#define THEMEDCONTEXTLABEL_H

#include "contextlabel.h"
#include <QDesktopWidget>

class QThemedView;
class QThemeItem;
class QThemeImageItem;
class QThemeTextItem;

class ThemedContextLabel : public BaseContextLabel
{
    Q_OBJECT
public:
    ThemedContextLabel(QWidget *parent = 0 , Qt::WFlags fl = 0 );
    ~ThemedContextLabel();
    virtual QSize reservedSize() const;

public slots:
    virtual void themeLoaded();

protected slots:
    void itemPressed(QThemeItem *item);
    void itemReleased(QThemeItem *item);
    void initializeButtons();
    void updateLabels();

protected:
    void moveEvent(QMoveEvent *e);

private:
    struct ThemedButton{
        QThemeImageItem *imgItem;
        QThemeTextItem *txtItem;
    };
    int buttonForItem(QThemeItem *item) const;
    QThemedView* phoneThemedView;
    bool themeInit;
    bool loadedTheme;
    ThemedButton *themedButtons;
};

#endif
