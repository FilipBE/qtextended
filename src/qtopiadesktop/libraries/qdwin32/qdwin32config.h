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
#ifndef QDWIN32CONFIG_H
#define QDWIN32CONFIG_H

#include <qglobal.h>
#include <qplugin.h>

// The _EXPORT macros...

#ifdef Q_OS_WIN32
# if defined(QDWIN32_MAKEDLL)
#  define QDWIN32_EXPORT __declspec(dllexport)
# elif defined(QDWIN32_DLL)
#  define QDWIN32_EXPORT __declspec(dllimport)
# endif
#endif
#ifndef QDWIN32_EXPORT
#define QDWIN32_EXPORT
#endif

#endif
