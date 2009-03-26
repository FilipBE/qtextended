/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtSVG module of the Qt Toolkit.
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

#ifndef QSVGGRAPHICS_P_H
#define QSVGGRAPHICS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qsvgnode_p.h"

#ifndef QT_NO_SVG

#include "QtGui/qpainterpath.h"
#include "QtGui/qimage.h"
#include "QtGui/qtextlayout.h"
#include "QtGui/qtextoption.h"
#include "QtCore/qstack.h"

QT_BEGIN_NAMESPACE

class QTextCharFormat;

class QSvgAnimation : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

class QSvgArc : public QSvgNode
{
public:
    QSvgArc(QSvgNode *parent, const QPainterPath &path);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QPainterPath cubic;
    QRectF m_cachedBounds;
};

class QSvgCircle : public QSvgNode
{
public:
    QSvgCircle(QSvgNode *parent, const QRectF &rect);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QRectF m_bounds;
};

class QSvgEllipse : public QSvgNode
{
public:
    QSvgEllipse(QSvgNode *parent, const QRectF &rect);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QRectF m_bounds;
};

class QSvgImage : public QSvgNode
{
public:
    QSvgImage(QSvgNode *parent, const QImage &image,
              const QRect &bounds);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QImage m_image;
    QRect  m_bounds;
};

class QSvgLine : public QSvgNode
{
public:
    QSvgLine(QSvgNode *parent, const QLineF &line);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QLineF m_bounds;
};

class QSvgPath : public QSvgNode
{
public:
    QSvgPath(QSvgNode *parent, const QPainterPath &qpath);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;

    QPainterPath *qpath() {
        return &m_path;
    }
private:
    QPainterPath m_path;
    QRectF m_cachedBounds;
};

class QSvgPolygon : public QSvgNode
{
public:
    QSvgPolygon(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QPolygonF m_poly;
};

class QSvgPolyline : public QSvgNode
{
public:
    QSvgPolyline(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
private:
    QPolygonF m_poly;
};

class QSvgRect : public QSvgNode
{
public:
    QSvgRect(QSvgNode *paren, const QRectF &rect, int rx=0, int ry=0);
    virtual Type type() const;
    virtual void draw(QPainter *p);
    virtual QRectF bounds() const;
private:
    QRectF m_rect;
    int m_rx, m_ry;
};

class  QSvgText : public QSvgNode
{
public:
    QSvgText(QSvgNode *parent, const QPointF &coord);
    ~QSvgText();
    virtual void draw(QPainter *p);
    virtual Type type() const;
    void insertText(const QString &text);
    void insertFormat(const QTextCharFormat &format);
    void popFormat();
    void setTextAlignment(const Qt::Alignment &alignment);
    const QTextCharFormat &topFormat() const;
    //virtual QRectF bounds() const;
private:
    QPointF m_coord;

    QString m_text;
    QStack<QTextCharFormat> m_formats;
    Qt::Alignment           m_textAlignment;
    QList<QTextLayout::FormatRange> m_formatRanges;

};

class  QSvgTextArea : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

class QSvgUse : public QSvgNode
{
public:
    QSvgUse(const QPointF &start, QSvgNode *parent, QSvgNode *link);
    virtual void draw(QPainter *p);
    virtual Type type() const;
    virtual QRectF bounds() const;
    virtual QRectF transformedBounds(const QMatrix &mat) const;

private:
    QSvgNode *m_link;
    QPointF   m_start;
    mutable QRectF    m_bounds;
};

class QSvgVideo : public QSvgNode
{
public:
    virtual void draw(QPainter *p);
    virtual Type type() const;
};

QT_END_NAMESPACE

#endif // QT_NO_SVG
#endif // QSVGGRAPHICS_P_H
