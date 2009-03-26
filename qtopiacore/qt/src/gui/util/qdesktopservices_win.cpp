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

#include <qsettings.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qprocess.h>
#include <qtemporaryfile.h>
#include <qcoreapplication.h>

#include <windows.h>
#include <shlobj.h>
#if !defined(Q_OS_WINCE)
#  include <intshcut.h>
#else
#  include <qguifunctions_wince.h>
#  if !defined(STANDARDSHELL_UI_MODEL)
#    include <winx.h>
#  endif
#endif

#ifndef QT_NO_DESKTOPSERVICES

QT_BEGIN_NAMESPACE

//#undef UNICODE

static bool openDocument(const QUrl &file)
{
    if (!file.isValid())
        return false;

    quintptr returnValue;
    QT_WA({
                returnValue = (quintptr)ShellExecute(0, 0, (TCHAR *)file.toString().utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                returnValue = (quintptr)ShellExecuteA(0, 0, file.toString().toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);
            });
    return (returnValue > 32); //ShellExecute returns a value greater than 32 if successful
}

static QString expandEnvStrings(const QString &command)
{

#if defined(Q_OS_WINCE)
    return command;
#else
    QByteArray path = command.toLocal8Bit();
    char commandValue[2 * MAX_PATH] = {0};
    DWORD returnValue = ExpandEnvironmentStringsA(path.data(), commandValue, MAX_PATH);
    if (returnValue)
        return QString::fromLocal8Bit(commandValue);
    else
        return command;
#endif
}

static bool launchWebBrowser(const QUrl &url)
{
    if (url.scheme() == QLatin1String("mailto")) {
        //Retrieve the commandline for the default mail client
        //the key used below is the command line for the mailto: shell command
        DWORD  bufferSize = 2 * MAX_PATH;
        long  returnValue =  -1;
        QString command;

        HKEY handle;
        LONG res;
        QT_WA ({
            res = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"mailto\\Shell\\Open\\Command", 0, KEY_READ, &handle);
            if (res != ERROR_SUCCESS)
                return false;

            wchar_t keyValue[2 * MAX_PATH] = {0};
            returnValue = RegQueryValueExW(handle, L"", 0, 0, reinterpret_cast<unsigned char*>(keyValue), &bufferSize);
            if (!returnValue)
                command = QString::fromRawData((QChar*)keyValue, bufferSize);
        }, {
            res = RegOpenKeyExA(HKEY_CLASSES_ROOT, "mailto\\Shell\\Open\\Command", 0, KEY_READ, &handle);
            if (res != ERROR_SUCCESS)
                return false;

            char keyValue[2 * MAX_PATH] = {0};
            returnValue = RegQueryValueExA(handle, "", 0, 0, reinterpret_cast<unsigned char*>(keyValue), &bufferSize);
            if (!returnValue)
                command = QString::fromLocal8Bit(keyValue);
        });
        RegCloseKey(handle);

        if(returnValue)
            return false;
        command = expandEnvStrings(command);
        command = command.trimmed();
        //Make sure the path for the process is in quotes
        int index = -1 ;
        if (command[0]!= QLatin1Char('\"')) {
            index = command.indexOf(QLatin1String(".exe "), 0, Qt::CaseInsensitive);
            command.insert(index+4, QLatin1Char('\"'));
            command.insert(0, QLatin1Char('\"'));
        }
        //pass the url as the parameter
        index =  command.lastIndexOf(QLatin1String("%1"));
        if (index != -1){
            command.replace(index, 2, url.toString());
        }
        //start the process
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        QT_WA ({
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            returnValue = CreateProcess(NULL, (TCHAR*)command.utf16(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        }, {
            STARTUPINFOA si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            returnValue = CreateProcessA(NULL, command.toLocal8Bit().data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        });

        if (!returnValue)
            return false;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    if (!url.isValid())
        return false;

    quintptr returnValue;
     QT_WA ({
         returnValue = (quintptr)ShellExecute(0, 0, (TCHAR *) QString::fromUtf8(url.toEncoded().constData()).utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                returnValue = (quintptr)ShellExecuteA(0, 0, url.toEncoded().constData(), 0, 0, SW_SHOWNORMAL);
            });
    return (returnValue > 32);
}

QString QDesktopServices::storageLocation(StandardLocation type)
{
    QSettings settings(QSettings::UserScope, QLatin1String("Microsoft"), QLatin1String("Windows"));
    settings.beginGroup(QLatin1String("CurrentVersion/Explorer/Shell Folders"));
    switch (type) {
    case DataLocation:
        if (!settings.contains(QLatin1String("Local AppData")))
            break;
        return settings.value(QLatin1String("Local AppData")).toString()
            + QLatin1String("\\") + QCoreApplication::organizationName()
            + QLatin1String("\\") + QCoreApplication::applicationName();
        break;
    case DesktopLocation:
        return settings.value(QLatin1String("Desktop")).toString();
        break;

    case DocumentsLocation:
        return settings.value(QLatin1String("Personal")).toString();
        break;

    case FontsLocation:
        return settings.value(QLatin1String("Fonts")).toString();
        break;

    case ApplicationsLocation:
        return settings.value(QLatin1String("Programs")).toString();
        break;

    case MusicLocation:
        return settings.value(QLatin1String("My Music")).toString();
        break;

    case MoviesLocation:
        return settings.value(QLatin1String("My Video")).toString();
        break;

    case PicturesLocation:
        return settings.value(QLatin1String("My Pictures")).toString();
        break;

    case QDesktopServices::HomeLocation:
        return QDir::homePath(); break;

    case QDesktopServices::TempLocation:
        return QDir::tempPath(); break;

    default:
        break;
    }

    return QString();
}

QString QDesktopServices::displayName(StandardLocation type)
{
    Q_UNUSED(type);
    return QString();
}

QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES
