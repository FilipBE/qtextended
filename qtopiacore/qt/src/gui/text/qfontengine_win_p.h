/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QFONTENGINE_WIN_P_H
#define QFONTENGINE_WIN_P_H

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

#include <QtCore/qconfig.h>

QT_BEGIN_NAMESPACE

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin(const QString &name, HFONT, bool, LOGFONT);
    ~QFontEngineWin();

    virtual QFixed lineThickness() const;
    virtual Properties properties() const;
    virtual void getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics);
    virtual FaceId faceId() const;
    bool getSfntTableData(uint tag, uchar *buffer, uint *length) const;
    virtual int synthesized() const;
    virtual QFixed emSquareSize() const;

    bool stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const;
    void recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags) const;

    void addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags);
    void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs,
                         QPainterPath *path, QTextItem::RenderFlags flags);

    HGDIOBJ selectDesignFont(QFixed *) const;

    glyph_metrics_t boundingBox(const QGlyphLayout *glyphs, int numGlyphs);
    glyph_metrics_t boundingBox(glyph_t glyph);

    QFixed ascent() const;
    QFixed descent() const;
    QFixed leading() const;
    QFixed xHeight() const;
    QFixed averageCharWidth() const;
    qreal maxCharWidth() const;
    qreal minLeftBearing() const;
    qreal minRightBearing() const;

    const char *name() const;

    bool canRender(const QChar *string, int len);

    Type type() const;

    virtual QImage alphaMapForGlyph(glyph_t);

    int getGlyphIndexes(const QChar *ch, int numChars, QGlyphLayout *glyphs, bool mirrored) const;
    void getCMap();

    QString        _name;
    HFONT        hfont;
    LOGFONT     logfont;
    uint        stockFont   : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    union {
        TEXTMETRICW        w;
        TEXTMETRICA        a;
    } tm;
    int                lw;
    const unsigned char *cmap;
    QByteArray cmapTable;
    mutable qreal lbearing;
    mutable qreal rbearing;
    QFixed designToDevice;
    int unitsPerEm;
    QFixed x_height;
    FaceId _faceId;

    mutable int synthesized_flags;
    mutable QFixed lineWidth;
    mutable unsigned char *widthCache;
    mutable uint widthCacheSize;
    mutable QFixed *designAdvances;
    mutable int designAdvancesSize;
};

class QFontEngineMultiWin : public QFontEngineMulti
{
public:
    QFontEngineMultiWin(QFontEngineWin *first, const QStringList &fallbacks);
    void loadEngine(int at);

    QStringList fallbacks;
};

QT_END_NAMESPACE

#endif // QFONTENGINE_WIN_P_H
