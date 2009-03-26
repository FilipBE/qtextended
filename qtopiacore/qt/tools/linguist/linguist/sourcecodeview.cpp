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

#include "sourcecodeview.h"
#include <QFile>
#include <QFileInfo>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>

QT_BEGIN_NAMESPACE

SourceCodeView::SourceCodeView(QWidget *parent) :
    QPlainTextEdit(parent),
    m_isActive(true),
    m_lineNumToLoad(0)
{
    setReadOnly(true);
    m_directory = QDir::current();
}

void SourceCodeView::setSourceContext(const QString &fileName, const int lineNum)
{
    m_fileToLoad.clear();
    setToolTip(fileName);

    if (fileName.isNull()) {
        clear();
        m_currentFileName.clear();
        appendHtml(tr("<i>Source code not available</i>"));
        return;
    }

    if (m_isActive) {
        showSourceCode(fileName, lineNum);
    } else {
        m_fileToLoad = fileName;
        m_lineNumToLoad = lineNum;
    }
}

void SourceCodeView::setActivated(bool activated)
{
    m_isActive = activated;
    if (activated && !m_fileToLoad.isEmpty()) {
        showSourceCode(m_fileToLoad, m_lineNumToLoad);
        m_fileToLoad.clear();
    }
}

void SourceCodeView::showSourceCode(const QString &fileName, const int lineNum)
{
    QString absFileName = m_directory.absoluteFilePath(fileName);
    QString fileText(fileHash.value(absFileName));

    if (fileText.isNull()) { // File not in hash
        m_currentFileName.clear();

        // Assume fileName is relative to directory
        QFile file(absFileName);

        if (!file.exists()) {
            clear();
            appendHtml(tr("<i>File %1 not available</i>").arg(absFileName));
            return;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            clear();
            appendHtml(tr("<i>File %1 not readable</i>").arg(absFileName));
            return;
        }
        fileText = QString::fromLatin1(file.readAll());
        fileHash.insert(absFileName, fileText);
    }


    if (m_currentFileName != absFileName) {
        setPlainText(fileText);
        m_currentFileName = absFileName;
    }

    QTextCursor cursor = textCursor();
    cursor.setPosition(document()->findBlockByNumber(lineNum - 1).position());
    setTextCursor(cursor);
    centerCursor();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    QTextEdit::ExtraSelection selectedLine;
    selectedLine.cursor = cursor;

    // Define custom color for line selection
    const QColor fg = palette().color(QPalette::Highlight);
    const QColor bg = palette().color(QPalette::Base);
    QColor col;
    const qreal ratio = 0.25;
    col.setRedF(fg.redF() * ratio + bg.redF() * (1 - ratio));
    col.setGreenF(fg.greenF() * ratio + bg.greenF() * (1 - ratio));
    col.setBlueF(fg.blueF() * ratio + bg.blueF() * (1 - ratio));

    selectedLine.format.setBackground(col);
    selectedLine.format.setProperty(QTextFormat::FullWidthSelection, true);
    setExtraSelections(QList<QTextEdit::ExtraSelection>() << selectedLine);
}

QT_END_NAMESPACE
