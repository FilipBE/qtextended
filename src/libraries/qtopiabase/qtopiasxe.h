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

#ifndef QTOPIASXE_H
#define QTOPIASXE_H

#include <qtopiaglobal.h>

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER) || defined (Q_QDOC)

#include <QFile>

#include <qtransportauthdefs_qws.h>

#include <qtopiaglobal.h>
#include <QDebug>

class SxeProgramInfoPrivate;

struct QTOPIABASE_EXPORT SxeProgramInfo
{
    SxeProgramInfo();
    ~SxeProgramInfo();
    QString fileName;   // eg calculator, bomber
    QString relPath;    // eg bin, packages/bin
    QString runRoot;    // eg /opt/Qtopia.rom, /opt/Qtopia.user
    QString installRoot; // eg $HOME/build/qtopia/42-phone/image
    QString domain;     // security domains, csv
    unsigned char id;   // program identity
    char key[QSXE_KEY_LEN]; // key

    bool isValid() const;
    QString absolutePath() const;
    bool locateBinary();
    void suid();
private:
    SxeProgramInfoPrivate *d;
};

QTOPIABASE_EXPORT QDebug operator<<(QDebug debug, const SxeProgramInfo &progInfo);

#endif  // QT_NO_SXE

#ifdef SINGLE_EXEC
#define QSXE_KEY_TEMPLATE
#define QSXE_APP_KEY ;
#define QSXE_QL_APP_KEY ;
#define QSXE_SET_QL_KEY(APPNAME) ;
#define QSXE_SET_APP_KEY(APPNAME) ;
#else
QTOPIABASE_EXPORT void checkAndSetProcessKey( const char *key, const char *app );
#define QSXE_KEY_TEMPLATE "XOXOXOauthOXOXOX99"
#define QSXE_APP_KEY char _key[] = QSXE_KEY_TEMPLATE;
#define QSXE_QL_APP_KEY char _ql_key[] = QSXE_KEY_TEMPLATE;
#define QSXE_SET_QL_KEY(APPNAME) checkAndSetProcessKey( _ql_key, APPNAME );
#define QSXE_SET_APP_KEY(APPNAME) checkAndSetProcessKey( _key, APPNAME );
#endif

#endif
