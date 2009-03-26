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

#include <stdio.h>
#include <stdlib.h>
#include <QImage>
#include <QString>
#include <QPicture>
#include <QSvgRenderer>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include <qglobal.h>

#include <math.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s input output [input output] ... [input output]\n", name);
    fprintf(stderr, "  convert input SVG to output QPicture\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv, false);

    int i;
    for (i = 1; i < argc; i++) {
        QString arg(argv[i]);
        if (arg[0] == '-') {
            usage(argv[0]);
        } else {
            break;
        }
    }

    if ( argc - i < 2 || (argc - i) % 2 == 1 )
        usage(argv[0]);
    for ( ; i < argc; i += 2 ) {
        QString input(argv[i]);
        QString output(argv[i+1]);

        QSvgRenderer renderer;
        if (renderer.load(input)) {
            QPicture picture;
            QSizeF sizef = renderer.viewBoxF().size();
            QSize size((int)ceil(sizef.width()), (int)ceil(sizef.height()));
            picture.setBoundingRect(QRect(QPoint(0,0), size));
            QPainter painter(&picture);
            renderer.render(&painter);
            painter.end();
            if (!picture.save(output)) {
                fprintf(stderr, "Cannot save %s\n", output.toLatin1().data());
                return -1;
            }
        } else {
            fprintf(stderr, "Cannot load %s\n", input.toLatin1().data());
            return -1;
        }
    }

    return 0;
}

