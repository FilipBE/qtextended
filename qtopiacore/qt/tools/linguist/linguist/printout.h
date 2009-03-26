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

#ifndef PRINTOUT_H
#define PRINTOUT_H

#include <QFont>
#include <QPainter>
#include <QRect>
#include <QList>
#include <QDateTime>

QT_BEGIN_NAMESPACE

class QPrinter;
class QFontMetrics;

class PrintOut
{
public:
    enum Rule { NoRule, ThinRule, ThickRule };
    enum Style { Normal, Strong, Emphasis };

    PrintOut( QPrinter *printer );
    ~PrintOut();

    void setRule( Rule rule );
    void setGuide( const QString& guide );
    void vskip();
    void flushLine( bool mayBreak = false );
    void addBox( int percent, const QString& text = QString(),
                 Style style = Normal,
                 int halign = Qt::AlignLeft | Qt::TextWordWrap ); //NEW WordBread -> TextWordWrap

    int pageNum() const { return page; }

    struct Box
    {
        QRect rect;
        QString text;
        QFont font;
        int align;

        Box() : align( 0 ) { }
        Box( const QRect& r, const QString& t, const QFont& f, int a )
            : rect( r ), text( t ), font( f ), align( a ) { }
        Box( const Box& b )
            : rect( b.rect ), text( b.text ), font( b.font ),
              align( b.align ) { }

        Box& operator=( const Box& b ) {
            rect = b.rect;
            text = b.text;
            font = b.font;
            align = b.align;
            return *this;
        }

        bool operator==( const Box& b ) const {
            return rect == b.rect && text == b.text && font == b.font &&
                   align == b.align;
        }
    };

private:
    void breakPage(bool init = false);
    void drawRule( Rule rule );

    struct Paragraph {
        QRect rect;
        QList<Box> boxes;

        Paragraph() { }
        Paragraph( QPoint p ) : rect( p, QSize(0, 0) ) { }
    };

    QPrinter *pr;
    QPainter p;
    QFont f8;
    QFont f10;
    QFontMetrics *fmetrics;
    Rule nextRule;
    Paragraph cp;
    int page;
    bool firstParagraph;
    QString g;
    QDateTime dateTime;

    int hmargin;
    int vmargin;
    int voffset;
    int hsize;
    int vsize;
};

QT_END_NAMESPACE

#endif
