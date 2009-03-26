/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qplatformdefs.h"

#include <qfile.h>
#include "qlibrary_p.h"
#include <qfileinfo.h>
#include <qcoreapplication.h>

#ifndef QT_NO_LIBRARY

#ifdef Q_OS_MAC
#  include <private/qcore_mac_p.h>
#endif

#if defined(QT_AOUT_UNDERSCORE)
#include <string.h>
#endif

QT_BEGIN_NAMESPACE

#if defined(QT_HPUX_LD) // for HP-UX < 11.x and 32 bit

bool QLibraryPrivate::load_sys()
{
    pHnd = (void*)shl_load(QFile::encodeName(fileName), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
    if (pluginState != IsAPlugin) {
        if (!pHnd)
            pHnd = (void*)shl_load(QFile::encodeName(fileName + ".sl"), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
        if (!pHnd) {
            QFileInfo fi(fileName);
            int dlFlags = DYNAMIC_PATH | BIND_NONFATAL;
            if (loadHints & QLibrary::ResolveAllSymbolsHint) {
                dlFlags |= BIND_IMMEDIATE;
            } else {
                dlFlags |= BIND_DEFERRED;
            }
            pHnd = (void*)shl_load(QFile::encodeName(fi.path() + "/lib" + fi.fileName() + ".sl"),
                                   dlFlags, 0);
        }
    }
    if (!pHnd) {
        errorString = QLibrary::tr("QLibrary::load_sys: Cannot load %1 (%2)").arg(fileName).arg(QString());
    } else {
        errorString.clear();
    }
    return pHnd != 0;
}

bool QLibraryPrivate::unload_sys()
{
    if (shl_unload((shl_t)pHnd)) {
        errorString = QLibrary::tr("QLibrary::unload_sys: Cannot unload %1 (%2)").arg(fileName).arg(QString());
        return false;
    }
    errorString.clear();
    return true;
}

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
    void* address = 0;
    if (shl_findsym((shl_t*)&pHnd, symbol, TYPE_UNDEFINED, &address) < 0) {
        errorString = QLibrary::tr("QLibrary::resolve_sys: Symbol \"%1\" undefined in %2 (%3)").arg(
            QString::fromAscii(symbol)).arg(fileName).arg(QString());
        address = 0;
    } else {
        errorString.clear();
    }
    return address;
}

#else // POSIX
QT_BEGIN_INCLUDE_NAMESPACE
#include <dlfcn.h>
QT_END_INCLUDE_NAMESPACE

static QString qdlerror()
{
    const char *err = dlerror();
    return err ? QString::fromLocal8Bit(err) : QString();
}

bool QLibraryPrivate::load_sys()
{
    QFileInfo fi(fileName);
    QString path = fi.path();
    QString name = fi.fileName();
    if (path == QLatin1String(".") && !fileName.startsWith(path))
        path.clear();
    else
        path += QLatin1Char('/');

    // The first filename we want to attempt to load is the filename as the callee specified.
    // Thus, the first attempt we do must be with an empty prefix and empty suffix.
    QStringList suffixes(QLatin1String("")), prefixes(QLatin1String(""));
    if (pluginState != IsAPlugin) {
        prefixes << QLatin1String("lib");
#if defined(Q_OS_HPUX)
        if (!fullVersion.isEmpty()) {
            suffixes << QString::fromLatin1(".sl.%1").arg(fullVersion);
        } else {
            suffixes << QLatin1String(".sl");
        }
# if defined(__ia64)
        if (!fullVersion.isEmpty()) {
            suffixes << QString::fromLatin1(".so.%1").arg(fullVersion);
        } else {
            suffixes << QLatin1String(".so");
        }
# endif
#elif defined(Q_OS_AIX)
        suffixes << ".a";
#else
        if (!fullVersion.isEmpty()) {
            suffixes << QString::fromLatin1(".so.%1").arg(fullVersion);
        } else {
            suffixes << QLatin1String(".so");
        }
#endif
# ifdef Q_OS_MAC
        if (!fullVersion.isEmpty()) {
            suffixes << QString::fromLatin1(".%1.bundle").arg(fullVersion);
            suffixes << QString::fromLatin1(".%1.dylib").arg(fullVersion);
        } else {
            suffixes << QLatin1String(".bundle") << QLatin1String(".dylib");
        }
#endif
    }
    int dlFlags = 0;
    if (loadHints & QLibrary::ResolveAllSymbolsHint) {
        dlFlags |= RTLD_NOW;
    } else {
        dlFlags |= RTLD_LAZY;
    }
    if (loadHints & QLibrary::ExportExternalSymbolsHint) {
        dlFlags |= RTLD_GLOBAL;
    } 
#if !defined(Q_OS_CYGWIN)
    else {
#if defined(Q_OS_MAC)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
#endif        
        dlFlags |= RTLD_LOCAL;
    }
#endif
#if defined(Q_OS_AIX)	// Not sure if any other platform actually support this thing.
    if (loadHints & QLibrary::LoadArchiveMemberHint) {
        dlFlags |= RTLD_MEMBER;
    }
#endif
    QString attempt;
    bool retry = true;
    for(int prefix = 0; retry && !pHnd && prefix < prefixes.size(); prefix++) {
        for(int suffix = 0; retry && !pHnd && suffix < suffixes.size(); suffix++) {
            if (!prefixes.at(prefix).isEmpty() && name.startsWith(prefixes.at(prefix)))
                continue;
            if (!suffixes.at(suffix).isEmpty() && name.endsWith(suffixes.at(suffix)))
                continue;
            if (loadHints & QLibrary::LoadArchiveMemberHint) {
                attempt = name;
                int lparen = attempt.indexOf(QLatin1Char('('));
                if (lparen == -1)
                    lparen = attempt.count();
                attempt = path + prefixes.at(prefix) + attempt.insert(lparen, suffixes.at(suffix));
            } else {
                attempt = path + prefixes.at(prefix) + name + suffixes.at(suffix);
            }
            pHnd = dlopen(QFile::encodeName(attempt), dlFlags);
            if (!pHnd && fileName.startsWith(QLatin1Char('/')) && QFile::exists(attempt)) {
                // We only want to continue if dlopen failed due to that the shared library did not exist.
                // However, we are only able to apply this check for absolute filenames (since they are
                // not influenced by the content of LD_LIBRARY_PATH, /etc/ld.so.cache, DT_RPATH etc...)
                // This is all because dlerror is flawed and cannot tell us the reason why it failed.
                retry = false;
            }
        }
    }

#ifdef Q_OS_MAC
    if (!pHnd) {
        if (CFBundleRef bundle = CFBundleGetBundleWithIdentifier(QCFString(fileName))) {
            QCFType<CFURLRef> url = CFBundleCopyExecutableURL(bundle);
            QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            pHnd = dlopen(QFile::encodeName(str), dlFlags);
            attempt = str;
        }
    }
# endif
    if (!pHnd) {
        errorString = QLibrary::tr("QLibrary::load_sys: Cannot load %1 (%2)").arg(fileName).arg(qdlerror());
    }
    if (pHnd) {
        qualifiedFileName = attempt;
        errorString.clear();
    }
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (dlclose(pHnd)) {
        errorString = QLibrary::tr("QLibrary::unload_sys: Cannot unload %1 (%2)").arg(fileName).arg(qdlerror());
        return false;
    }
    errorString.clear();
    return true;
}

#ifdef Q_OS_MAC
Q_CORE_EXPORT void *qt_mac_resolve_sys(void *handle, const char *symbol)
{
    return dlsym(handle, symbol);
}
#endif

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
#if defined(QT_AOUT_UNDERSCORE)
    // older a.out systems add an underscore in front of symbols
    char* undrscr_symbol = new char[strlen(symbol)+2];
    undrscr_symbol[0] = '_';
    strcpy(undrscr_symbol+1, symbol);
    void* address = dlsym(pHnd, undrscr_symbol);
    delete [] undrscr_symbol;
#else
    void* address = dlsym(pHnd, symbol);
#endif
    if (!address) {
        errorString = QLibrary::tr("QLibrary::resolve_sys: Symbol \"%1\" undefined in %2 (%3)").arg(
            QString::fromAscii(symbol)).arg(fileName).arg(qdlerror());
    } else {
        errorString.clear();
    }
    return address;
}

#endif // POSIX

QT_END_NAMESPACE

#endif // QT_NO_LIBRARY
