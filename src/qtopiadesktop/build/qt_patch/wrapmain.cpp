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

#include <qglobal.h>
#include <qplugin.h>
#undef Q_EXPORT_STATIC_PLUGIN
#undef Q_EXPORT_PLUGIN2
#define Q_EXPORT_STATIC_PLUGIN(x)\
    Q_DECL_EXPORT QObject *qt_plugin_instance_##x()\
    Q_PLUGIN_INSTANCE(x)\
    Q_IMPORT_PLUGIN(x)
#define Q_EXPORT_PLUGIN2(x,y)
#include "main.cpp"
