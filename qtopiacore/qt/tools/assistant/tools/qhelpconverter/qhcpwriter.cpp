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

#include <QtCore/QFile>

#include "qhcpwriter.h"

QT_BEGIN_NAMESPACE

QhcpWriter::QhcpWriter()
{
    setAutoFormatting(true);
}

void QhcpWriter::setHelpProjectFile(const QString &qhpFile)
{
    m_qhpFile = qhpFile;    
}

void QhcpWriter::setProperties(const QMap<QString, QString> props)
{
    m_properties = props;
}

void QhcpWriter::setTitlePath(const QString &path)
{
    m_titlePath = path;
}

bool QhcpWriter::writeFile(const QString &fileName)
{
    QFile out(fileName);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    
    setDevice(&out);
    writeStartDocument();
    writeStartElement(QLatin1String("QHelpCollectionProject"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    writeAssistantSettings();
    writeDocuments();
    writeEndDocument();
    return true;
}

void QhcpWriter::writeAssistantSettings()
{
    if (m_properties.isEmpty())
        return;

    writeStartElement(QLatin1String("assistant"));

    if (m_properties.contains(QLatin1String("title")))
        writeTextElement(QLatin1String("title"), m_properties.value(QLatin1String("title")));
    if (m_properties.contains(QLatin1String("applicationicon")))
        writeTextElement(QLatin1String("applicationIcon"),
            m_properties.value(QLatin1String("applicationicon")));
    if (m_properties.contains(QLatin1String("startpage")))
        writeTextElement(QLatin1String("startPage"), m_titlePath + QLatin1String("/")
            + m_properties.value(QLatin1String("startpage")));
    if (m_properties.contains(QLatin1String("aboutmenutext"))) {
        writeStartElement(QLatin1String("aboutMenuText"));
        writeTextElement(QLatin1String("text"),
            m_properties.value(QLatin1String("aboutmenutext")));
        writeEndElement();
    }
    if (m_properties.contains(QLatin1String("abouturl"))) {
        writeStartElement(QLatin1String("aboutDialog"));
        writeTextElement(QLatin1String("file"), m_properties.value(QLatin1String("abouturl")));
        writeEndElement();
    }
    if (m_properties.contains(QLatin1String("name"))) {
        writeTextElement(QLatin1String("cacheDirectory"),
            QLatin1String(".") + m_properties.value(QLatin1String("name")));
    }

    writeEndElement();
}

void QhcpWriter::writeDocuments()
{
    if (m_qhpFile.isEmpty())
        return;

    QString out = m_qhpFile;
    int i = out.indexOf(QLatin1Char('.'));
    if (i > -1)
        out = out.left(i);
    out.append(QLatin1String(".qch"));

    writeStartElement(QLatin1String("docFiles"));
    
    writeStartElement(QLatin1String("generate"));
    writeStartElement(QLatin1String("file"));
    writeTextElement(QLatin1String("input"), m_qhpFile);
    writeTextElement(QLatin1String("output"), out);
    writeEndElement();
    writeEndElement();

    writeStartElement(QLatin1String("register"));
    writeTextElement(QLatin1String("file"), out);
    writeEndElement();

    writeEndElement();
}

QT_END_NAMESPACE
