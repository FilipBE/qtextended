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
  codechunk.h
*/

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <qstring.h>

QT_BEGIN_NAMESPACE

// ### get rid of that class

/*
  The CodeChunk class represents a tiny piece of C++ code.

  The class provides convertion between a list of lexemes and a string.  It adds
  spaces at the right place for consistent style.  The tiny pieces of code it
  represents are data types, enum values, and default parameter values.

  Apart from the piece of code itself, there are two bits of metainformation
  stored in CodeChunk: the base and the hotspot.  The base is the part of the
  piece that may be a hypertext link.  The base of

      QMap<QString, QString>

  is QMap.

  The hotspot is the place the variable name should be inserted in the case of a
  variable (or parameter) declaration.  The base of

      char * []

  is between '*' and '[]'.
*/
class CodeChunk
{
public:
    CodeChunk();
    CodeChunk( const QString& str );

    void append( const QString& lexeme );
    void appendHotspot();

    bool isEmpty() const { return s.isEmpty(); }
    QString toString() const;
    QStringList toPath() const;
    QString left() const { return s.left(hotspot == -1 ? s.length() : hotspot); }
    QString right() const { return s.mid(hotspot == -1 ? s.length() : hotspot); }

private:
    QString s;
    int hotspot;
};

inline bool operator==( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() == d.toString();
}

inline bool operator!=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c == d );
}

inline bool operator<( const CodeChunk& c, const CodeChunk& d ) {
    return c.toString() < d.toString();
}

inline bool operator>( const CodeChunk& c, const CodeChunk& d ) {
    return d < c;
}

inline bool operator<=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c > d );
}

inline bool operator>=( const CodeChunk& c, const CodeChunk& d ) {
    return !( c < d );
}

QT_END_NAMESPACE

#endif
