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

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QMap>

#include <stdlib.h>

#include <qdawg.h>

static void usage( const char *progname )
{
    // See also doc/html/qdawggen.html

    printf("%s converts word list files into the qdawg format\n", progname);
    printf("Usage:\n");
    printf("\t%s "
        "[-v] [-d] <output directory> <input file> [input file...]\n", progname);
    printf("\t  [-v]  Verify result\n");
    printf("\t  [-e]  Reverse endianness of resulting file\n");
    printf("\t  [-d]  Dump result\n");
    exit(-1);
}



int main(int argc, char **argv)
{
    QFile wordlistFile;
    QFile e(argv[0]);
    bool error=false,dump=false;
    bool swapbytes=false;
    int verify=0;
    if (argc < 3)
        error=true;
    // Parse the command-line.
    QString path( argv[1] );

    while ( path[0] == '-' ) {
        if ( path == "-v" ) ++verify;
        else if ( path == "-e" ) swapbytes=!swapbytes;
        else if ( path == "-d" ) dump=true;
        else error=true;
        --argc; ++argv; path=argv[1];
    }

    if (error)
        usage(e.fileName().toLocal8Bit().constData());

    QDir d;
    d.setPath(path);
    if ( !d.exists() ) {
        bool ok = d.mkpath( path );
        if ( !ok ) {
            printf( "Could not create directory %s\n", path.toLocal8Bit().constData() );
            return 1;
        }
    }

    for (int index = 2; index < argc; index++) {
        QFile qdawgFile;
        QFile wordlistFile;

        wordlistFile.setFileName(QString(argv[index]));
        qdawgFile.setFileName(QString("%1/%2.dawg").arg(path).arg(QFileInfo(wordlistFile).fileName()));

        QDawg dawg;
        if (!wordlistFile.open(QIODevice::ReadOnly)) {
            printf("Cannot open %s\n", wordlistFile.fileName().toLocal8Bit().constData());
            continue;
        }
        if (!qdawgFile.open(QIODevice::WriteOnly)) {
            printf("Cannot write %s\n", qdawgFile.fileName().toLocal8Bit().constData());
            continue;
        }

        dawg.createFromWords(&wordlistFile);
        if ( swapbytes )
            dawg.writeByteSwapped(&qdawgFile);
        else
            dawg.write(&qdawgFile);
        if ( dump )
            dawg.dump();

        if ( verify ) {
            QMap<QString,int> trieval;
            QStringList wrote = dawg.allWords();
            QStringList read;
            wordlistFile.reset();
            QTextStream ts(&wordlistFile);
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                QStringList t = line.split(' ');
                read += t[0];
                trieval[line]=t[1].toUInt();
            }
            if ( wrote == read ) {
                printf("  verified\n");
            } else {
                printf("  error in verification (are words sorted?)\n");
                if ( verify > 1 ) {
                    // XXX ... debug
                }
            }
        }
    }

    return 0;
}

