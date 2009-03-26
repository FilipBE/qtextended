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

#ifndef PROREADER_H
#define PROREADER_H

#include <QtCore/QStack>
#include <QtCore/QList>
#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE

class ProBlock;
class ProItem;
class ProFile;

class ProReader
{
public:
    ProReader();
    ProFile *read(const QString &fileName);
    void setEnableBackSlashFixing(bool enable);
    inline int currentLine() const { return m_currentLineNumber; }

protected:
    ProFile *read(QIODevice *device, const QString &name = QLatin1String("device"));

    void writeItem(const QList<ProItem *> &items, int index, QTextStream &out, QString indent);
    ProBlock *currentBlock();
    void updateItem();
    bool parseline(QByteArray line);
    void insertVariable(const QByteArray &line, int *i);
    void insertOperator(const char op);
    void insertComment(const QByteArray &comment);
    void enterScope(bool multiLine);
    void leaveScope();
    void finalizeBlock();
    void cleanup();

private:
    QStack<ProBlock *> m_blockstack;
    ProBlock *m_block;

    ProItem *m_commentItem;
    QByteArray m_proitem;
    QByteArray m_pendingComment;
    bool m_syntaxerror;
    bool m_fixBackSlashes;
    int m_currentLineNumber;
    bool m_contNextLine;
};

QT_END_NAMESPACE

#endif //PROREADER_H
