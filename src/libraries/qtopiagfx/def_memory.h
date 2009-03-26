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

#ifndef DEF_MEMORY_H
#define DEF_MEMORY_H

void def_memcpy(unsigned char *dest, unsigned char *src, int len);
void def_memmove(unsigned char *dest, unsigned char *src, int len);
void def_memset_8(unsigned char *dest, char c, int len);
void def_memset_16(unsigned short *dest, short c, int len);
void def_memset_32(unsigned int *dest, int c, int len);

#endif
