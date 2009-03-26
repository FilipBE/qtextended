/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3SYNTAXHIGHLIGHTER_H
#define Q3SYNTAXHIGHLIGHTER_H

#include <QtGui/qfont.h>
#include <QtGui/qcolor.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

class Q3TextEdit;
class Q3SyntaxHighlighterInternal;
class Q3SyntaxHighlighterPrivate;
class Q3TextParagraph;

class Q_COMPAT_EXPORT Q3SyntaxHighlighter
{
    friend class Q3SyntaxHighlighterInternal;

public:
    Q3SyntaxHighlighter(Q3TextEdit *textEdit);
    virtual ~Q3SyntaxHighlighter();

    virtual int highlightParagraph(const QString &text, int endStateOfLastPara) = 0;

    void setFormat(int start, int count, const QFont &font, const QColor &color);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    Q3TextEdit *textEdit() const { return edit; }

    void rehighlight();

    int currentParagraph() const;

private:
    Q3TextParagraph *para;
    Q3TextEdit *edit;
    Q3SyntaxHighlighterPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3SYNTAXHIGHLIGHTER_H
