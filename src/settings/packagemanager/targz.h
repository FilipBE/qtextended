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
#ifndef TARGZ_H
#define TARGZ_H

#include <QString>
#include <libtar.h>

bool targz_extract_all( const QString &tarfile, const QString &destpath = QString(), bool verbose = true );
bool targz_archive_all( const QString &tarfile, const QString &srcpath, bool gzip = true, bool verbose = true );

TAR * get_tar_ptr( const QString &tarfile );
qulonglong targz_archive_size( const QString &tarfile );
bool check_tar_valid( const QString &tarfile );
#endif
