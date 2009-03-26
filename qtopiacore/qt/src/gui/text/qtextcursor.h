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

#ifndef QTEXTCURSOR_H
#define QTEXTCURSOR_H

#include <QtCore/qstring.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qtextformat.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QTextDocument;
class QTextCursorPrivate;
class QTextDocumentFragment;
class QTextCharFormat;
class QTextBlockFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextFrameFormat;
class QTextImageFormat;
class QTextDocumentPrivate;
class QTextList;
class QTextTable;
class QTextFrame;
class QTextBlock;

class Q_GUI_EXPORT QTextCursor
{
public:
    QTextCursor();
    explicit QTextCursor(QTextDocument *document);
    QTextCursor(QTextDocumentPrivate *p, int pos);
    explicit QTextCursor(QTextFrame *frame);
    explicit QTextCursor(const QTextBlock &block);
    explicit QTextCursor(QTextCursorPrivate *d);
    QTextCursor(const QTextCursor &cursor);
    QTextCursor &operator=(const QTextCursor &other);
    ~QTextCursor();

    bool isNull() const;

    enum MoveMode {
        MoveAnchor,
        KeepAnchor
    };

    void setPosition(int pos, MoveMode mode = MoveAnchor);
    int position() const;

    int anchor() const;

    void insertText(const QString &text);
    void insertText(const QString &text, const QTextCharFormat &format);

    enum MoveOperation {
        NoMove,

        Start,
        Up,
        StartOfLine,
        StartOfBlock,
        StartOfWord,
        PreviousBlock,
        PreviousCharacter,
        PreviousWord,
        Left,
        WordLeft,

        End,
        Down,
        EndOfLine,
        EndOfWord,
        EndOfBlock,
        NextBlock,
        NextCharacter,
        NextWord,
        Right,
        WordRight
    };

    bool movePosition(MoveOperation op, MoveMode = MoveAnchor, int n = 1);

    bool visualNavigation() const;
    void setVisualNavigation(bool b);

    void deleteChar();
    void deletePreviousChar();

    enum SelectionType {
        WordUnderCursor,
        LineUnderCursor,
        BlockUnderCursor,
        Document
    };
    void select(SelectionType selection);

    bool hasSelection() const;
    bool hasComplexSelection() const;
    void removeSelectedText();
    void clearSelection();
    int selectionStart() const;
    int selectionEnd() const;

    QString selectedText() const;
    QTextDocumentFragment selection() const;
    void selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const;

    QTextBlock block() const;

    QTextCharFormat charFormat() const;
    void setCharFormat(const QTextCharFormat &format);
    void mergeCharFormat(const QTextCharFormat &modifier);

    QTextBlockFormat blockFormat() const;
    void setBlockFormat(const QTextBlockFormat &format);
    void mergeBlockFormat(const QTextBlockFormat &modifier);

    QTextCharFormat blockCharFormat() const;
    void setBlockCharFormat(const QTextCharFormat &format);
    void mergeBlockCharFormat(const QTextCharFormat &modifier);

    bool atBlockStart() const;
    bool atBlockEnd() const;
    bool atStart() const;
    bool atEnd() const;

    void insertBlock();
    void insertBlock(const QTextBlockFormat &format);
    void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);

    QTextList *insertList(const QTextListFormat &format);
    QTextList *insertList(QTextListFormat::Style style);

    QTextList *createList(const QTextListFormat &format);
    QTextList *createList(QTextListFormat::Style style);
    QTextList *currentList() const;

    QTextTable *insertTable(int rows, int cols, const QTextTableFormat &format);
    QTextTable *insertTable(int rows, int cols);
    QTextTable *currentTable() const;

    QTextFrame *insertFrame(const QTextFrameFormat &format);
    QTextFrame *currentFrame() const;

    void insertFragment(const QTextDocumentFragment &fragment);

#ifndef QT_NO_TEXTHTMLPARSER
    void insertHtml(const QString &html);
#endif // QT_NO_TEXTHTMLPARSER

    void insertImage(const QTextImageFormat &format, QTextFrameFormat::Position alignment);
    void insertImage(const QTextImageFormat &format);
    void insertImage(const QString &name);

    void beginEditBlock();
    void joinPreviousEditBlock();
    void endEditBlock();

    bool operator!=(const QTextCursor &rhs) const;
    bool operator<(const QTextCursor &rhs) const;
    bool operator<=(const QTextCursor &rhs) const;
    bool operator==(const QTextCursor &rhs) const;
    bool operator>=(const QTextCursor &rhs) const;
    bool operator>(const QTextCursor &rhs) const;

    bool isCopyOf(const QTextCursor &other) const;

    int blockNumber() const;
    int columnNumber() const;

private:
    QSharedDataPointer<QTextCursorPrivate> d;
    friend class QTextDocumentFragmentPrivate;
    friend class QTextCopyHelper;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QTEXTCURSOR_H
