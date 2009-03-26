/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>

#include <QtGui/QApplication>
#include <QtGui/QDesktopServices>

#include <QtHelp/QHelpEngineCore>

#include "mainwindow.h"
#include "cmdlineparser.h"

QT_USE_NAMESPACE

#if defined(USE_STATIC_SQLITE_PLUGIN)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(qsqlite)
#endif



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CmdLineParser cmd;
    CmdLineParser::Result res = cmd.parse(a.arguments());
    if (res == CmdLineParser::Help)
        return 0;
    else if (res == CmdLineParser::Error)
        return -1;

    if (cmd.registerRequest() != CmdLineParser::None) {
        QString colFile = cmd.collectionFile();
        if (colFile.isEmpty())
            colFile = MainWindow::defaultHelpCollectionFileName();
        QHelpEngineCore help(colFile);
        help.setupData();
        if (cmd.registerRequest() == CmdLineParser::Register) {
            if (!help.registerDocumentation(cmd.helpFile())) {
                cmd.showMessage(
                    QObject::tr("Could not register documentation file\n%1\n\nReason:\n%2")
                    .arg(cmd.helpFile()).arg(help.error()), true);
                return -1;
            } else {
                cmd.showMessage(QObject::tr("Documentation successfully registered."),
                    false);
            }
        } else {
            if (!help.unregisterDocumentation(QHelpEngineCore::namespaceName(cmd.helpFile()))) {
                cmd.showMessage(
                    QObject::tr("Could not unregister documentation file\n%1\n\nReason:\n%2")
                    .arg(cmd.helpFile()).arg(help.error()), true);
                return -1;
            } else {
                cmd.showMessage(QObject::tr("Documentation successfully unregistered."),
                    false);
            }
        }
        help.setCustomValue(QLatin1String("DocUpdate"), true);
        return 0;
    }

    if (!cmd.collectionFile().isEmpty()) {
        QHelpEngineCore he(cmd.collectionFile());
        if (!he.setupData()) {
            cmd.showMessage(QObject::tr("The specified collection file could not be read!"),
                true);
            return -1;
        }
        QString fileName = QFileInfo(cmd.collectionFile()).fileName();
        QString dir = MainWindow::collectionFileDirectory(false,
            he.customValue(QLatin1String("CacheDirectory"), QString()).toString());

        QFileInfo fi(dir + QDir::separator() + fileName);
        bool copyCollectionFile = false;
        if (!fi.exists()) {
            copyCollectionFile = true;
            if (!he.copyCollectionFile(fi.absoluteFilePath())) {
                cmd.showMessage(he.error(), true);                
                return -1;
            }
        }

        if (!copyCollectionFile) {
            QHelpEngineCore userHelpEngine(fi.absoluteFilePath());
            if (userHelpEngine.setupData()) {
                bool docUpdate = he.customValue(QLatin1String("DocUpdate"), false).toBool();
                if (userHelpEngine.customValue(QLatin1String("CreationTime"), 0).toUInt()
                    != he.customValue(QLatin1String("CreationTime"), 0).toUInt()) {
                    userHelpEngine.setCustomValue(QLatin1String("CreationTime"),
                        he.customValue(QLatin1String("CreationTime")));
                    userHelpEngine.setCustomValue(QLatin1String("WindowTitle"),
                        he.customValue(QLatin1String("WindowTitle")));
                    userHelpEngine.setCustomValue(QLatin1String("LastShownPages"),
                        he.customValue(QLatin1String("LastShownPages")));
                    userHelpEngine.setCustomValue(QLatin1String("CurrentFilter"),
                        he.customValue(QLatin1String("CurrentFilter")));
                    userHelpEngine.setCustomValue(QLatin1String("CacheDirectory"),
                        he.customValue(QLatin1String("CacheDirectory")));
                    userHelpEngine.setCustomValue(QLatin1String("EnableFilterFunctionality"),
                        he.customValue(QLatin1String("EnableFilterFunctionality")));
                    userHelpEngine.setCustomValue(QLatin1String("HideFilterFunctionality"),
                        he.customValue(QLatin1String("HideFilterFunctionality")));
                    userHelpEngine.setCustomValue(QLatin1String("EnableDocumentationManager"),
                        he.customValue(QLatin1String("EnableDocumentationManager")));
                    userHelpEngine.setCustomValue(QLatin1String("EnableAddressBar"),
                        he.customValue(QLatin1String("EnableAddressBar")));
                    userHelpEngine.setCustomValue(QLatin1String("HideAddressBar"),
                        he.customValue(QLatin1String("HideAddressBar")));
                    userHelpEngine.setCustomValue(QLatin1String("ApplicationIcon"),
                        he.customValue(QLatin1String("ApplicationIcon")));
                    userHelpEngine.setCustomValue(QLatin1String("AboutMenuTexts"),
                        he.customValue(QLatin1String("AboutMenuTexts")));
                    userHelpEngine.setCustomValue(QLatin1String("AboutIcon"),
                        he.customValue(QLatin1String("AboutIcon")));
                    userHelpEngine.setCustomValue(QLatin1String("AboutTexts"),
                        he.customValue(QLatin1String("AboutTexts")));
                    userHelpEngine.setCustomValue(QLatin1String("AboutImages"),
                        he.customValue(QLatin1String("AboutImages")));
                    docUpdate = true;
                }

                if (docUpdate) {
                    QStringList registeredDocs = he.registeredDocumentations();
                    QStringList userDocs = userHelpEngine.registeredDocumentations();
                    foreach (const QString &doc, registeredDocs) {
                        if (!userDocs.contains(doc))
                            userHelpEngine.registerDocumentation(he.documentationFileName(doc));
                    }
                    foreach (const QString &doc, userDocs) {
                        if (!registeredDocs.contains(doc)
                            && !doc.startsWith(QLatin1String("com.trolltech.com.assistantinternal_")))
                            userHelpEngine.unregisterDocumentation(doc);
                    }
                    he.removeCustomValue(QLatin1String("DocUpdate"));
                }
            }
        }
        cmd.setCollectionFile(fi.absoluteFilePath());
    }
    
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QTranslator translator(0);
    translator.load(QLatin1String("assistant_") + QLocale::system().name(), resourceDir);
    a.installTranslator(&translator);

    QTranslator qtTranslator(0);
    qtTranslator.load(QLatin1String("qt_") + QLocale::system().name(), resourceDir);
    a.installTranslator(&qtTranslator);

    QTranslator qtHelpTranslator(0);
    qtHelpTranslator.load(QLatin1String("qt_help_") + QLocale::system().name(), resourceDir);
    a.installTranslator(&qtHelpTranslator);

    MainWindow w(&cmd);
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
