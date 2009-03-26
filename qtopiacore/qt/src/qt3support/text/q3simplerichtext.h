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

#ifndef Q3SIMPLERICHTEXT_H
#define Q3SIMPLERICHTEXT_H

#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtGui/qregion.h>
#include <QtGui/qcolor.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_RICHTEXT

class QPainter;
class QWidget;
class Q3StyleSheet;
class QBrush;
class Q3MimeSourceFactory;
class Q3SimpleRichTextData;

class Q_COMPAT_EXPORT Q3SimpleRichText
{
public:
    Q3SimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context = QString(), const Q3StyleSheet* sheet = 0);
    Q3SimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context, const Q3StyleSheet *sheet,
                     const Q3MimeSourceFactory* factory, int pageBreak = -1,
                     const QColor& linkColor = Qt::blue, bool linkUnderline = true);
    ~Q3SimpleRichText();

    void setWidth(int);
    void setWidth(QPainter*, int);
    void setDefaultFont(const QFont &f);
    int width() const;
    int widthUsed() const;
    int height() const;
    void adjustSize();

    void draw(QPainter* p, int x, int y, const QRect& clipRect,
               const QColorGroup& cg, const QBrush* paper = 0) const;

    void draw(QPainter* p, int x, int y, const QRegion& clipRegion,
               const QColorGroup& cg, const QBrush* paper = 0) const {
        draw(p, x, y, clipRegion.boundingRect(), cg, paper);
    }

    QString context() const;
    QString anchorAt(const QPoint& pos) const;

    bool inText(const QPoint& pos) const;

private:
    Q_DISABLE_COPY(Q3SimpleRichText)

    Q3SimpleRichTextData* d;
};

#endif // QT_NO_RICHTEXT

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3SIMPLERICHTEXT_H
