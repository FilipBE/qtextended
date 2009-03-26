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

#ifndef DCFSECTION_H
#define DCFSECTION_H

#include <qlist.h>
#include <qpair.h>
#include <qstring.h>

QT_BEGIN_NAMESPACE

class QTextStream;

struct DcfSection
{
    QString title;
    QString ref;
    QList<QPair<QString, QString> > keywords;
    QList<DcfSection> subsections;
};

inline bool operator<( const DcfSection& s1, const DcfSection& s2 ) {
    QString title1 = s1.title;
    QString title2 = s2.title;

    // cheat with Q3 classes
    if (title1.startsWith("Q3"))
        title1.insert(1, '~');
    if (title2.startsWith("Q3"))
        title2.insert(1, '~');

    int delta = title1.toLower().compare( title2.toLower() );
    if ( delta == 0 ) {
	delta = title1.compare( title2 );
	if ( delta == 0 )
	    delta = s1.ref.localeAwareCompare( s2.ref );
    }
    return delta < 0;
}

inline bool operator>( const DcfSection& s1, const DcfSection& s2 ) { return s2 < s1; }
inline bool operator<=( const DcfSection& s1, const DcfSection& s2 ) { return !( s2 < s1 ); }
inline bool operator>=( const DcfSection& s1, const DcfSection& s2 ) { return !( s1 < s2 ); }
inline bool operator==( const DcfSection& s1, const DcfSection& s2 ) { return &s1 == &s2; }
inline bool operator!=( const DcfSection& s1, const DcfSection& s2 ) { return !( s1 == s2 ); }

void appendDcfSubSection(DcfSection *dcfSect, const DcfSection &sub);
void appendDcfSubSections(DcfSection *dcfSect, const QList<DcfSection> &subs);
void generateDcfSubSections(QString indent, QTextStream &out, const DcfSection &sect);
void generateDcfSections(const DcfSection &rootSect, const QString& fileName,
                         const QString& category );

QT_END_NAMESPACE

#endif
