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

#ifndef EXAMPLESCREEN_H
#define EXAMPLESCREEN_H

#include <QLinuxFbScreen>

class ExampleScreen : public QLinuxFbScreen
{
public:
    ExampleScreen(int displayId);
    ~ExampleScreen();

    void exposeRegion(QRegion region, int windowIndex);

private:
    int fbd;
    QRect dirtyrect;
};

#endif
