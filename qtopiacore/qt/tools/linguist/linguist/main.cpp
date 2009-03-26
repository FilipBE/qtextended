/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "trwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QTextCodec>
#include <QTranslator>
#include <QSettings>
#include <QSplashScreen>
#include <QLibraryInfo>
#include <QLocale>
#include <QFile>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(linguist);

    QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QStringList files;
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QStringList args = app.arguments();

    for (int i = 1; i < args.count(); ++i)
    {
        QString argument = args.at(i);
        if (argument == QLatin1String("-resourcedir")) {
            if (i + 1 < args.count()) {
                resourceDir = QFile::decodeName(args.at(++i).toLocal8Bit());
            } else {
                // issue a warning
            }
        } else if (!files.contains(argument)) {
            files.append(argument);
        }
    }

    QTranslator translator(0);
    translator.load(QLatin1String("linguist_") + QLocale::system().name(), resourceDir);
    app.installTranslator(&translator);

    QTranslator qtTranslator(0);
    qtTranslator.load(QLatin1String("qt_") + QLocale::system().name(), resourceDir);
    app.installTranslator(&qtTranslator);

    app.setOrganizationName(QLatin1String("Trolltech"));
    app.setApplicationName(QLatin1String("Linguist"));
    QString keybase(QString::number( (QT_VERSION >> 16) & 0xff ) +
                     QLatin1Char('.') + QString::number( (QT_VERSION >> 8) & 0xff ) + QLatin1Char('/') );
    QSettings config;

    QWidget tmp;
    tmp.restoreGeometry(config.value(keybase + QLatin1String("Geometry/WindowGeometry")).toByteArray());

    QSplashScreen *splash = 0;
    int screenId = QApplication::desktop()->screenNumber(tmp.geometry().center());
    splash = new QSplashScreen(QApplication::desktop()->screen(screenId),
        QPixmap(QLatin1String(":/images/splash.png")));
    if (QApplication::desktop()->isVirtualDesktop()) {
        QRect srect(0, 0, splash->width(), splash->height());
        splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center() );
    }
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    TrWindow tw;
    tw.show();

    splash->finish(&tw);

    if (files.count())
        tw.openFile(files.last());

    QApplication::restoreOverrideCursor();

    return app.exec();
}
