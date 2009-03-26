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

#include "metatranslator.h"

#include <QVector>
#include <QMap>
#include <QStringList>
#include <stdio.h>
#include <ctype.h>

QT_BEGIN_NAMESPACE

typedef QMap<QByteArray, TranslatorMessage> TMM;
typedef QList<TranslatorMessage> TML;

static bool isDigitFriendly( int c )
{
    return ispunct((uchar)c) || isspace((uchar)c);
}

static int numberLength( const char *s )
{
    int i = 0;

    if ( isdigit((uchar)s[0]) ) {
        do {
            i++;
        } while (isdigit((uchar)s[i]) ||
                 (isDigitFriendly(s[i]) &&
                  (isdigit((uchar)s[i + 1]) ||
                   (isDigitFriendly(s[i + 1]) && isdigit((uchar)s[i + 2])))));
    }
    return i;
}

/*
  Returns a version of 'key' where all numbers have been replaced by zeroes.  If
  there were none, returns "".
*/
static QByteArray zeroKey( const char *key )
{
    QByteArray zeroed;
    zeroed.resize( int(strlen(key)) + 1 );
    char *z = zeroed.data();
    int i = 0, j = 0;
    int len;
    bool metSomething = false;

    while ( key[i] != '\0' ) {
        len = numberLength( key + i );
        if ( len > 0 ) {
            i += len;
            z[j++] = '0';
            metSomething = true;
        } else {
            z[j++] = key[i++];
        }
    }
    zeroed.resize(j);

    if ( metSomething )
        return zeroed;
    else
        return "";
}

static QString translationAttempt( const QString& oldTranslation,
                                   const char *oldSource,
                                   const char *newSource )
{
    int p = zeroKey( oldSource ).count( '0' );
    int oldSourceLen = qstrlen( oldSource );
    QString attempt;
    QStringList oldNumbers;
    QStringList newNumbers;
    QVector<bool> met( p );
    QVector<int> matchedYet( p );
    int i, j;
    int k = 0, ell, best;
    int m, n;
    int pass;

    /*
      This algorithm is hard to follow, so we'll consider an example
      all along: oldTranslation is "XeT 3.0", oldSource is "TeX 3.0"
      and newSource is "XeT 3.1".

      First, we set up two tables: oldNumbers and newNumbers. In our
      example, oldNumber[0] is "3.0" and newNumber[0] is "3.1".
    */
    for ( i = 0, j = 0; i < oldSourceLen; i++, j++ ) {
        m = numberLength( oldSource + i );
        n = numberLength( newSource + j );
        if ( m > 0 ) {
            oldNumbers.append( QString::fromLatin1(QByteArray(oldSource + i, m + 1) .constData()) );
            newNumbers.append( QString::fromLatin1(QByteArray(newSource + j, n + 1).constData()) );
            i += m;
            j += n;
            met[k] = false;
            matchedYet[k] = 0;
            k++;
        }
    }

    /*
      We now go over the old translation, "XeT 3.0", one letter at a
      time, looking for numbers found in oldNumbers. Whenever such a
      number is met, it is replaced with its newNumber equivalent. In
      our example, the "3.0" of "XeT 3.0" becomes "3.1".
    */
    for ( i = 0; i < (int) oldTranslation.length(); i++ ) {
        attempt += oldTranslation[i];
        for ( k = 0; k < p; k++ ) {
            if ( oldTranslation[i] == oldNumbers[k][matchedYet[k]] )
                matchedYet[k]++;
            else
                matchedYet[k] = 0;
        }

        /*
          Let's find out if the last character ended a match. We make
          two passes over the data. In the first pass, we try to
          match only numbers that weren't matched yet; if that fails,
          the second pass does the trick. This is useful in some
          suspicious cases, flagged below.
        */
        for ( pass = 0; pass < 2; pass++ ) {
            best = p; // an impossible value
            for ( k = 0; k < p; k++ ) {
                if ( (!met[k] || pass > 0) &&
                     matchedYet[k] == (int) oldNumbers[k].length() &&
                     numberLength(oldTranslation.toLatin1().constData() + (i + 1) -
                                  matchedYet[k]) == matchedYet[k] ) {
                    // the longer the better
                    if ( best == p || matchedYet[k] > matchedYet[best] )
                        best = k;
                }
            }
            if ( best != p ) {
                attempt.truncate( attempt.length() - matchedYet[best] );
                attempt += newNumbers[best];
                met[best] = true;
                for ( k = 0; k < p; k++ )
                    matchedYet[k] = 0;
                break;
            }
        }
    }

    /*
      We flag two kinds of suspicious cases. They are identified as
      such with comments such as "{2000?}" at the end.

      Example of the first kind: old source text "TeX 3.0" translated
      as "XeT 2.0" is flagged "TeX 2.0 {3.0?}", no matter what the
      new text is.
    */
    for ( k = 0; k < p; k++ ) {
        if ( !met[k] )
            attempt += QString( QLatin1String(" {") ) + newNumbers[k] + QString( QLatin1String("?}") );
    }

    /*
      Example of the second kind: "1 of 1" translated as "1 af 1",
      with new source text "1 of 2", generates "1 af 2 {1 or 2?}"
      because it's not clear which of "1 af 2" and "2 af 1" is right.
    */
    for ( k = 0; k < p; k++ ) {
        for ( ell = 0; ell < p; ell++ ) {
            if ( k != ell && oldNumbers[k] == oldNumbers[ell] &&
                    newNumbers[k] < newNumbers[ell] )
                attempt += QString( QLatin1String(" {") ) + newNumbers[k] + QString( QLatin1String(" or ") ) +
                           newNumbers[ell] + QString( QLatin1String("?}") );
        }
    }
    return attempt;
}

/*
  Augments a MetaTranslator with translations easily derived from
  similar existing (probably obsolete) translations.

  For example, if "TeX 3.0" is translated as "XeT 3.0" and "TeX 3.1"
  has no translation, "XeT 3.1" is added to the translator and is
  marked Unfinished.

  Returns the number of additional messages that this heuristic translated.
*/
int applyNumberHeuristic( MetaTranslator *tor )
{
    TMM translated, untranslated;
    TMM::Iterator t, u;
    TML all = tor->messages();
    TML::Iterator it;
    int inserted = 0;

    for ( it = all.begin(); it != all.end(); ++it ) {
        bool hasTranslation = (*it).isTranslated();
        if ( (*it).type() == TranslatorMessage::Unfinished ) {
            if ( !hasTranslation )
                untranslated.insert((*it).context() + "\n"
                    + (*it).sourceText() + "\n"
                    + (*it).comment(), *it);
        } else if ( hasTranslation && (*it).translations().count() == 1 ) {
            translated.insert( zeroKey((*it).sourceText()), *it );
        }
    }

    for ( u = untranslated.begin(); u != untranslated.end(); ++u ) {
        t = translated.find( zeroKey((*u).sourceText()) );
        if ( t != translated.end() && !t.key().isEmpty() &&
             qstrcmp((*t).sourceText(), (*u).sourceText()) != 0 ) {
            TranslatorMessage m( *u );
            m.setTranslation(translationAttempt((*t).translation(), (*t).sourceText(),
                                                (*u).sourceText()));
            tor->insert( m );
            inserted++;
        }
    }
    return inserted;
}

QT_END_NAMESPACE
