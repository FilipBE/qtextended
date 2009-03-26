/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/
#ifndef QDAWG_H
#define QDAWG_H

#include <qtopiaglobal.h>
#include <qstringlist.h>
#include <cstdio>

class QIODevice;
class QDawgPrivate;

class QTOPIABASE_EXPORT QDawg {
public:
    QDawg();
    ~QDawg();

    bool readFile(const QString&); // may mmap
    bool read(QIODevice* dev);
    bool write(QIODevice* dev) const;
    bool writeByteSwapped(QIODevice* dev) const;
    bool createFromWords(QIODevice* dev);
    void createFromWords(const QStringList&);
    QStringList allWords() const;

    bool contains(const QString&) const;
    int countWords() const;

    class QTOPIABASE_EXPORT Node {
        friend class QDawgPrivate;
        quint16 let;
        // lower 14 bits  of val are not used unless
        // QTOPIA_INTERNAL_QDAWG_TRIE is defined
        // Upper two bits of val are (always) used for isWord and isLast
        quint16 val;
        static const quint16 isword = 0x8000;
        static const quint16 islast = 0x4000;
        qint32 offset;
        Node() { val = 0; /*set zero for better compression*/ }
        inline void setValue(uint i) { val = (val & ~MaxValue) | (i & MaxValue); }
        inline void setIsWord(bool isWord) { isWord ? val |= isword : val &= ~isword; }
        inline void setIsLast(bool isLast) { isLast ? val |= islast : val &= ~islast; }

    public:
        QChar letter() const { return QChar((ushort)let); }
        inline bool isWord() const { return isword & val; }
        inline bool isLast() const { return islast & val; }
        const Node* next() const { return (islast & val) ? 0 : this+1; }
        const Node* jump() const { return offset ? this+offset : 0; }

        static const quint16 MaxValue = 0x3fff; // also a mask
        inline int value() const { return val & MaxValue; }
    };

    const Node* root() const;

    void dump() const; // debug

private:
    friend class QDawgPrivate;
    QDawgPrivate* d;
};


#endif
