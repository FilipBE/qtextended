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

#include "dialerlineedit.h"
#include <QThemeItem>
#include <qsoftmenubar.h>
#include <QtopiaApplication>
#include <QLineEdit>
#include <QDebug>

/****************************************************************************/

DialerLineEditPlugin::DialerLineEditPlugin()
  : QThemeItemPlugin()
{
}

DialerLineEditPlugin::~DialerLineEditPlugin()
{
}

QStringList DialerLineEditPlugin::keys() const
{
    QStringList list;
    list << "dialer-lineedit-plugin";
    return list;
}

QThemeItem* DialerLineEditPlugin::create(const QString &type, QThemeItem *parent)
{
    Q_UNUSED(type);

    QThemeWidgetItem *item = new QThemeWidgetItem(parent);
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setWindowFlags(Qt::Window);
#ifdef QTOPIA_HOMEUI
    lineEdit->setFrame(false);
    lineEdit->setReadOnly(true);
#endif
    item->setWidget(lineEdit);
    return item;
}

QTOPIA_EXPORT_PLUGIN(DialerLineEditPlugin)
