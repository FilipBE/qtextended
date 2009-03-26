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

#include "adpreader.h"

QT_BEGIN_NAMESPACE

void AdpReader::readData(const QByteArray &contents)
{
    clear();
    m_contents.clear();
    m_keywords.clear();
    m_properties.clear();
    m_files.clear();
    addData(contents);
	while (!atEnd()) {
		readNext();
		if (isStartElement()) {
            if (name().toString().toLower() == QLatin1String("assistantconfig")
                && attributes().value(QLatin1String("version")) == QLatin1String("3.2.0")) {
                readProject();
            } else if (name().toString().toLower() == QLatin1String("dcf")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, 0));
                readDCF();
            } else {
                raiseError();
            }
		}
	}
}

QList<ContentItem> AdpReader::contents() const
{
    return m_contents;
}

QList<KeywordItem> AdpReader::keywords() const
{
    return m_keywords;
}

QSet<QString> AdpReader::files() const
{
    return m_files;
}

QMap<QString, QString> AdpReader::properties() const
{
    return m_properties;
}

void AdpReader::readProject()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            QString s = name().toString().toLower();
            if (s == QLatin1String("profile")) {
                readProfile();
            } else if (s == QLatin1String("dcf")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, 0));
                readDCF();
            } else {
                raiseError();
            }
        }
    }
}

void AdpReader::readProfile()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name().toString().toLower() == QLatin1String("property")) {
                QString prop = attributes().value(QLatin1String("name")).toString().toLower();
                m_properties[prop] = readElementText();
            } else {
                raiseError();
            }
        } else if (isEndElement()) {
            break;
        }
    }
}

void AdpReader::readDCF()
{
    int depth = 0;
    while (!atEnd()) {
        readNext();
        QString str = name().toString().toLower();
        if (isStartElement()) {
            if (str == QLatin1String("section")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, ++depth));
            } else if (str == QLatin1String("keyword")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_keywords.append(KeywordItem(readElementText(), ref));
            } else {
                raiseError();
            }
        } else if (isEndElement()) {
            if (str == QLatin1String("section"))
                --depth;
            else if (str == QLatin1String("dcf"))
                break;
        }
    }
}

void AdpReader::addFile(const QString &file)
{
    QString s = file;
    if (s.startsWith(QLatin1String("./")))
        s = s.mid(2);
    int i = s.indexOf(QLatin1Char('#'));
    if (i > -1)
        s = s.left(i);
    if (!m_files.contains(s))
        m_files.insert(s);
}

QT_END_NAMESPACE
