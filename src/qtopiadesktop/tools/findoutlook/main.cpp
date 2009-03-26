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
#include <qdwin32>
using namespace QDWIN32;

#include <QString>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

int main( int /*argc*/, char **argv )
{
    QString mso;
    QString msoutl;

    {   // Use mso.dll and msoutl.olb from src/qtopiadesktop/dist if they exist
        // This is useful when you want to target a version of Outlook older
        // than what you have installed.
        QString dist = QDir::cleanPath( QFileInfo(argv[0]).absoluteDir().absoluteFilePath("../src/qtopiadesktop/dist") );
        QString _mso = QString("%1/mso.dll").arg(dist);
        //qDebug() << "mso" << _mso;
        if ( QFile::exists(_mso) )
            mso = QDir::toNativeSeparators( _mso );
        QString _msoutl = QString("%1/msoutl.olb").arg(dist);
        if ( QFile::exists(_msoutl) )
            msoutl = QDir::toNativeSeparators( _msoutl );
    }

    if ( mso.isEmpty() ) {
        QString clsid = readRegKey( HKEY_CLASSES_ROOT, "OfficeCompatible.Application\\CLSID" );
        //qDebug() << "CLSID" << clsid;
        if ( clsid.isEmpty() )
            return 1;

        mso = readRegKey( HKEY_CLASSES_ROOT, QString("CLSID\\%1\\InprocServer32").arg(clsid) );
        //qDebug() << "mso" << mso;
    }

    if ( msoutl.isEmpty() ) {
        QString clsid = readRegKey( HKEY_CLASSES_ROOT, "Outlook.Application\\CLSID" );
        //qDebug() << "CLSID" << clsid;
        if ( clsid.isEmpty() )
            return 1;

        QString typelib = readRegKey( HKEY_CLASSES_ROOT, QString("CLSID\\%1\\Typelib").arg(clsid) );
        //qDebug() << "typelib" << typelib;
        if ( typelib.isEmpty() )
            return 1;

        QString sub = findSubKey( HKEY_CLASSES_ROOT, QString("Typelib\\%1").arg(typelib) );
        //qDebug() << "sub" << sub;
        if ( sub.isEmpty() )
            return 1;

        msoutl = readRegKey( HKEY_CLASSES_ROOT, QString("Typelib\\%1\\%2\\0\\win32").arg(typelib).arg(sub) );
        //qDebug() << "msoutl" << msoutl;
    }

    if ( mso.isEmpty() )
        qDebug() << "Could not find mso.dll";

    if ( msoutl.isEmpty() )
        qDebug() << "Could not find msoutl.olb";

    if ( mso.isEmpty() || msoutl.isEmpty() ) {
        qDebug() << "Do you have Outlook installed?";
        return 1;
    }

    {
        QFile f( "qoutlook_detect.h" );
        f.open( QIODevice::WriteOnly );
        QTextStream stream( &f );
        stream << "#import \"" << mso << "\" \\" << endl
               << "    no_namespace, no_implementation, rename(\"DocumentProperties\", \"DocProps\")" << endl
               << "#import \"" << msoutl << "\" \\" << endl
               << "    rename_namespace(\"Outlook\")" << endl;
        f.close();
    }

    {
        QFile f( "qoutlook_detect.cpp" );
        f.open( QIODevice::WriteOnly );
        QTextStream stream( &f );
        stream << "#import \"" << mso << "\" \\" << endl
               << "    no_namespace, implementation_only, rename(\"DocumentProperties\", \"DocProps\")" << endl;
               //<< "#import \"" << msoutl << "\" \\" << endl
               //<< "    rename_namespace(\"Outlook\")" << endl;
        f.close();
    }

    return 0;
}

