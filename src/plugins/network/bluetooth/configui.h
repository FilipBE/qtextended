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

#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_advancedbtbase.h"

class AdvancedBTPage : public QWidget
{
    Q_OBJECT
public:
    AdvancedBTPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0,
            Qt::WFlags flags = 0 );

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop );

private:
    Ui::AdvancedBtBase ui;
};

#include "ui_dialingbtbase.h"

class DialingBTPage : public QWidget
{
    Q_OBJECT
public:
    DialingBTPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0,
            Qt::WFlags flags = 0 );

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop );

private:
    Ui::DialingBtBase ui;
};

#endif
