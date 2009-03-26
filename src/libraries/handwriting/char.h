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

#ifndef CHAR_H
#define CHAR_H

#include <qlist.h>
#include "stroke.h"

#include <qtopiaglobal.h>

class QTOPIAHW_EXPORT QIMPenChar
{
public:
    QIMPenChar();
    QIMPenChar( const QIMPenChar & );
    ~QIMPenChar();

    uint key() const { return mKey; }
    QChar repCharacter() const { return mUnicode; }

    void setKey(uint);
    void setRepCharacter(QChar);

    QString name() const;
    bool isEmpty() const { return strokes.isEmpty(); }
    uint strokeCount() const { return strokes.count(); }
    uint strokeLength( int s ) const;
    void clear();
    int match( QIMPenChar *ch );
    const QIMPenStrokeList &penStrokes() const { return strokes; }
    QPoint startingPoint() const { return strokes.at( 0 )->startingPoint(); }
    QRect boundingRect();

    void setFlag( int f ) { flags |= f; }
    void clearFlag( int f ) { flags &= ~f; }
    bool testFlag( int f ) { return flags & f; }



    enum Flags { System=0x01,
        Deleted=0x02,
        CombineRight=0x04,
        Data=0x08 // kept so can read old files
    };
    // Correspond to codes in template files.  Do not change values.
    enum Mode {
        ModeBase=0x4000,
        Caps=0x4001,
        Shortcut=0x4002,
        CapsLock=0x4003,
        Punctuation=0x4004,
        Symbol=0x4005,
        NextWord=0x4007, // 6 skipped, for compat reasons.
        WordPopup=0x4008,
        SymbolPopup=0x4009,
        ModePopup=0x400A
    };

    QIMPenChar &operator=( const QIMPenChar &s );

    void addStroke( QIMPenStroke * );

protected:
    QChar mUnicode;
    uint mKey;
    quint8 flags;
    QIMPenStrokeList strokes;

    friend QDataStream &operator<< (QDataStream &, const QIMPenChar &);
    friend QDataStream &operator>> (QDataStream &, QIMPenChar &);
};

typedef QList<QIMPenChar *> QIMPenCharList;
typedef QList<QIMPenChar *>::ConstIterator QIMPenCharIterator;


QTOPIAHW_EXPORT QDataStream & operator<< (QDataStream & s, const QIMPenChar &ws);
QTOPIAHW_EXPORT QDataStream & operator>> (QDataStream & s, QIMPenChar &ws);

struct QTOPIAHW_EXPORT QIMPenCharMatch
{
    int error;
    QIMPenChar *penChar;

    bool operator>( const QIMPenCharMatch &m ) const;
    bool operator<( const QIMPenCharMatch &m ) const;
    bool operator<=( const QIMPenCharMatch &m ) const;
};

typedef QList<QIMPenCharMatch> QIMPenCharMatchList;
typedef QList<QIMPenCharMatch>::Iterator QIMPenCharMatchIterator;


class QTOPIAHW_EXPORT QIMPenCharSet
{
public:
    QIMPenCharSet();
    explicit QIMPenCharSet( const QString &fn );
    ~QIMPenCharSet();

    bool isEmpty() const { return chars.isEmpty(); }
    uint count() const { return chars.count(); }
    void clear();

    void setDescription( const QString &d );
    QString description() const { return desc; }
    void setTitle( const QString &t );
    QString title() const { return csTitle; }

    QIMPenCharMatchList match( QIMPenChar *ch );
    void addChar( QIMPenChar *ch );
    void removeChar( QIMPenChar *ch );

    // make it not a pointer?
    QIMPenChar *at( int i );

    unsigned maximumStrokes() const { return maxStrokes; }

    void up( QIMPenChar *ch );
    void down( QIMPenChar *ch );

    enum Type {
        Unknown=0x00,
        Lower=0x01,
        Upper=0x02,
        Combining=0x04,
        Numeric=0x08,
        Punctuation=0x10,
        Symbol=0x20,
        Shortcut=0x40
    };

    //const QIMPenCharList &characters() const { return chars; }

    void setType( Type t );
    Type type() const { return csType; }

    const QString &filename( ) const;
    void setFilename( const QString &fn );

    // loads as if it was user, if no user, as system.
    bool load( const QString & = QString() );
    // always saves as user.
    bool save( ) const;

    const QIMPenCharList &characters() const { return chars; }

private:
    void markDeleted( const QIMPenChar & );

    QString userPath() const;
    QString systemPath() const;

    QString csTitle;
    QString desc;
    QString sysFilename;
    QString userFilename;
    Type csType;
    unsigned maxStrokes;
    QIMPenCharList chars;
    QIMPenCharMatchList matches; // not used?
};

typedef QList<QIMPenCharSet *> QIMPenCharSetList;
typedef QList<QIMPenCharSet *>::const_iterator QIMPenCharSetIterator;

#endif
