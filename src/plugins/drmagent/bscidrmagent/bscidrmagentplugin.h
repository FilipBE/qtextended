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

#ifndef BSCIDRMAGENTPLUGIN_H
#define BSCIDRMAGENTPLUGIN_H

#include <qdrmcontentplugin.h>

class QTOPIA_PLUGIN_EXPORT BSciDrmAgentPlugin : public QObject, public QDrmAgentPlugin
{
    Q_OBJECT
    Q_INTERFACES(QDrmAgentPlugin)
public:
    BSciDrmAgentPlugin();
    virtual ~BSciDrmAgentPlugin();

    virtual QDrmContentPlugin *createDrmContentPlugin();
    virtual void createService( QObject *parent );
    virtual QList< QWidget * > createManagerWidgets( QWidget *parent = 0 );
};

#endif
