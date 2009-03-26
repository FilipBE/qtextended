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

#include "ui3reader.h"

#include <QDomElement>
#include <QFile>

QT_BEGIN_NAMESPACE

void Ui3Reader::computeDeps(const QDomElement &e,
        QStringList &globalIncludes,
        QStringList &localIncludes, bool impl)
{
    QDomNodeList nl;

    // additional includes (local or global) and forward declaractions
    nl = e.toElement().elementsByTagName(QLatin1String("include"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();

        if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
            continue;

        if (impl && n2.attribute(QLatin1String("impldecl"), QLatin1String("in implementation")) != QLatin1String("in implementation"))
            continue;

        if (n2.attribute(QLatin1String("location")) != QLatin1String("local"))
            globalIncludes += s;
        else
            localIncludes += s;
    }

    // do the local includes afterwards, since global includes have priority on clashes
    nl = e.toElement().elementsByTagName(QLatin1String("header"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();
        if (n2.attribute(QLatin1String("location")) == QLatin1String("local") && !globalIncludes.contains(s)) {
            if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
                continue;

            if (impl && n2.attribute(QLatin1String("impldecl"), QLatin1String("in implementation")) != QLatin1String("in implementation"))
                continue;

            localIncludes += s;
        }
    }

    // additional custom widget headers
    nl = e.toElement().elementsByTagName(QLatin1String("header"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();

        if (n2.attribute(QLatin1String("location")) != QLatin1String("local"))
            globalIncludes += s;
        else
            localIncludes += s;
    }

    { // fix globalIncludes
        globalIncludes = unique(globalIncludes);
        QMutableStringListIterator it(globalIncludes);
        while (it.hasNext()) {
            QString v = it.next();

            if (v.isEmpty()) {
                it.remove();
                continue;
            }

            it.setValue(fixHeaderName(v));
        }
    }

    { // fix the localIncludes
        localIncludes = unique(localIncludes);
        QMutableStringListIterator it(localIncludes);
        while (it.hasNext()) {
            QString v = it.next();

            if (v.isEmpty()) {
                it.remove();
                continue;
            }

            it.setValue(fixHeaderName(v));
        }
    }
}

QT_END_NAMESPACE
