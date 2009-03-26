/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QT_NO_DESKTOPSERVICES

#include <qprocess.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
#include <private/qcore_mac_p.h>
#include <qcoreapplication.h>

QT_BEGIN_NAMESPACE

/*
    Translates a QDesktopServices::StandardLocation into the mac equivalent.
*/
OSType translateLocation(QDesktopServices::StandardLocation type)
{
    switch (type) {
    case QDesktopServices::DesktopLocation:
        return kDesktopFolderType; break;

    case QDesktopServices::DocumentsLocation:
        return kDocumentsFolderType; break;

    case QDesktopServices::FontsLocation:
        // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
        // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
        return kFontsFolderType; break;

    case QDesktopServices::ApplicationsLocation:
        return kApplicationsFolderType; break;

    case QDesktopServices::MusicLocation:
        return kMusicDocumentsFolderType; break;

    case QDesktopServices::MoviesLocation:
        return kMovieDocumentsFolderType; break;

    case QDesktopServices::PicturesLocation:
        return kPictureDocumentsFolderType; break;

    case QDesktopServices::TempLocation:
        return kTemporaryFolderType; break;

    case QDesktopServices::DataLocation:
        return kApplicationSupportFolderType; break;
    default:
        return kDesktopFolderType; break;
    }
}

static bool lsOpen(const QUrl &url)
{
    if (!url.isValid())
        return false;

    QCFType<CFURLRef> cfUrl = CFURLCreateWithString(0, QCFString(QString::fromLatin1(url.toEncoded())), 0);
    if (cfUrl == 0)
        return false;

    const OSStatus err = LSOpenCFURLRef(cfUrl, 0);
    return (err == noErr);
}

static bool launchWebBrowser(const QUrl &url)
{
    return lsOpen(url);
}

static bool openDocument(const QUrl &file)
{
    if (!file.isValid())
        return false;

   // LSOpen does not work in this case, use QProcess open instead.
   return QProcess::startDetached(QLatin1String("open"), QStringList() << file.toLocalFile());
}

/*
    Returns a QString given an HFSUniStr255.
*/
static QString qt_mac_hfsunistr_to_qstring(const HFSUniStr255 &hfs)
{
    QCFString str = CFStringCreateWithCharacters( kCFAllocatorDefault, hfs.unicode, hfs.length ); 
    return str;
}

/*
    Returns wether the given fsRef is something valid.
*/
static Boolean FSRefIsValid(const FSRef &fsRef)
{
  return (FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, NULL, NULL) == noErr);
}

/*
    Constructs a full unicode path from a FSRef (I can't find a system function
    that does this.. )
*/
static QString getFullPath(FSRef ref)
{
    QString path;
    FSRef parent;
    do {
        HFSUniStr255 name;
        OSErr err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, &name, NULL, &parent);
        if (err)
            return QString();

        if (FSRefIsValid(parent))
            path.prepend(qt_mac_hfsunistr_to_qstring(name) + QLatin1Char('/'));
        ref = parent;
    } while (FSRefIsValid(ref));

    path.prepend(QLatin1String("/"));
    if (QFile::exists(path) == false)
        path.prepend(QLatin1String("/Volumes"));

    return path;
}

QString QDesktopServices::storageLocation(StandardLocation type)
{
     if (QDesktopServices::HomeLocation == type)
        return QDir::homePath();

    short domain = kOnAppropriateDisk;

    if (QDesktopServices::DataLocation == type)
        domain = kUserDomain;

     // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html
     FSRef ref;
     OSErr err = FSFindFolder(domain, translateLocation(type), false, &ref);
     if (err)
        return QString();

    QString path = getFullPath(ref);
   
    if (QDesktopServices::DataLocation == type)
        path += QCoreApplication::applicationName();

    return path;
}

QString QDesktopServices::displayName(StandardLocation type)
{
    if (QDesktopServices::HomeLocation == type)
        return QObject::tr("Home");

    FSRef ref;
    OSErr err = FSFindFolder(kOnAppropriateDisk, translateLocation(type), false, &ref);
    if (err)
        return QString();

    QCFString displayName;
    err = LSCopyDisplayNameForRef(&ref, &displayName);
    if (err)
        return QString();

    return static_cast<QString>(displayName);
}

QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES
