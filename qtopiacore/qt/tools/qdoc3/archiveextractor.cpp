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

/*
  archiveextractor.cpp
*/

#include "archiveextractor.h"

QT_BEGIN_NAMESPACE

QList<ArchiveExtractor *> ArchiveExtractor::extractors;

/*!
  \class ArchiveExtractor
  
  \brief The ArchiveExtractor class is a base class for classes that
  know how to unpack a certain kind of archive file.

  The archive extractor contains a list of the filename extensions
  of the files that the archive extractor knows how to unpack.

  It maintains a static list of all the instances of ArchiveExtractor
  that have been created. It also has a static function for searching
  that list to find the archive extracter for a file with a certain
  extension.
 */

/*!
  The constructor takes a list of filename extensions, which it
  copies and saves internally. This archive extractor is prepended
  to the static list.
 */
ArchiveExtractor::ArchiveExtractor( const QStringList& extensions )
    : fileExts( extensions )
{
    extractors.prepend( this );
}

/*!
  The destructor deletes all the filename extensions.
 */
ArchiveExtractor::~ArchiveExtractor()
{
    extractors.removeAll( this );
}

/*!
  This function searches the static list of archive extractors
  to find the first one that can handle \a fileName. If it finds
  an acceptable extractor, it returns a pointer to it. Otherwise
  it returns null.
 */
ArchiveExtractor*
ArchiveExtractor::extractorForFileName( const QString& fileName )
{
    int dot = -1;
    while ( (dot = fileName.indexOf(QLatin1Char('.'), dot + 1)) != -1 ) {
        QString ext = fileName.mid( dot + 1 );
        QList<ArchiveExtractor *>::ConstIterator e = extractors.begin();
        while ( e != extractors.end() ) {
            if ( (*e)->fileExtensions().contains(ext) )
                return *e;
            ++e;
        }
    }
    return 0;
}

QT_END_NAMESPACE
