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

#ifndef QDAUTOPLUGIN_H
#define QDAUTOPLUGIN_H

#define QD_REGISTER_PLUGIN(IMPLEMENTATION) \
    void qd_registerPlugin_AUTOPLUGIN_TARGET(const QString &id, qdPluginCreateFunc_t createFunc);\
    static QDPlugin *create_ ## IMPLEMENTATION( QObject *parent ) \
        { return new IMPLEMENTATION(parent); } \
    static qdPluginCreateFunc_t append_ ## IMPLEMENTATION() \
        { qd_registerPlugin_AUTOPLUGIN_TARGET(#IMPLEMENTATION, create_ ## IMPLEMENTATION); \
            return create_ ## IMPLEMENTATION; } \
    static qdPluginCreateFunc_t dummy_ ## IMPLEMENTATION = \
        append_ ## IMPLEMENTATION();

#endif
