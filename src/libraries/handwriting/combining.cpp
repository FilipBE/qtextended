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

#include <qfile.h>
#include <math.h>
#include <limits.h>
#include <qdatastream.h>
#include "combining_p.h"

// set of chararaters that can be combined against
static unsigned int combiningSymbols[] = { '\\', '/', '^', '~', '\"', 'o' };

// other set of characters that can be combined against
// struct is
//  char
//  combiningSymbols[i] == unicode symbol for...

// definately should be made extensible....
// make configurable out of etc file?:
static const ushort combiningChars[][7] = {
    //     \       /       ^       ~       "
    { 'A', 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5 },
    { 'O', 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0000 },
    { 'U', 0x00D9, 0x00DA, 0x00DB, 0x0000, 0x00DC, 0x0000 },
    { 'E', 0x00C8, 0x00C9, 0x00CA, 0x0000, 0x00CB, 0x0000 },
    { 'I', 0x00CC, 0x00CD, 0x00CE, 0x0000, 0x00CF, 0x0000 },
    { 'a', 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5 },
    { 'e', 0x00E8, 0x00E9, 0x00EA, 0x0000, 0x00EB, 0x0000 },
    { 'i', 0x00EC, 0x00ED, 0x00EE, 0x0000, 0x00EF, 0x0000 },
    { 'n', 0x0000, 0x0000, 0x0000, 0x00F1, 0x0000, 0x0000 },
    { 'o', 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0000 },
    { 'u', 0x00F9, 0x00FA, 0x00FB, 0x0000, 0x00FC, 0x0000 },
    { 'y', 0x0000, 0x00FD, 0x0000, 0x0000, 0x00FF, 0x0000 },
    { 0, 0, 0, 0, 0, 0, 0 }
};


QIMPenCombining::QIMPenCombining()
{
}

QIMPenCombining::QIMPenCombining( const QString &fn )
    : QIMPenCharSet( fn )
{
}

/*
  For each character in \a cs (that isn't marked as deleted)
  and is n, y, or a vowel, add to it a combination of each
  combining character.

  so a turns into áâä, etc.

  can work with any chars that are added to top of this file
*/
void QIMPenCombining::addCombined( QIMPenCharSet *cs )
{
    QIMPenCharList toAdd;
    foreach( QIMPenChar *pc, cs->characters()) {
        if ( pc->testFlag( QIMPenChar::Deleted ) )
            continue;
        int charIdx = findCombining( pc->repCharacter() );
        if ( charIdx < 0 )
            continue;
        for ( int i = 0; i < 6; i++ ) {
            if ( combiningChars[charIdx][i+1] ) {
                // this craracters
                foreach(QIMPenChar *accentPc, characters()) {
                    if ( accentPc->repCharacter() == combiningSymbols[i] ) {
                        QIMPenChar *combined = combine( pc, accentPc );
                        combined->setRepCharacter( combiningChars[charIdx][i+1] );
                        toAdd.append( combined );
                        // cs->addChar( combined );
                    }
                }
            }
        }
    }
    foreach(QIMPenChar *pc, toAdd) {
        cs->addChar( pc );
    }
}

int QIMPenCombining::findCombining( QChar ch ) const
{
    int i = 0;
    while ( combiningChars[i][0] ) {
        if ( combiningChars[i][0] == ch )
            return i;
        i++;
    }

    return -1;
}

QIMPenChar *QIMPenCombining::combine( QIMPenChar *base, QIMPenChar *accent )
{
    QRect brect = base->boundingRect();
    QRect arect = accent->boundingRect();
    int offset;
    if ( accent->testFlag( QIMPenChar::CombineRight ) )
        offset = brect.left() - arect.left() + brect.width() + 2;
    else
        offset = brect.left() - arect.left() + (brect.width() - arect.width())/2;
    QIMPenChar *combined = 0;
    if ( base->repCharacter() == QChar('i') ) {
        // Hack to remove the dot from i's when combining.
        if ( base->penStrokes().count() > 1 ) {
            combined = new QIMPenChar;
            QIMPenStrokeConstIterator it = base->penStrokes().begin();
            for ( int i = 0; i < base->penStrokes().count()-1; ++it, i++ ) {
                QIMPenStroke st( **it );
                combined->addStroke( &st );
            }
            combined->setFlag( QIMPenChar::System );
        }
    }
    if ( !combined )
        combined = new QIMPenChar( *base );
    QIMPenStrokeConstIterator it = accent->penStrokes().begin();
    for ( ; it != accent->penStrokes().end(); ++it ) {
        QIMPenStroke *st = new QIMPenStroke( **it );
        st->setStartingPoint( st->startingPoint() + QPoint(offset, 0 ));
        combined->addStroke( st );
        delete st;
    }

    return combined;
}

QIMPenChar *QIMPenCombining::penChar( int type )
{
    // lookup
    QIMPenCharIterator it = characters().begin();
    for ( ; it != characters().end(); ++it ) {
        QIMPenChar *pc = *it;
        if ( pc->repCharacter().unicode() == combiningSymbols[type] )
            return pc;
    }

    return 0;
}

