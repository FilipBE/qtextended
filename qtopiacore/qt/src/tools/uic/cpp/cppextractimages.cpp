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

#include "cppextractimages.h"
#include "cppwriteicondata.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"

#include <QtCore/QDataStream>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

QT_BEGIN_NAMESPACE

namespace CPP {

ExtractImages::ExtractImages(const Option &opt)
    : m_output(0), m_option(opt)
{
}

void ExtractImages::acceptUI(DomUI *node)
{
    if (!m_option.extractImages)
        return;

    if (node->elementImages() == 0)
        return;

    QString className = node->elementClass() + m_option.postfix;

    QFile f;
    if (m_option.qrcOutputFile.size()) {
        f.setFileName(m_option.qrcOutputFile);
        if (!f.open(QIODevice::WriteOnly | QFile::Text)) {
            fprintf(stderr, "Could not create resource file\n");
            return;
        }

        QFileInfo fi(m_option.qrcOutputFile);
        QDir dir = fi.absoluteDir();
        if (!dir.exists(QLatin1String("images")) && !dir.mkdir(QLatin1String("images"))) {
            fprintf(stderr, "Could not create image dir\n");
            return;
        }
        dir.cd(QLatin1String("images"));
        m_imagesDir = dir;

        m_output = new QTextStream(&f);
        m_output->setCodec(QTextCodec::codecForName("UTF-8"));

        QTextStream &out = *m_output;

        out << "<RCC>\n";
        out << "    <qresource prefix=\"/" << className << "\" >\n";
        TreeWalker::acceptUI(node);
        out << "    </qresource>\n";
        out << "</RCC>\n";

        f.close();
        delete m_output;
        m_output = 0;
    }
}

void ExtractImages::acceptImages(DomImages *images)
{
    TreeWalker::acceptImages(images);
}

void ExtractImages::acceptImage(DomImage *image)
{
    QString format = image->elementData()->attributeFormat();
    QString extension = format.left(format.indexOf(QLatin1Char('.'))).toLower();
    QString fname = m_imagesDir.absoluteFilePath(image->attributeName() + QLatin1Char('.') + extension);

    *m_output << "        <file>images/" << image->attributeName() << QLatin1Char('.') + extension << "</file>\n";

    QFile f;
    f.setFileName(fname);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not create image file\n");
        return;
    }

    if (format == QLatin1String("XPM.GZ")) {
        QTextStream *imageOut = new QTextStream(&f);
        imageOut->setCodec(QTextCodec::codecForName("UTF-8"));

        CPP::WriteIconData::writeImage(*imageOut, QString(), image);
        delete imageOut;
    } else {
        CPP::WriteIconData::writeImage(f, image);
    }

    f.close();
}

} // namespace CPP

QT_END_NAMESPACE
