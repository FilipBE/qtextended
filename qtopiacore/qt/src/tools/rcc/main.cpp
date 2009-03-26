/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include <rcc.h>
#include "../../corelib/kernel/qcorecmdlineargs_p.h"

#include <QDebug>
#include <QDir>
#include <QFile>

QT_BEGIN_NAMESPACE

int showHelp(const QString &argv0, const QString &error)
{
    fprintf(stderr, "Qt resource compiler\n");
    if (!error.isEmpty())
        fprintf(stderr, "%s: %s\n", qPrintable(argv0), qPrintable(error));
    fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
            "Options:\n"
            "  -o file           write output to file rather than stdout\n"
            "  -name name        create an external initialization function with name\n"
            "  -threshold level  threshold to consider compressing files\n"
            "  -compress level   compress input files by level\n"
            "  -root path        prefix resource access path with root path\n"
            "  -no-compress      disable all compression\n"
            "  -binary           output a binary file for use as a dynamic resource\n"
            "  -namespace        turn off namespace macros\n"
            "  -version          display version\n"
            "  -help             display this information\n",
            qPrintable(argv0));
    return 1;
}


int runRcc(int argc, char *argv[])
{
    QString outFilename;
    bool helpRequested = false;
    bool list = false;
    QStringList files;

    QStringList args = qCmdLineArgs(argc, argv);

    RCCBuilder builder;

    //parse options
    QString errorMsg;
    for (int i = 1; i < args.count() && errorMsg.isEmpty(); i++) {
        if (args[i].isEmpty())
            continue;
        if (args[i][0] == QLatin1Char('-')) {   // option
            QString opt = args[i];
            if (opt == QLatin1String("-o")) {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing output name");
                    break;
                }
                outFilename = args[++i];
            } else if (opt == QLatin1String("-name")) {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing target name");
                    break;
                }
                builder.initName = args[++i];
            } else if (opt == QLatin1String("-root")) {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing root path");
                    break;
                }
                builder.resourceRoot = QDir::cleanPath(args[++i]);
                if (builder.resourceRoot.isEmpty()
                        || builder.resourceRoot.at(0) != QLatin1Char('/'))
                    errorMsg = QLatin1String("Root must start with a /");
            } else if (opt == QLatin1String("-compress")) {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing compression level");
                    break;
                }
                builder.compressLevel = args[++i].toInt();
            } else if (opt == QLatin1String("-threshold")) {
                if (!(i < argc-1)) {
                    errorMsg = QLatin1String("Missing compression threshold");
                    break;
                }
                builder.compressThreshold = args[++i].toInt();
            } else if (opt == QLatin1String("-binary")) {
                builder.writeBinary = true;
            } else if (opt == QLatin1String("-namespace")) {
                builder.useNameSpace = !builder.useNameSpace;
            } else if (opt == QLatin1String("-verbose")) {
                builder.verbose = true;
            } else if (opt == QLatin1String("-list")) {
                list = true;
            } else if (opt == QLatin1String("-version") || opt == QLatin1String("-v")) {
                fprintf(stderr, "Qt Resource Compiler version %s\n", QT_VERSION_STR);
                return 1;
            } else if (opt == QLatin1String("-help") || opt == QLatin1String("-h")) {
                helpRequested = true;
            } else if (opt == QLatin1String("-no-compress")) {
                builder.compressLevel = -2;
            } else {
                errorMsg = QString::fromLatin1("Unknown option: '%1'").arg(args[i]);
            }
        } else {
            if (!QFile::exists(args[i])) {
                qWarning("%s: File does not exist '%s'",
                    qPrintable(args[0]), qPrintable(args[i]));
                return 1;
            }
            files.append(args[i]);
        }
    }

    if (!files.size() || !errorMsg.isEmpty() || helpRequested)
        return showHelp(args[0], errorMsg);
    return int(!builder.processResourceFile(files, outFilename, list));
}

QT_END_NAMESPACE

int main(int argc, char *argv[])
{
    return QT_PREPEND_NAMESPACE(runRcc)(argc, argv);
}
