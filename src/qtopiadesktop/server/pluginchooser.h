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
#ifndef PLUGINCHOOSER_H
#define PLUGINCHOOSER_H

#include <qdplugindefs.h>

#include <QFrame>

class PluginChooserPrivate;

class QMenu;

class PluginChooser : public QFrame
{
    Q_OBJECT
public:
    PluginChooser( QWidget *parent = 0 );
    virtual ~PluginChooser();

    enum Orientation { Vertical, Horizontal };
    void setOrientation( Orientation orientation );

    enum WidgetType { App, Settings };
    void setWidgetType( WidgetType widgetType );

    void setWindowMenu( QMenu *windowMenu );

public slots:
    void highlightPlugin( QDAppPlugin *plugin );

signals:
    void showPlugin( QDAppPlugin *plugin );

private slots:
    void pluginsChanged();

private:
    PluginChooserPrivate *d;
};

#endif
