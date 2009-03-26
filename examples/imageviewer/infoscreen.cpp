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

#include "infoscreen.h"
#include "iviewer.h"
#include <QStringList>

InfoScreen::InfoScreen(IViewer *iviewer)
: QTextEdit(), _viewer(iviewer)
{
    setupUi();
    createActions();
    createMenu();
}

void InfoScreen::createActions()
{
}

void InfoScreen::createMenu()
{
}

void InfoScreen::setupUi() 
{
    setReadOnly(true);
}

void InfoScreen::setImage(const QContent &content) 
{
    QStringList keys;
    QStringList values;

    keys << "Name:"; 
    values << content.name();
    keys << "Type:";
    values << content.type();
    keys << "Size:";
    values << QString("%1kB").arg(content.size()/1024);
    keys << "Modified:";
    values << content.lastUpdated().toString(Qt::LocalDate);

    QString html = "<table>";
    for (int i=0; i<keys.count(); i++) {
        QString key = keys[i];
        QString value = values[i];
        html += QString("<tr><th>%1</th><td>%2</td></tr>").arg(key, value);
    }

    html += "</table>";
    document()->setHtml(html);
}
