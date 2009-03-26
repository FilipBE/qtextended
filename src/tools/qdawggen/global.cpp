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



// simplified version of Global::tempDir(). This avoids having to include more
// source files

#include "global.h"

/*!
  \internal
  Returns the default system path for storing temporary files.
  Note: This does not ensure that the provided directory exists.
*/
namespace Qtopia
{
    QString tempDir()
    {
        QString result;
#ifdef Q_OS_UNIX
        result="/tmp/";
#else
        if (getenv("TEMP"))
            result = getenv("TEMP");
        else
            result = getenv("TMP");

        if (result[(int)result.length() - 1] != QDir::separator())
            result.append(QDir::separator());
#endif

        return result;
    }
}

