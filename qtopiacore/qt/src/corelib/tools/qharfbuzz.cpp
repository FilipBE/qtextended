/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qharfbuzz_p.h"

#include "qunicodetables_p.h"

QT_USE_NAMESPACE

HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_GraphemeClass) prop->graphemeBreak;
}

HB_WordClass HB_GetWordClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_WordClass) prop->wordBreak;
}

HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_SentenceClass) prop->sentenceBreak;
}

HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch)
{
    return (HB_LineBreakClass)QUnicodeTables::lineBreakClass(ch);
}


void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    *grapheme = (HB_GraphemeClass) prop->graphemeBreak;
    *lineBreak = (HB_LineBreakClass) prop->line_break_class;
}

void HB_GetUnicodeCharProperties(HB_UChar32 ch, HB_CharCategory *category, int *combiningClass)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    *category = (HB_CharCategory)prop->category;
    *combiningClass = prop->combiningClass;
}

HB_CharCategory HB_GetUnicodeCharCategory(HB_UChar32 ch)
{
    return (HB_CharCategory)QChar::category(ch);
}

int HB_GetUnicodeCharCombiningClass(HB_UChar32 ch)
{
    return QChar::combiningClass(ch);
}

HB_UChar16 HB_GetMirroredChar(HB_UChar16 ch)
{
    return QChar::mirroredChar(ch);
}

QT_BEGIN_NAMESPACE

void qGetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength,
                        const HB_ScriptItem *items, hb_uint32 numItems,
                        HB_CharAttributes *attributes)
{
    HB_GetCharAttributes(string, stringLength, items, numItems, attributes);
}

HB_Bool qShapeItem(HB_ShaperItem *item)
{
    return HB_ShapeItem(item);
}

HB_Face qHBNewFace(void *font, HB_GetFontTableFunc tableFunc)
{
    return HB_NewFace(font, tableFunc);
}

void qHBFreeFace(HB_Face face)
{
    HB_FreeFace(face);
}

QT_END_NAMESPACE
